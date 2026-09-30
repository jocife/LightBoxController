#include "presetspagewidget.h"
#include "ui_presetspagewidget.h"
#include "presetfilemanager.h"

#include <QFileDialog>
#include <QFile>
#include <QTextStream>

PresetsPageWidget::PresetsPageWidget(const QString& presetsFolder, const QString& iconsFolder, const QList<LEDControllerWidget*>& ledControllers, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PresetsPageWidget)
    , presetsFolderAbsolutePath(presetsFolder)
    , iconsFolderAbsolutePath(iconsFolder)
    , dirPathPrevious(presetsFolder)
    , fileSystemModel(new QFileSystemModel(this))
    , ledControllerWidgetList(ledControllers)
    , series(new QSplineSeries(this))
    , chart(new QChart())
    , chartView(new QChartView(chart, this))
{
    ui->setupUi(this);

    // Initialize File Explorer
    fileSystemModel->setRootPath(presetsFolderAbsolutePath);

    ui->treeView->setModel(fileSystemModel);
    ui->treeView->setRootIndex(fileSystemModel->index(presetsFolderAbsolutePath));
    ui->treeView->setHeaderHidden(true);
    ui->treeView->hideColumn(1);
    ui->treeView->hideColumn(2);
    ui->treeView->hideColumn(3);

    connect(ui->treeView, &QTreeView::clicked, this, &PresetsPageWidget::treeViewIsClicked);

    // Initialize Load/Delete Buttons
    ui->labelFileInvalid->setText("");
    ui->labelFileInvalid->setAlignment(Qt::AlignHCenter);
    ui->labelFileInvalid->setStyleSheet("font: 12pt; color: red;");

    int iconWidth = 60;
    int iconHeight = 60;
    ui->pushButtonLoadRightSide->setIcon(QIcon(iconsFolderAbsolutePath + "/right-long-solid.svg"));
    ui->pushButtonLoadRightSide->setIconSize(QSize(iconWidth, iconHeight));
    ui->pushButtonDeleteRightSide->setIcon(QIcon(iconsFolderAbsolutePath + "/circle-xmark-regular.svg"));
    ui->pushButtonDeleteRightSide->setIconSize(QSize(iconWidth, iconHeight));

    ui->pushButtonLoadLeftSide->setIcon(QIcon(iconsFolderAbsolutePath + "/left-long-solid.svg"));
    ui->pushButtonLoadLeftSide->setIconSize(QSize(iconWidth, iconHeight));
    ui->pushButtonDeleteLeftSide->setIcon(QIcon(iconsFolderAbsolutePath + "/circle-xmark-regular.svg"));
    ui->pushButtonDeleteLeftSide->setIconSize(QSize(iconWidth, iconHeight));

    ui->pushButtonLoadBothSides->setIcon(QIcon(iconsFolderAbsolutePath + "/left-right-solid.svg"));
    ui->pushButtonLoadBothSides->setIconSize(QSize(iconWidth, iconHeight));
    ui->pushButtonDeleteBothSides->setIcon(QIcon(iconsFolderAbsolutePath + "/circle-xmark-regular.svg"));
    ui->pushButtonDeleteBothSides->setIconSize(QSize(iconWidth, iconHeight));

    ui->pushButtonchangeSides->setIcon(QIcon(iconsFolderAbsolutePath + "/right-left-solid.svg"));
    ui->pushButtonchangeSides->setIconSize(QSize(iconWidth, iconHeight));

    connect(ui->pushButtonLoadRightSide, &QPushButton::clicked, this, &PresetsPageWidget::pushButtonLoadRightSideIsClicked);
    connect(ui->pushButtonDeleteRightSide, &QPushButton::clicked, this, &PresetsPageWidget::pushButtonDeleteRightSideIsClicked);
    connect(ui->pushButtonLoadLeftSide, &QPushButton::clicked, this, &PresetsPageWidget::pushButtonLoadLeftSideIsClicked);
    connect(ui->pushButtonDeleteLeftSide, &QPushButton::clicked, this, &PresetsPageWidget::pushButtonDeleteLeftSideIsClicked);
    connect(ui->pushButtonLoadBothSides, &QPushButton::clicked, this, &PresetsPageWidget::pushButtonLoadBothSidesIsClicked);
    connect(ui->pushButtonDeleteBothSides, &QPushButton::clicked, this, &PresetsPageWidget::pushButtonDeleteBothSidesIsClicked);
    connect(ui->pushButtonchangeSides, &QPushButton::clicked, this, &PresetsPageWidget::pushButtonchangeSidesIsClicked);

    // Initialize Preview table
    ui->tableWidget->setColumnWidth(0, 90);
    ui->tableWidget->setColumnWidth(1, 45);
    ui->tableWidget->setColumnWidth(2, 45);
    ui->tableWidget->setColumnWidth(3, 45);
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    for (int i = 0; i < ledControllerWidgetList.size(); ++i) {
        ui->tableWidget->insertRow(i);

        QTableWidgetItem *ledNameItem = new QTableWidgetItem(ledControllerWidgetList[i]->labelText());
        ledNameItem->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        ui->tableWidget->setItem(i, 0, ledNameItem);

        QTableWidgetItem *leftSliderValueItem = new QTableWidgetItem("0%");
        leftSliderValueItem->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        ui->tableWidget->setItem(i, 1, leftSliderValueItem);

        QTableWidgetItem *rightSliderValueItem = new QTableWidgetItem("0%");
        rightSliderValueItem->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        ui->tableWidget->setItem(i, 2, rightSliderValueItem);

        QTableWidgetItem *fileSliderValueItem = new QTableWidgetItem("0%");
        fileSliderValueItem->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        ui->tableWidget->setItem(i, 3, fileSliderValueItem);
    }

    connect(ui->tableWidget, &QTableWidget::cellClicked, this, &PresetsPageWidget::tableWidgetCellIsClicked);

    // Initialize Global sliders
    setGlobalSliderValues(100, 100);
    ui->widgetGlobalSliders->hide(); // Hide by default until Dimming is enabled

    connect(ui->spinBoxGlobalLeft, SIGNAL(valueChanged(int)), this, SLOT(spinBoxGlobalLeftValueHasChanged(int)));
    connect(ui->spinBoxGlobalRight, SIGNAL(valueChanged(int)), this, SLOT(spinBoxGlobalRightValueHasChanged(int)));

    connect(ui->sliderGlobalLeft, &QSlider::sliderPressed, this, &PresetsPageWidget::sliderGlobalLeftIsPressed);
    connect(ui->sliderGlobalRight, &QSlider::sliderPressed, this, &PresetsPageWidget::sliderGlobalRightIsPressed);

    connect(ui->sliderGlobalLeft, &QSlider::valueChanged, this, &PresetsPageWidget::sliderGlobalLeftValueHasChanged);
    connect(ui->sliderGlobalRight, &QSlider::valueChanged, this, &PresetsPageWidget::sliderGlobalRightValueHasChanged);

    connect(ui->sliderGlobalLeft, &QSlider::sliderReleased, this, &PresetsPageWidget::sliderGlobalReleased);
    connect(ui->sliderGlobalRight, &QSlider::sliderReleased, this, &PresetsPageWidget::sliderGlobalReleased);

    // Initialize Chart
    series->clear();

    chart->setTitle("LED Panel SPD");
    chart->legend()->hide();
    chart->addSeries(series);
    chart->createDefaultAxes();
    
    // Safety check axes
    auto axesH = chart->axes(Qt::Horizontal);
    if (!axesH.isEmpty()) {
        axesH.first()->setRange(300, 780);
        axesH.first()->setTitleText("Wavelength [nm]");
    }
    
    auto axesV = chart->axes(Qt::Vertical);
    if (!axesV.isEmpty()) {
        axesV.first()->setRange(0, 1 * 1.1); // Add 10% headroom for better visibility
        axesV.first()->setTitleText("Relative SPD [-]");
    }

    chart->setVisible(true);

    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setVisible(true);

    ui->horizontalLayoutPage2->addWidget(chartView, 4);
    chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    loadSPDChart();
    refreshTable();
}

PresetsPageWidget::~PresetsPageWidget()
{
    delete ui;
}

void PresetsPageWidget::loadSPDChart()
{
    series->clear();

    QModelIndex index = ui->treeView->currentIndex();
    if (!index.isValid() || fileSystemModel->isDir(index)) {
        return;
    }

    QVector<int> values;
    if (!PresetFileManager::load(fileSystemModel->filePath(index), values)) return;

    const int wavelengthCount = ledControllerWidgetList.isEmpty()
        ? 0
        : ledControllerWidgetList.first()->getWaveLengths().size();
    QVector<double> combinedSpd(wavelengthCount, 0.0);
    for (int ledIndex = 0; ledIndex < ledControllerWidgetList.size(); ++ledIndex) {
        const double percentage = values.at(ledIndex);
        const QStringList spdValues = ledControllerWidgetList[ledIndex]->getSPDValues();
        const int pointCount = qMin(combinedSpd.size(), spdValues.size());
        for (int point = 0; point < pointCount; ++point) {
            bool spdOk = false;
            const double spd = spdValues.at(point).toDouble(&spdOk);
            if (spdOk) {
                combinedSpd[point] += spd * percentage / 100.0;
            }
        }
    }

    double maximumSpd = 0.0;
    for (double value : combinedSpd) {
        maximumSpd = qMax(maximumSpd, value);
    }
    if (maximumSpd <= 0.0) {
        return;
    }

    const QStringList wavelengths = ledControllerWidgetList.first()->getWaveLengths();
    const int pointCount = qMin(combinedSpd.size(), wavelengths.size());
    for (int point = 0; point < pointCount; ++point) {
        bool wavelengthOk = false;
        const double wavelength = wavelengths.at(point).toDouble(&wavelengthOk);
        if (wavelengthOk) {
            series->append(wavelength, combinedSpd.at(point) / maximumSpd);
        }
    }

    chart->createDefaultAxes();
    const auto axesH = chart->axes(Qt::Horizontal);
    if (!axesH.isEmpty()) {
        axesH.first()->setRange(300, 780);
        axesH.first()->setTitleText("Wavelength [nm]");
    }
    const auto axesV = chart->axes(Qt::Vertical);
    if (!axesV.isEmpty()) {
        axesV.first()->setRange(0, 1 * 1.1); // Add 10% headroom for better visibility
        axesV.first()->setTitleText("Relative SPD [-]");
    }
}

void PresetsPageWidget::refreshTable()
{
    for (int row = 0; row < ledControllerWidgetList.size(); ++row) {
        if (ui->tableWidget->item(row, 1))
            ui->tableWidget->item(row, 1)->setText(QString::number(ledControllerWidgetList[row]->leftSliderValue()) + "%");
        if (ui->tableWidget->item(row, 2))
            ui->tableWidget->item(row, 2)->setText(QString::number(ledControllerWidgetList[row]->rightSliderValue()) + "%");
    }
    loadSPDChart();
}

void PresetsPageWidget::setDimmingEnabled(bool enabled)
{
    if (enabled) {
        autoSetGlobalSliderValues();
        ui->widgetGlobalSliders->show();
        chartView->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
    } else {
        setGlobalSliderValues(100, 100);
        ui->widgetGlobalSliders->hide();
        chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
}

void PresetsPageWidget::triggerOpenPresetsFolder()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                        presetsFolderAbsolutePath,
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dirPath.isEmpty()) dirPath = dirPathPrevious;

    ui->treeView->setRootIndex(fileSystemModel->index(dirPath));
    dirPathPrevious = dirPath;
}

void PresetsPageWidget::setGlobalSliderValues(int maxValueGlobalLeft, int maxValueGlobalRight)
{
    ui->spinBoxGlobalLeft->blockSignals(true);
    ui->spinBoxGlobalRight->blockSignals(true);
    ui->sliderGlobalLeft->blockSignals(true);
    ui->sliderGlobalRight->blockSignals(true);

    ui->spinBoxGlobalLeft->setRange(0, maxValueGlobalLeft);
    ui->spinBoxGlobalRight->setRange(0, maxValueGlobalRight);
    ui->sliderGlobalLeft->setRange(0, maxValueGlobalLeft);
    ui->sliderGlobalRight->setRange(0, maxValueGlobalRight);

    ui->spinBoxGlobalLeft->setValue(100);
    ui->spinBoxGlobalRight->setValue(100);
    ui->sliderGlobalLeft->setValue(100);
    ui->sliderGlobalRight->setValue(100);

    ui->spinBoxGlobalLeft->blockSignals(false);
    ui->spinBoxGlobalRight->blockSignals(false);
    ui->sliderGlobalLeft->blockSignals(false);
    ui->sliderGlobalRight->blockSignals(false);

    emit globalDimmingChanged(100, 100);
}

void PresetsPageWidget::autoSetGlobalSliderValues()
{
    int maxValueLeftSlider = 0;
    int maxValueRightSlider = 0;
    for (auto ledControllerWidget : ledControllerWidgetList) {
        if (ledControllerWidget->leftSliderValue() > maxValueLeftSlider) maxValueLeftSlider = ledControllerWidget->leftSliderValue();
        if (ledControllerWidget->rightSliderValue() > maxValueRightSlider) maxValueRightSlider = ledControllerWidget->rightSliderValue();
    }

    if (maxValueLeftSlider != 0) { 
        maxValueLeftSlider = static_cast<int>(1/(maxValueLeftSlider*0.0001)); 
    } else { 
        maxValueLeftSlider = 999;
    }

    if (maxValueRightSlider != 0) {
        maxValueRightSlider = static_cast<int>(1/(maxValueRightSlider*0.0001));
    } else {
        maxValueRightSlider = 999;
    }

    setGlobalSliderValues(maxValueLeftSlider, maxValueRightSlider);
}

void PresetsPageWidget::treeViewIsClicked(const QModelIndex &index)
{
    if (fileSystemModel->isDir(index)) {
        ui->pushButtonLoadRightSide->setEnabled(false);
        ui->pushButtonLoadLeftSide->setEnabled(false);
        ui->pushButtonLoadBothSides->setEnabled(false);
        ui->labelFileInvalid->setText("");
    } else {
        QString filePath = fileSystemModel->filePath(index);
        QVector<int> values;
        if (PresetFileManager::load(filePath, values)) {
            ui->labelFileInvalid->setText("");
            ui->pushButtonLoadRightSide->setEnabled(true);
            ui->pushButtonLoadLeftSide->setEnabled(true);
            ui->pushButtonLoadBothSides->setEnabled(true);

            // Fetch table data based on preview
            for (int row = 0; row < ledControllerWidgetList.size(); ++row) {
                if (ui->tableWidget->item(row, 3))
                    ui->tableWidget->item(row, 3)->setText(QString::number(values.at(row)) + "%");
            }
            loadSPDChart();
        } else {
            ui->labelFileInvalid->setText("Invalid preset.");
            ui->pushButtonLoadRightSide->setEnabled(false);
            ui->pushButtonLoadLeftSide->setEnabled(false);
            ui->pushButtonLoadBothSides->setEnabled(false);
        }
    }
}

void PresetsPageWidget::tableWidgetCellIsClicked(int row, int column)
{
    // Implementation can mirror LightBoxController click selections
}

void PresetsPageWidget::pushButtonLoadRightSideIsClicked()
{
    emit disableSymmetric();
    
    QModelIndex index = ui->treeView->currentIndex();
    QString filePath = fileSystemModel->filePath(index);
    
    QVector<int> values;
    if (PresetFileManager::load(filePath, values)) {
        ui->labelFileInvalid->setText("");
        for (int row = 0; row < ledControllerWidgetList.size(); ++row) {
            // Notice: the widget directly changes right slider value, then we need to fire network
            ledControllerWidgetList[row]->rightSliderSetValue(values.at(row));
        }
        refreshTable();
        emit requestNetworkSend();
    } else {
        ui->labelFileInvalid->setText("Invalid preset.");
    }
}

void PresetsPageWidget::pushButtonDeleteRightSideIsClicked()
{
    emit disableSymmetric();
    for (auto ledControllerWidget : ledControllerWidgetList) {
        ledControllerWidget->rightSliderSetValue(0);
    }
    refreshTable();
    emit requestNetworkSend();
}

void PresetsPageWidget::pushButtonLoadLeftSideIsClicked()
{
    emit disableSymmetric();
    
    QModelIndex index = ui->treeView->currentIndex();
    QString filePath = fileSystemModel->filePath(index);
    
    QVector<int> values;
    if (PresetFileManager::load(filePath, values)) {
        ui->labelFileInvalid->setText("");
        for (int row = 0; row < ledControllerWidgetList.size(); ++row) {
            ledControllerWidgetList[row]->leftSliderSetValue(values.at(row));
        }
        refreshTable();
        emit requestNetworkSend();
    } else {
        ui->labelFileInvalid->setText("Invalid preset.");
    }
}

void PresetsPageWidget::pushButtonDeleteLeftSideIsClicked()
{
    emit disableSymmetric();
    for (auto ledControllerWidget : ledControllerWidgetList) {
        ledControllerWidget->leftSliderSetValue(0);
    }
    refreshTable();
    emit requestNetworkSend();
}

void PresetsPageWidget::pushButtonLoadBothSidesIsClicked()
{
    QModelIndex index = ui->treeView->currentIndex();
    QString filePath = fileSystemModel->filePath(index);
    
    QVector<int> values;
    if (PresetFileManager::load(filePath, values)) {
        ui->labelFileInvalid->setText("");
        for (int row = 0; row < ledControllerWidgetList.size(); ++row) {
            ledControllerWidgetList[row]->leftSliderSetValue(values.at(row));
            ledControllerWidgetList[row]->rightSliderSetValue(values.at(row));
        }
        refreshTable();
        emit requestNetworkSend();
    } else {
        ui->labelFileInvalid->setText("Invalid preset.");
    }
}

void PresetsPageWidget::pushButtonDeleteBothSidesIsClicked()
{
    emit requestReset();
    refreshTable();
    // Network send handled by reset
}

void PresetsPageWidget::pushButtonchangeSidesIsClicked()
{
    for (auto ledControllerWidget : ledControllerWidgetList) {
        int temp = ledControllerWidget->rightSliderValue();
        ledControllerWidget->rightSliderSetValue(ledControllerWidget->leftSliderValue());
        ledControllerWidget->leftSliderSetValue(temp);
    }
    refreshTable();
    emit requestNetworkSend();
}

void PresetsPageWidget::spinBoxGlobalLeftValueHasChanged(int value)
{
    ui->sliderGlobalLeft->blockSignals(true);
    ui->sliderGlobalLeft->setValue(value);
    ui->sliderGlobalLeft->blockSignals(false);
    emit globalDimmingChanged(value, ui->spinBoxGlobalRight->value());
}

void PresetsPageWidget::spinBoxGlobalRightValueHasChanged(int value)
{
    ui->sliderGlobalRight->blockSignals(true);
    ui->sliderGlobalRight->setValue(value);
    ui->sliderGlobalRight->blockSignals(false);
    emit globalDimmingChanged(ui->spinBoxGlobalLeft->value(), value);
}

void PresetsPageWidget::sliderGlobalLeftIsPressed() {}
void PresetsPageWidget::sliderGlobalRightIsPressed() {}

void PresetsPageWidget::sliderGlobalLeftValueHasChanged(int value)
{
    ui->spinBoxGlobalLeft->blockSignals(true);
    ui->spinBoxGlobalLeft->setValue(value);
    ui->spinBoxGlobalLeft->blockSignals(false);
}

void PresetsPageWidget::sliderGlobalRightValueHasChanged(int value)
{
    ui->spinBoxGlobalRight->blockSignals(true);
    ui->spinBoxGlobalRight->setValue(value);
    ui->spinBoxGlobalRight->blockSignals(false);
}

void PresetsPageWidget::sliderGlobalReleased()
{
    emit globalDimmingChanged(ui->sliderGlobalLeft->value(), ui->sliderGlobalRight->value());
}

