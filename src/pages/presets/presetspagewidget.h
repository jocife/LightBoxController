#ifndef PRESETSPAGEWIDGET_H
#define PRESETSPAGEWIDGET_H

#include <QWidget>
#include <QFileSystemModel>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QTableWidgetItem>
#include "ledcontrollerwidget.h"

namespace Ui {
class PresetsPageWidget;
}

class PresetsPageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PresetsPageWidget(const QString& presetsFolder, const QString& iconsFolder, const QList<LEDControllerWidget*>& ledControllers, QWidget *parent = nullptr);
    ~PresetsPageWidget();

    void loadSPDChart();
    void setGlobalSliderValues(int maxValueGlobalLeft, int maxValueGlobalRight);
    void autoSetGlobalSliderValues();

    void setDimmingEnabled(bool enabled);

public slots:
    void triggerOpenPresetsFolder();

signals:
    void globalDimmingChanged(int left, int right);
    void requestNetworkSend();
    void requestReset();
    void disableSymmetric();
    void logMessage(const QString& source, const QString& message);

public slots:
    void refreshTable();

private slots:
    void treeViewIsClicked(const QModelIndex &index);

    void tableWidgetCellIsClicked(int row, int column);
    
    void pushButtonLoadRightSideIsClicked();
    void pushButtonDeleteRightSideIsClicked();

    void pushButtonLoadLeftSideIsClicked();
    void pushButtonDeleteLeftSideIsClicked();

    void pushButtonLoadBothSidesIsClicked();
    void pushButtonDeleteBothSidesIsClicked();

    void pushButtonchangeSidesIsClicked();

    void spinBoxGlobalLeftValueHasChanged(int value);
    void spinBoxGlobalRightValueHasChanged(int value);

    void sliderGlobalLeftIsPressed();
    void sliderGlobalRightIsPressed();

    void sliderGlobalLeftValueHasChanged(int value);
    void sliderGlobalRightValueHasChanged(int value);

    void sliderGlobalReleased();

private:
    void updateSliderGlobalLeftValue(int value);
    void updateSliderGlobalRightValue(int value);

    Ui::PresetsPageWidget *ui;

    QString presetsFolderAbsolutePath;
    QString iconsFolderAbsolutePath;
    QString dirPathPrevious;

    QFileSystemModel *fileSystemModel;

    // LED Controllers (reference from LedControl tab for getting value/names without hardcoding)
    QList<LEDControllerWidget*> ledControllerWidgetList;

    // Charting
    QSplineSeries *series;
    QChart *chart;
    QChartView *chartView;
};

#endif // PRESETSPAGEWIDGET_H
