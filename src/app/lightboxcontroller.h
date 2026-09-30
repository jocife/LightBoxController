#pragma once

#include <QMainWindow>
#include <QActionGroup>
#include "connectionpagewidget.h"
#include "ledcontrolpagewidget.h"
#include "presetspagewidget.h"
#include "cie1931pagewidget.h"
#include "sunlightsimulatorpagewidget.h"

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

    // Sub-Pages
    ConnectionPageWidget *connectionPageWidget;
    LedControlPageWidget *ledControlPageWidget;
    PresetsPageWidget *presetsPageWidget;
    Cie1931PageWidget *cie1931PageWidget;
    SunlightSimulatorPageWidget *sunlightSimulatorPageWidget;

    // Actions
    QActionGroup *pageSelector;
};
