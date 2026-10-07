#include "cie1976pagewidget.h"
#include "ui_cie1976pagewidget.h"
#include "presetfilemanager.h"
#include <QCoreApplication>
#include <QFileDialog>
#include <QListWidgetItem>
#include <QPainter>
#include <QLabel>
#include <QTableWidgetItem>
#include <QtCharts/QChart>
#include <QtCharts/QLegend>

Cie1976PageWidget::Cie1976PageWidget(const QString& imagesFolderAbsolutePath, const QString& calibrationFile,
                                     QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Cie1976PageWidget)
    , imagesFolderAbsolutePath(imagesFolderAbsolutePath)
    , calibrationFilePath(calibrationFile)
    , seriesCIE(new QLineSeries(this))
    , chartCIE(new QChart())
    , chartViewCIE(new QChartView(chartCIE, this))
    , axisX(new QValueAxis(this))
    , axisY(new QValueAxis(this))
{
    ui->setupUi(this);

    ui->doubleSpinBoxU->setRange(0.00105, 0.62337);
    ui->doubleSpinBoxU->setDecimals(5);
    ui->doubleSpinBoxV->setRange(0.01755, 0.59532);
    ui->doubleSpinBoxV->setDecimals(5);

    populateLeds();
    chartCIE->setTheme(QChart::ChartThemeLight);
    chartCIE->setTitle("Calculated spectral power distribution");
    chartCIE->legend()->hide();
    seriesCIE->setName("Calculated LED spectrum");
    chartCIE->addSeries(seriesCIE);
    axisX->setTitleText("Wavelength [nm]");
    axisX->setRange(380, 780);
    axisX->setLabelFormat("%d");
    axisY->setTitleText("Relative SPD [-]");
    axisY->setRange(0, 1);
    chartCIE->addAxis(axisX, Qt::AlignBottom);
    chartCIE->addAxis(axisY, Qt::AlignLeft);
    seriesCIE->attachAxis(axisX);
    seriesCIE->attachAxis(axisY);

    chartViewCIE->setRenderHint(QPainter::Antialiasing);
    chartViewCIE->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->gridLayoutPageCIE1976->addWidget(chartViewCIE, 0, 0);

    // CIE 1976 Chromaticity Diagram
    // Source: http://hyperphysics.phy-astr.gsu.edu/hbase/vision/cie1976.html (HyperPhysics, Georgia State University)
    QLabel* labelDisplayCie1976Diagram = new QLabel("");
    QImage imageCie1976Diagram(imagesFolderAbsolutePath + "/" + "cie1976-chromaticity-diagram.jpg");
    labelDisplayCie1976Diagram->setPixmap(QPixmap::fromImage(imageCie1976Diagram));
    labelDisplayCie1976Diagram->setScaledContents(true);
    ui->gridLayoutPageCIE1976_2->addWidget(labelDisplayCie1976Diagram, 0, 0, -1, -1, Qt::AlignHCenter);

    connect(ui->pushButtonCalculate, &QPushButton::clicked, this, &Cie1976PageWidget::triggerCalculate);
    connect(ui->pushButtonSave, &QPushButton::clicked, this, &Cie1976PageWidget::triggerSave);
}

Cie1976PageWidget::~Cie1976PageWidget()
{
    delete ui;
}

void Cie1976PageWidget::setCalibrationFile(const QString& filePath)
{
    calibrationFilePath = filePath;
}

void Cie1976PageWidget::populateLeds()
{
    ui->listWidgetLeds->clear();
    for (const QString& name : PresetFileManager::ledNames()) {
        QListWidgetItem* item = new QListWidgetItem(name, ui->listWidgetLeds);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
    }
}

QVector<int> Cie1976PageWidget::selectedLedIndices() const
{
    QVector<int> indices;
    for (int index = 0; index < ui->listWidgetLeds->count(); ++index) {
        if (ui->listWidgetLeds->item(index)->checkState() == Qt::Checked) {
            indices.append(index);
        }
    }
    return indices;
}

void Cie1976PageWidget::triggerCalculate()
{
    const QString csvPath = calibrationFilePath;
    const QVector<int> selectedLeds = selectedLedIndices();
    if (selectedLeds.isEmpty()) {
        emit logMessage("CIE 1976", "Select at least one LED before calculating.");
        return;
    }

    const double targetU = ui->doubleSpinBoxU->value();
    const double targetV = ui->doubleSpinBoxV->value();
    lastWeights = runChromaticityTargetOptimization(csvPath, targetU, targetV, ',', ';', selectedLeds);
    if (lastWeights.size() != PresetFileManager::ledNames().size()) {
        emit logMessage("CIE 1976", "Failed to calculate LED weights.");
        return;
    }

    const EvaluatedMetrics metrics = runMetricsEvaluation(lastWeights, csvPath, 2, ',', ';', selectedLeds);

    updateChart(metrics.wavelengths, metrics.testSPD);
    emit logMessage("CIE 1976", QString("Calculated LED weights for target u'=%1, v'=%2.")
                                      .arg(targetU, 0, 'f', 5)
                                      .arg(targetV, 0, 'f', 5));
}

void Cie1976PageWidget::triggerSave()
{
    if (lastWeights.size() != PresetFileManager::ledNames().size()) {
        emit logMessage("CIE 1976", "Calculate LED weights before saving.");
        return;
    }

    const QString filePath = QFileDialog::getSaveFileName(this, tr("Save CIE 1976 Preset"),
                                                          QCoreApplication::applicationDirPath() + "/presets/untitled.xml",
                                                          tr("XML (*.xml);;All Files (*)"));
    if (filePath.isEmpty()) {
        return;
    }

    QVector<int> values;
    for (double weight : lastWeights) {
        values.append(qRound(weight * 100.0));
    }
    QString errorMessage;
    if (!PresetFileManager::save(filePath, values, &errorMessage)) {
        emit logMessage("CIE 1976", "Failed to save preset: " + errorMessage);
        return;
    }
    emit logMessage("CIE 1976", "Saved preset: " + filePath);
}

void Cie1976PageWidget::updateChart(const QVector<double>& wavelengths, const QVector<double>& spectrum)
{
    seriesCIE->clear();
    double maximum = 0.0;
    for (int index = 0; index < wavelengths.size() && index < spectrum.size(); ++index) {
        seriesCIE->append(wavelengths.at(index), spectrum.at(index));
        maximum = qMax(maximum, spectrum.at(index));
    }
    axisY->setRange(0.0, maximum > 0.0 ? maximum * 1.1 : 1.0);

}
