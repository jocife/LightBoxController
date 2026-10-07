#ifndef LEDCONTROLPAGEWIDGET_H
#define LEDCONTROLPAGEWIDGET_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QTimer>
#include <QDir>
#include <QFileDialog>
#include "ledcontrollerwidget.h"

namespace Ui {
class LedControlPageWidget;
}

class LedControlPageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LedControlPageWidget(const QString& configsFolderAbsolutePath, const QString& presetsFolderAbsolutePath,
                                  const QString& ledSpdFolderAbsolutePath, QWidget *parent = nullptr);
    ~LedControlPageWidget();

    // Expose widgets for other tabs (like Presets & CIE1976) that need to read from/write to them
    const QList<LEDControllerWidget*>& getLedControllers() const { return ledControllerWidgetList; }

    void setGlobalDimming(int left, int right);

    bool isAutoSendEnabled() const { return autoSendEnabled; }
    void setAutoSendEnabled(bool enabled);

public slots:
    void setLedSPDsFolder(const QString& folderPath);
    void triggerSend();
    void triggerSymmetric(bool symmetric);
    
    void triggerSave();
    void triggerSaveLeft();
    void triggerSaveRight();
    
    void triggerLoad();
    void triggerLoadLeft();
    void triggerLoadRight();
    
    void triggerReset();

signals:
    void logMessage(const QString& source, const QString& message);
    void requestNetworkSend(const QByteArray& data);
    void autoSendStatusChanged(bool enabled);
    void ledValuesChanged();

private slots:
    void onSliderReleased();

private:
    void readLedSPDs();

    Ui::LedControlPageWidget *ui;

    QList<LEDControllerWidget *> ledControllerWidgetList;
    
    QString configsFolderPath;
    QString ledSPDsFolderAbsolutePath;
    QString presetsFolderPath;

    int globalLeftValue;
    int globalRightValue;
    bool autoSendEnabled;
};

#endif // LEDCONTROLPAGEWIDGET_H
