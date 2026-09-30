#ifndef SUNLIGHTSIMULATORPAGEWIDGET_H
#define SUNLIGHTSIMULATORPAGEWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QFileDialog>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include "../../optimization/optimization_runner.h"

namespace Ui {
class SunlightSimulatorPageWidget;
}

class SunlightSimulatorPageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SunlightSimulatorPageWidget(QWidget *parent = nullptr);
    ~SunlightSimulatorPageWidget();

public slots:
    void triggerCalculate();
    void triggerSave();

signals:
    // Forwards log messages to the main controller's log panel
    void logMessage(const QString& source, const QString& message);

private:
    Ui::SunlightSimulatorPageWidget *ui;
    
    QChart *m_chart;
    QLineSeries *m_testSeries;
    QLineSeries *m_refSeries;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;

    // Store the last evaluated metrics so the UI can replot without re-running
    // the MATLAB evaluator when toggling normalization.
    EvaluatedMetrics m_lastMetrics;
    QVector<double> m_lastWeights;
    QVector<int> m_lastSelectedLedIndices;

    void populateLeds();
    QVector<int> selectedLedIndices() const;
    void setupChart();
    void updateChart();
    void updateLabels(double mu, double mv, double duv, double cri, double cct);
};

#endif // SUNLIGHTSIMULATORPAGEWIDGET_H
