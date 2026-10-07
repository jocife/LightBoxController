#include "lightboxcontroller.h"
#include "ui_lightboxcontroller.h"
#include <QCoreApplication>
#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QDebug>
#include <QRegularExpression>
#include <QSettings>

LightBoxController::LightBoxController(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LightBoxController)
{
    ui->setupUi(this);

    // Set paths
    QString executableAbsolutePath = QCoreApplication::applicationDirPath();
    imagesFolderAbsolutePath = QDir(executableAbsolutePath).absoluteFilePath("images");
    iconsFolderAbsolutePath = QDir(executableAbsolutePath).absoluteFilePath("icons");
    themesFolderAbsolutePath = QDir(executableAbsolutePath).absoluteFilePath("themes");
    themeFileAbsolutePath = QDir(themesFolderAbsolutePath).absoluteFilePath("Diffnes.qss");
    configsFolderAbsolutePath = QDir(executableAbsolutePath).absoluteFilePath("configs");
    ledSPDsFolderAbsolutePath = QDir(configsFolderAbsolutePath).absoluteFilePath("LED_SPDs");
    logsFolderAbsolutePath = QDir(executableAbsolutePath).absoluteFilePath("logs");
    presetsFolderAbsolutePath = QDir(executableAbsolutePath).absoluteFilePath("presets");

    QSettings settings("BME", "LED_GUI");
    configurationValues.ledSpdFolder = settings.value("ledSpdFolder", ledSPDsFolderAbsolutePath).toString();
    if (!QDir(configurationValues.ledSpdFolder).exists()) {
        configurationValues.ledSpdFolder = ledSPDsFolderAbsolutePath;
        settings.setValue("ledSpdFolder", configurationValues.ledSpdFolder);
    }
    configurationValues.calibrationFile = settings.value(
        "calibrationFile", QDir(configsFolderAbsolutePath).absoluteFilePath("LED_SPD_CALIBRATION.csv")).toString();
    configurationValues.wifiSsid = settings.value("wifiSsid", "LightBooth-WiFi").toString();
    configurationValues.wifiPassword = settings.value("wifiPassword", "thereisnospoon").toString();
    configurationValues.hostAddress = settings.value("hostAddress", "192.168.4.1").toString();
    configurationValues.hostPort = settings.value("hostPort", 5001).toInt();

    logFilePath = QDir(logsFolderAbsolutePath).absoluteFilePath(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + ".txt");
    
    // Create new log file
    QFile file(logFilePath);
    if(file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream textStream(&file);
        textStream << "Start Logging..." << Qt::endl;
        file.close();
    }

    // Instantiate Pages
    connectionPageWidget = new ConnectionPageWidget(imagesFolderAbsolutePath, configurationValues.hostAddress,
                                                    configurationValues.hostPort, configurationValues.wifiSsid,
                                                    configurationValues.wifiPassword, this);
    ledControlPageWidget = new LedControlPageWidget(configsFolderAbsolutePath, presetsFolderAbsolutePath,
                                                    configurationValues.ledSpdFolder, this);
    presetsPageWidget = new PresetsPageWidget(presetsFolderAbsolutePath, iconsFolderAbsolutePath, ledControlPageWidget->getLedControllers(), this);
    cie1976PageWidget = new Cie1976PageWidget(imagesFolderAbsolutePath, configurationValues.calibrationFile, this);
    sunlightSimulatorPageWidget = new SunlightSimulatorPageWidget(configurationValues.calibrationFile, this);

    // Emplace inside ui->stackedWidget
    ui->stackedWidget->insertWidget(0, connectionPageWidget);
    ui->stackedWidget->insertWidget(1, ledControlPageWidget);
    ui->stackedWidget->insertWidget(2, presetsPageWidget);
    ui->stackedWidget->insertWidget(3, cie1976PageWidget);
    ui->stackedWidget->insertWidget(4, sunlightSimulatorPageWidget);

    // Initial setup for stacked widget
    ui->stackedWidget->setCurrentIndex(0);

    // Connect Log messages
    connect(connectionPageWidget, &ConnectionPageWidget::logMessage, this, &LightBoxController::logMessage);
    connect(ledControlPageWidget, &LedControlPageWidget::logMessage, this, &LightBoxController::logMessage);
    connect(presetsPageWidget, &PresetsPageWidget::logMessage, this, &LightBoxController::logMessage);
    connect(cie1976PageWidget, &Cie1976PageWidget::logMessage, this, &LightBoxController::logMessage);
    connect(sunlightSimulatorPageWidget, &SunlightSimulatorPageWidget::logMessage, this, &LightBoxController::logMessage);

    // Connect Orchestration
    connect(ledControlPageWidget, &LedControlPageWidget::requestNetworkSend, connectionPageWidget, &ConnectionPageWidget::sendData);
    connect(presetsPageWidget, &PresetsPageWidget::requestNetworkSend, ledControlPageWidget, &LedControlPageWidget::triggerSend);
    connect(presetsPageWidget, &PresetsPageWidget::globalDimmingChanged, ledControlPageWidget, &LedControlPageWidget::setGlobalDimming);
    connect(presetsPageWidget, &PresetsPageWidget::requestReset, ledControlPageWidget, &LedControlPageWidget::triggerReset);
    connect(presetsPageWidget, &PresetsPageWidget::disableSymmetric, this, [this]() {
        ui->actionSymmetric->setChecked(false);
    });

    // Actions grouping
    pageSelector = new QActionGroup(this);
    pageSelector->addAction(ui->actionConnection);
    pageSelector->addAction(ui->actionLedControl);
    pageSelector->addAction(ui->actionPresets);
    pageSelector->addAction(ui->actionCIE1976);
    pageSelector->addAction(ui->actionSunlightSimulator);
    
    // UI Connections
    connect(ui->actionExit, &QAction::triggered, this, &LightBoxController::actionExit);
    connect(ui->actionConfiguration, &QAction::triggered, this, &LightBoxController::showConfigurationDialog);
    connect(ui->actionConnection, &QAction::toggled, this, &LightBoxController::changeStakedWidgetPage);
    connect(ui->actionLedControl, &QAction::toggled, this, &LightBoxController::changeStakedWidgetPage);
    connect(ui->actionPresets, &QAction::toggled, this, &LightBoxController::changeStakedWidgetPage);
    connect(ui->actionCIE1976, &QAction::toggled, this, &LightBoxController::changeStakedWidgetPage);
    connect(ui->actionSunlightSimulator, &QAction::toggled, this, &LightBoxController::changeStakedWidgetPage);

    // Explicitly connect remaining Toolbar commands if handled inside Tabs
    connect(ui->actionSend, &QAction::triggered, ledControlPageWidget, &LedControlPageWidget::triggerSend);
    connect(ui->actionSave, &QAction::triggered, ledControlPageWidget, &LedControlPageWidget::triggerSave);
    connect(ui->actionLoad, &QAction::triggered, ledControlPageWidget, &LedControlPageWidget::triggerLoad);
    connect(ui->actionReset, &QAction::triggered, ledControlPageWidget, &LedControlPageWidget::triggerReset);

    // Additional LedControl Actions
    connect(ui->actionAutoSend, &QAction::toggled, ledControlPageWidget, &LedControlPageWidget::setAutoSendEnabled);
    connect(ui->actionSymmetric, &QAction::toggled, ledControlPageWidget, &LedControlPageWidget::triggerSymmetric);
    connect(ui->actionSymmetric, &QAction::toggled, this, [this](bool symmetric) {
        ui->actionSaveLeft->setVisible(!symmetric);
        ui->actionSaveRight->setVisible(!symmetric);
        ui->actionLoadLeft->setVisible(!symmetric);
        ui->actionLoadRight->setVisible(!symmetric);
    });
    connect(ui->actionSaveLeft, &QAction::triggered, ledControlPageWidget, &LedControlPageWidget::triggerSaveLeft);
    connect(ui->actionSaveRight, &QAction::triggered, ledControlPageWidget, &LedControlPageWidget::triggerSaveRight);
    connect(ui->actionLoadLeft, &QAction::triggered, ledControlPageWidget, &LedControlPageWidget::triggerLoadLeft);
    connect(ui->actionLoadRight, &QAction::triggered, ledControlPageWidget, &LedControlPageWidget::triggerLoadRight);

    // Presets Page Actions
    connect(ui->actionOpenPresetsFolder, &QAction::triggered, presetsPageWidget, &PresetsPageWidget::triggerOpenPresetsFolder);
    connect(ui->actionDimming, &QAction::toggled, presetsPageWidget, &PresetsPageWidget::setDimmingEnabled);

    // Log Actions
    ui->actionShowLog->setChecked(false);
    connect(ui->actionShowLog, &QAction::toggled, ui->logPanel, &QTextBrowser::setVisible);
    ui->logPanel->setVisible(ui->actionShowLog->isChecked());

    // Initialize LED Control Page menu elements like in the previous version
    ui->actionAutoSend->trigger();
    ui->actionSymmetric->trigger();

    // Set initial state
    ui->actionConnection->setChecked(true);
    changeStakedWidgetPage();

    // Initial connection attempt
    connectionPageWidget->pushButtonConnectIsClicked();
}

LightBoxController::~LightBoxController()
{
    delete ui;
}

QString LightBoxController::getThemeFileAbsolutePath()
{
    return themeFileAbsolutePath;
}

void LightBoxController::actionExit()
{
    QCoreApplication::quit();
}

void LightBoxController::showConfigurationDialog()
{
    ConfigurationDialog dialog(configurationValues, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    configurationValues = dialog.values();
    if (!QDir(configurationValues.ledSpdFolder).exists()) {
        configurationValues.ledSpdFolder = ledSPDsFolderAbsolutePath;
    }
    QSettings settings("BME", "LED_GUI");
    settings.setValue("ledSpdFolder", configurationValues.ledSpdFolder);
    settings.setValue("calibrationFile", configurationValues.calibrationFile);
    settings.setValue("wifiSsid", configurationValues.wifiSsid);
    settings.setValue("wifiPassword", configurationValues.wifiPassword);
    settings.setValue("hostAddress", configurationValues.hostAddress);
    settings.setValue("hostPort", configurationValues.hostPort);

    connectionPageWidget->setConnectionSettings(configurationValues.hostAddress,
                                                 configurationValues.hostPort,
                                                 configurationValues.wifiSsid,
                                                 configurationValues.wifiPassword);
    ledControlPageWidget->setLedSPDsFolder(configurationValues.ledSpdFolder);
    cie1976PageWidget->setCalibrationFile(configurationValues.calibrationFile);
    sunlightSimulatorPageWidget->setCalibrationFile(configurationValues.calibrationFile);
    logMessage("Configuration", "Configuration updated.");
}

void LightBoxController::changeStakedWidgetPage()
{
    if (ui->actionConnection->isChecked()) {
        ui->stackedWidget->setCurrentIndex(0);
        ui->menuLEDControl->setEnabled(false);
        ui->menuPresets->setEnabled(false);
        ui->menuCIE1976->setEnabled(false);
        ui->menuSunlightSimulator->setEnabled(false);
    } else if (ui->actionLedControl->isChecked()) {
        ui->stackedWidget->setCurrentIndex(1);
        ui->menuLEDControl->setEnabled(true);
        ui->menuPresets->setEnabled(false);
        ui->menuCIE1976->setEnabled(false);
        ui->menuSunlightSimulator->setEnabled(false);
    } else if (ui->actionPresets->isChecked()) {
        ui->stackedWidget->setCurrentIndex(2);
        ui->menuLEDControl->setEnabled(false);
        ui->menuPresets->setEnabled(true);
        ui->menuCIE1976->setEnabled(false);
        ui->menuSunlightSimulator->setEnabled(false);
    } else if (ui->actionCIE1976->isChecked()) {
        ui->stackedWidget->setCurrentIndex(3);
        ui->menuLEDControl->setEnabled(false);
        ui->menuPresets->setEnabled(false);
        ui->menuCIE1976->setEnabled(true);
        ui->menuSunlightSimulator->setEnabled(false);
    } else if (ui->actionSunlightSimulator->isChecked()) {
        ui->stackedWidget->setCurrentIndex(4);
        ui->menuLEDControl->setEnabled(false);
        ui->menuPresets->setEnabled(false);
        ui->menuCIE1976->setEnabled(false);
        ui->menuSunlightSimulator->setEnabled(true);
    }
}

void LightBoxController::logMessage(const QString& sender, const QString& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString displaySender = sender;
    if (sender == "ConnectionPage") {
        displaySender = "Connection";
    } else if (sender == "LedControl") {
        displaySender = "LED Control";
    } else if (sender == "CIE1976") {
        displaySender = "CIE 1976";
    } else if (sender == "SunlightSimulatorPage") {
        displaySender = "Sunlight Simulator";
    } else if (sender == "PresetsPage") {
        displaySender = "Presets";
    }
    QString logLine = timestamp + " - [" + displaySender + "] " + message;

    QFile file(logFilePath);
    if(file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream textStream(&file);
        textStream << logLine << Qt::endl;
        file.close();
    }
    qDebug() << logLine;
    ui->logPanel->append(logLine);

    const QString plainMessage = QString(message).remove(QRegularExpression("<[^>]*>"));
    const QString warningText = plainMessage.toLower();
    if (warningText.contains("error")
        || warningText.contains("fail")
        || warningText.contains("not connected")
        || warningText.contains("select at least")
        || warningText.contains("before saving")
        || warningText.contains("invalid")) {
        ui->actionShowLog->setChecked(true);
    }
}
