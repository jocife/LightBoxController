#include "sunlightsimulatorpagewidget.h"
#include "ui_sunlightsimulatorpagewidget.h"
#include "optimization_runner.h"
#include "presetfilemanager.h"
#include <QListWidgetItem>
#include <QStringList>
#include <QCoreApplication>
#include <QtCharts/QLegendMarker>
#include <QBrush>

SunlightSimulatorPageWidget::SunlightSimulatorPageWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SunlightSimulatorPageWidget)
    , m_chart(new QChart())
    , m_testSeries(new QLineSeries())
    , m_refSeries(new QLineSeries())
    , m_axisX(new QValueAxis())
    , m_axisY(new QValueAxis())
{
    ui->setupUi(this);

    populateLeds();
    setupChart();

    connect(ui->pushButtonCalculate, &QPushButton::clicked, this, &SunlightSimulatorPageWidget::triggerCalculate);
    connect(ui->pushButtonSave, &QPushButton::clicked, this, &SunlightSimulatorPageWidget::triggerSave);

    // Automatically trigger calculation on page initialization to display SPD values
    triggerCalculate();
}

SunlightSimulatorPageWidget::~SunlightSimulatorPageWidget()
{
    delete ui;
}

void SunlightSimulatorPageWidget::populateLeds()
{
    QStringList ledNames = {
        "Violet", "Royal Blue", "Blue", "Cyan", "Green", "Lime", "Mint", 
        "PC Amber", "Amber", "Red Orange", "Red", "Deep Red", "Far Red", 
        "UV 345", "UV 365", "UV 385", "UV 395", "UV 405", "UV 415", 
        "Warm White", "Neutral White", "Cool White"
    };

    for (const QString& name : ledNames) {
        QListWidgetItem* item = new QListWidgetItem(name, ui->listWidgetLeds);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
    }
}

QVector<int> SunlightSimulatorPageWidget::selectedLedIndices() const
{
    QVector<int> indices;
    for (int index = 0; index < ui->listWidgetLeds->count(); ++index) {
        if (ui->listWidgetLeds->item(index)->checkState() == Qt::Checked) {
            indices.append(index);
        }
    }
    return indices;
}

void SunlightSimulatorPageWidget::setupChart()
{
    // Force a light chart theme and neutral background to avoid theme overrides
    m_chart->setTheme(QChart::ChartThemeLight);
    m_chart->setBackgroundBrush(QBrush(Qt::white));

    m_testSeries->setName("Test LED Source");
    m_refSeries->setName("Reference Illuminant");

    // Style the reference series to have a dashed line
    QPen refPen = m_refSeries->pen();
    refPen.setStyle(Qt::DashLine);
    refPen.setWidth(2);
    refPen.setColor(QColor("#ff7f0e")); // orange for reference
    m_refSeries->setPen(refPen);

    QPen testPen = m_testSeries->pen();
    testPen.setWidth(2);
    testPen.setColor(QColor("#1f77b4")); // blue for test source
    m_testSeries->setPen(testPen);

    m_chart->addSeries(m_testSeries);
    m_chart->addSeries(m_refSeries);

    m_axisX->setTitleText("Wavelength (nm)");
    m_axisX->setRange(380, 780);
    m_axisX->setTickCount(9); // 380, 430, 480, 530, 580, 630, 680, 730, 780
    m_axisX->setLabelFormat("%d");
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_testSeries->attachAxis(m_axisX);
    m_refSeries->attachAxis(m_axisX);

    m_axisY->setTitleText("Relative SPD (equal-area scaled)");
    m_axisY->setRange(0, 1.0); // Will dynamically update on calculation
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_testSeries->attachAxis(m_axisY);
    m_refSeries->attachAxis(m_axisY);

    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);

    // Ensure legend markers mirror series colors
    auto markers = m_chart->legend()->markers();
    for (auto marker : markers) {
        if (qobject_cast<QLineSeries*>(marker->series()) == m_testSeries) {
            QPen mp = marker->pen();
            mp.setColor(testPen.color());
            marker->setPen(mp);
            marker->setBrush(QBrush(testPen.color()));
        } else if (qobject_cast<QLineSeries*>(marker->series()) == m_refSeries) {
            QPen mp = marker->pen();
            mp.setColor(refPen.color());
            marker->setPen(mp);
            marker->setBrush(QBrush(refPen.color()));
        }
    }

    ui->chartView->setChart(m_chart);
    ui->chartView->setRenderHint(QPainter::Antialiasing);
    // Clear any widget-level stylesheet that might affect chart rendering
    ui->chartView->setStyleSheet("");
}

void SunlightSimulatorPageWidget::triggerCalculate()
{
    emit logMessage("SunlightSimulatorPage", "Calculating values for " + ui->comboBoxIlluminant->currentText());

    const QVector<int> selectedLeds = selectedLedIndices();
    if (selectedLeds.isEmpty()) {
        emit logMessage("SunlightSimulatorPage", "Select at least one LED before calculating.");
        return;
    }

    QString csvPath = QCoreApplication::applicationDirPath() + "/configs/LED_SPD_CALIBRATION.csv";
    int illuminantIndex = ui->comboBoxIlluminant->currentIndex(); // 0 for D50, 1 for D55, 2 for D65, etc.

    // 1. Run MATLAB algorithm to get optimal weights
    QVector<double> weights = runFminconOptimization(csvPath, illuminantIndex, ',', ';', selectedLeds);
    m_lastWeights = weights;
    m_lastSelectedLedIndices = selectedLeds;

    // 2. Evaluate all metrics using the weights
    EvaluatedMetrics metrics = runMetricsEvaluation(weights, csvPath, illuminantIndex, ',', ';', selectedLeds);

    // 3. Update the UI Text labels
    updateLabels(metrics.Mu, metrics.Mv, metrics.Duv, metrics.CRI, metrics.CCT);

    // 4. Cache metrics and refresh the normalized plot
    m_lastMetrics = metrics;
    updateChart();
}

void SunlightSimulatorPageWidget::triggerSave()
{
    if (m_lastWeights.size() != 22) {
        emit logMessage("SunlightSimulatorPage", "Calculate the sunlight simulation before saving.");
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Sunlight Simulator Preset"),
                                                    QCoreApplication::applicationDirPath() + "/presets/untitled.xml",
                                                    tr("XML (*.xml);;All Files (*)"));
    if (filePath.isEmpty()) return;

    QVector<int> values(PresetFileManager::ledNames().size(), 0);
    for (int ledIndex : m_lastSelectedLedIndices) {
        if (ledIndex >= 0 && ledIndex < m_lastWeights.size()) {
            values[ledIndex] = qRound(m_lastWeights.at(ledIndex) * 100.0);
        }
    }

    QString errorMessage;
    if (!PresetFileManager::save(filePath, values, &errorMessage)) {
        emit logMessage("SunlightSimulatorPage", "Failed to open file for saving.");
        return;
    }
    emit logMessage("SunlightSimulatorPage", "Saved preset: " + filePath);
}

void SunlightSimulatorPageWidget::updateLabels(double mu, double mv, double duv, double cri, double cct)
{
    ui->labelMu->setText(QString("Mu: %1").arg(mu));
    ui->labelMv->setText(QString("Mv: %1").arg(mv));
    ui->labelDuv->setText(QString("Duv: %1").arg(duv));
    ui->labelCRI->setText(QString("CRI: %1").arg(cri));
    ui->labelCCT->setText(QString("CCT: %1").arg(cct));
}

void SunlightSimulatorPageWidget::updateChart()
{
    // If we don't have cached metrics (no prior calculation), there is nothing to plot.
    if (m_lastMetrics.wavelengths.isEmpty()) {
        return;
    }

    // Normalize each SPD independently so both curves are displayed on a peak = 1 scale.
    QVector<double> testPlot = m_lastMetrics.testSPD;
    QVector<double> refPlot = m_lastMetrics.refSPD;

    // Equalize the total area under both curves to ensure they are visually comparable in terms of overall energy distribution.
    double testPlotSum = 0.0;
    double refPlotSum = 0.0;

    for (double value : testPlot) {
        testPlotSum += value;
    }

    for (double value : refPlot) {
        refPlotSum += value;
    }

    if (testPlotSum > 0.0 && refPlotSum > 0.0) {
        double scalingFactor = refPlotSum / testPlotSum;
        for (double &value : testPlot) {
            value *= scalingFactor;
        }
    }

    // Replot series
    m_testSeries->clear();
    m_refSeries->clear();

    // Add both spectra to the chart and find their shared maximum value
    // for calculating the Y-axis range below.
    double maxY = 0.0;
    for (int i = 0; i < m_lastMetrics.wavelengths.size(); ++i) {
        double w = m_lastMetrics.wavelengths.at(i);
        double valTest = testPlot.at(i);
        double valRef = refPlot.at(i);

        m_testSeries->append(w, valTest);
        m_refSeries->append(w, valRef);

        if (valTest > maxY) maxY = valTest;
        if (valRef > maxY) maxY = valRef;
    }

    // Dynamically adjust the Y-axis range based on the maximum value of both series,
    // with a 10% margin for better visualization.
    if (maxY > 0.0) {
        m_axisY->setRange(0, maxY * 1.1);
    }

    // Reapply legend marker colors in case the application theme or stylesheet
    // caused them to be overridden earlier.
    auto markers = m_chart->legend()->markers();
    for (auto marker : markers) {
        if (qobject_cast<QLineSeries*>(marker->series()) == m_testSeries) {
            QPen mp = marker->pen();
            mp.setColor(QColor("#1f77b4"));
            marker->setPen(mp);
            marker->setBrush(QBrush(QColor("#1f77b4")));
        } else if (qobject_cast<QLineSeries*>(marker->series()) == m_refSeries) {
            QPen mp = marker->pen();
            mp.setColor(QColor("#ff7f0e"));
            marker->setPen(mp);
            marker->setBrush(QBrush(QColor("#ff7f0e")));
        }
    }
}
