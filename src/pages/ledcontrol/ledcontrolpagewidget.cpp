#include "ledcontrolpagewidget.h"
#include "ui_ledcontrolpagewidget.h"
#include "presetfilemanager.h"
#include <QDebug>

LedControlPageWidget::LedControlPageWidget(const QString& configsFolder, const QString& presetsFolder,
                                           const QString& ledSpdFolder, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LedControlPageWidget)
    , configsFolderPath(configsFolder)
    , presetsFolderPath(presetsFolder)
    , ledSPDsFolderAbsolutePath(ledSpdFolder)
    , globalLeftValue(100)
    , globalRightValue(100)
    , autoSendEnabled(false)
{
    ui->setupUi(this);

    QStringList ledNames = {
        "Violet", "Royal Blue", "Blue", "Cyan", "Green", "Lime", "Mint", 
        "PC Amber", "Amber", "Red Orange", "Red", "Deep Red", "Far Red",
        "UV 345", "UV 365", "UV 385", "UV 395", "UV 405", "UV 415", 
        "Warm White", "Neutral White", "Cool White"
    };

    for (int i = 0; i < ledNames.size(); ++i) {
        LEDControllerWidget* widget = new LEDControllerWidget(this, ledNames[i]);
        ledControllerWidgetList.append(widget);
        
        // Add to grid layoyt: 11 per row
        int row = i / 11;
        int col = i % 11;
        ui->gridLayoutLedControllerWidgets->addWidget(widget, row, col);

        connect(widget, SIGNAL(sliderReleased()), this, SLOT(onSliderReleased()));
    }

    readLedSPDs();
}

LedControlPageWidget::~LedControlPageWidget()
{
    delete ui;
}

void LedControlPageWidget::setLedSPDsFolder(const QString& folderPath)
{
    ledSPDsFolderAbsolutePath = folderPath;
    if (!QDir(ledSPDsFolderAbsolutePath).exists()) {
        ledSPDsFolderAbsolutePath = QDir(configsFolderPath).absoluteFilePath("LED_SPDs");
    }
    readLedSPDs();
}

void LedControlPageWidget::readLedSPDs()
{
    QString fileName = "";
    bool validFile = false;
    bool allFilesLoadedSuccessfully = true;

    for (auto ledControllerWidget : ledControllerWidgetList) {
        fileName = ledControllerWidget->labelText().replace(" ", "_") + "_SPD.csv";
        validFile = ledControllerWidget->readSPDDataFromCSV(ledSPDsFolderAbsolutePath + "/" + fileName, ';');
        
        if (!validFile) {
            emit logMessage("LED Control", QString("<span style='color:red'>Error reading file: %1</span>").arg(fileName));
            allFilesLoadedSuccessfully = false;
        }
    }
    
    if (allFilesLoadedSuccessfully) {
        emit logMessage("LED Control", "<span style='color:green'>All LED SPD CSV files successfully read in.</span>");
    }
}

void LedControlPageWidget::setGlobalDimming(int left, int right)
{
    globalLeftValue = left;
    globalRightValue = right;

    if (autoSendEnabled) {
        triggerSend();
    }
    emit ledValuesChanged();
}

void LedControlPageWidget::setAutoSendEnabled(bool enabled)
{
    autoSendEnabled = enabled;
    emit autoSendStatusChanged(enabled);
}

void LedControlPageWidget::triggerSend()
{
    QString message = "D|65535|";
    QString messageLeftSliderValues = "";
    QString messageRightSliderValues = "";
    double messageLeftSliderValue = 0.0;
    double messageRightSliderValue = 0.0;

    for (auto ledControllerWidget : ledControllerWidgetList) {
        messageLeftSliderValue = ledControllerWidget->leftSliderValue() * globalLeftValue * 0.01;
        messageRightSliderValue = ledControllerWidget->rightSliderValue() * globalRightValue * 0.01;

        messageLeftSliderValues.append(QString("%1|").arg(static_cast<int>(round(messageLeftSliderValue * 655.35)), 5, 10, QChar('0')));
        messageRightSliderValues.append(QString("%1|").arg(static_cast<int>(round(messageRightSliderValue * 655.35)), 5, 10, QChar('0')));
    }

    message.append(messageRightSliderValues);
    message.append(messageLeftSliderValues);
    message.append("E");

    emit requestNetworkSend(message.toUtf8());
}

void LedControlPageWidget::triggerSymmetric(bool symmetric)
{
    if (symmetric) {
        for (auto ledControllerWidget : ledControllerWidgetList) {
            ledControllerWidget->rightSliderSetValue(ledControllerWidget->leftSliderValue());
            ledControllerWidget->rightSliderSetEnabled(false);
            ledControllerWidget->spinBoxSetPrefix("");
            ledControllerWidget->spinBoxRefreshValue();
        }
    } else {
        for (auto ledControllerWidget : ledControllerWidgetList) {
            ledControllerWidget->spinBoxSetPrefix("L ");
            ledControllerWidget->rightSliderSetEnabled(true);
        }
    }
    emit ledValuesChanged();
}

void LedControlPageWidget::triggerSave()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Both Sides Preset"),
                                                    presetsFolderPath + "/untitled.xml",
                                                    tr("XML (*.xml);;All Files (*)"));
    if (filePath.isEmpty()) return;

    QVector<int> values;
    for (auto ledControllerWidget : ledControllerWidgetList)
        values.append(ledControllerWidget->leftSliderValue());

    QString errorMessage;
    if (!PresetFileManager::save(filePath, values, &errorMessage)) {
        emit logMessage("LED Control", "<span style='color:red'>Failed to open file for saving.</span>");
        return;
    }
    emit logMessage("LED Control", "Saved preset: " + filePath);
}

void LedControlPageWidget::triggerSaveLeft()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Left Side Preset"),
                                                    presetsFolderPath + "/untitled.xml",
                                                    tr("XML (*.xml);;All Files (*)"));
    if (filePath.isEmpty()) return;

    QVector<int> values;
    for (auto ledControllerWidget : ledControllerWidgetList)
        values.append(ledControllerWidget->leftSliderValue());

    if (!PresetFileManager::save(filePath, values)) return;
    emit logMessage("LED Control", "Saved left preset: " + filePath);
}

void LedControlPageWidget::triggerSaveRight()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Right Side Preset"),
                                                    presetsFolderPath + "/untitled.xml",
                                                    tr("XML (*.xml);;All Files (*)"));
    if (filePath.isEmpty()) return;

    QVector<int> values;
    for (auto ledControllerWidget : ledControllerWidgetList)
        values.append(ledControllerWidget->rightSliderValue());

    if (!PresetFileManager::save(filePath, values)) return;
    emit logMessage("LED Control", "Saved right preset: " + filePath);
}

void LedControlPageWidget::triggerLoad()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Load Preset"),
                                                    presetsFolderPath,
                                                    tr("XML (*.xml);;All Files (*)"));
    if (filePath.isEmpty()) return;

    QVector<int> values;
    if (!PresetFileManager::load(filePath, values)) return;
    for (int row = 0; row < ledControllerWidgetList.size(); ++row)
        ledControllerWidgetList[row]->leftSliderSetValue(values.at(row));

    emit ledValuesChanged();
    emit logMessage("LED Control", "Loaded preset: " + filePath);
    if (autoSendEnabled) triggerSend();
}

void LedControlPageWidget::triggerLoadLeft()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Load Left Preset"),
                                                    presetsFolderPath,
                                                    tr("XML (*.xml);;All Files (*)"));
    if (filePath.isEmpty()) return;

    QVector<int> values;
    if (!PresetFileManager::load(filePath, values)) return;
    for (int row = 0; row < ledControllerWidgetList.size(); ++row)
        ledControllerWidgetList[row]->leftSliderSetValue(values.at(row));

    emit ledValuesChanged();
    if (autoSendEnabled) triggerSend();
}

void LedControlPageWidget::triggerLoadRight()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Load Right Preset"),
                                                    presetsFolderPath,
                                                    tr("XML (*.xml);;All Files (*)"));
    if (filePath.isEmpty()) return;

    QVector<int> values;
    if (!PresetFileManager::load(filePath, values)) return;
    for (int row = 0; row < ledControllerWidgetList.size(); ++row)
        ledControllerWidgetList[row]->rightSliderSetValue(values.at(row));

    emit ledValuesChanged();
    if (autoSendEnabled) triggerSend();
}

void LedControlPageWidget::triggerReset()
{
    for (auto ledControllerWidget : ledControllerWidgetList) {
        ledControllerWidget->resetValues();
    }
    
    emit ledValuesChanged();
    if (autoSendEnabled) triggerSend();
}

void LedControlPageWidget::onSliderReleased()
{
    emit ledValuesChanged();
    if (autoSendEnabled) {
        triggerSend();
    }
}
