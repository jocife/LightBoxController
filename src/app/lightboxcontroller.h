#pragma once

#include <QMainWindow>
#include <QActionGroup>
#include "connectionpagewidget.h"
#include "ledcontrolpagewidget.h"
#include "presetspagewidget.h"
#include "cie1976pagewidget.h"
#include "sunlightsimulatorpagewidget.h"
#include "configurationdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LightBoxController; }
QT_END_NAMESPACE

class LightBoxController : public QMainWindow
{
    Q_OBJECT

public:
    LightBoxController(QWidget *parent = nullptr);
    ~LightBoxController();

    QString getThemeFileAbsolutePath();
    
public slots:
    void actionExit();
    void showConfigurationDialog();
    void changeStakedWidgetPage();
    void logMessage(const QString& sender, const QString& message);

private:
    Ui::LightBoxController *ui;

    // Helper Paths
    QString configsFolderAbsolutePath;
    QString ledSPDsFolderAbsolutePath;
    QString presetsFolderAbsolutePath;
    QString themesFolderAbsolutePath;
    QString themeFileAbsolutePath;
    QString imagesFolderAbsolutePath;
    QString iconsFolderAbsolutePath;
    QString logsFolderAbsolutePath;
    QString logFilePath;
    ConfigurationValues configurationValues;

    // Sub-Pages
    ConnectionPageWidget *connectionPageWidget;
    LedControlPageWidget *ledControlPageWidget;
    PresetsPageWidget *presetsPageWidget;
    Cie1976PageWidget *cie1976PageWidget;
    SunlightSimulatorPageWidget *sunlightSimulatorPageWidget;

    // Actions
    QActionGroup *pageSelector;
};
