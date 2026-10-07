#ifndef CIE1976PAGEWIDGET_H
#define CIE1976PAGEWIDGET_H

#include <QWidget>
#include <QVector>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include "optimization_runner.h"

namespace Ui {
class Cie1976PageWidget;
}

class Cie1976PageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit Cie1976PageWidget(const QString& imagesFolderAbsolutePath, const QString& calibrationFile,
                               QWidget *parent = nullptr);
    ~Cie1976PageWidget();

public slots:
    void triggerCalculate();
    void triggerSave();
    void setCalibrationFile(const QString& filePath);

signals:
    void requestNetworkSend();
    void logMessage(const QString& source, const QString& message);
    
private:
    Ui::Cie1976PageWidget *ui;

    QString imagesFolderAbsolutePath;
    QString calibrationFilePath;

    QLineSeries *seriesCIE;
    QChart *chartCIE;
    QChartView *chartViewCIE;
    QValueAxis *axisX;
    QValueAxis *axisY;
    QVector<double> lastWeights;

    void populateLeds();
    QVector<int> selectedLedIndices() const;
    void updateChart(const QVector<double>& wavelengths, const QVector<double>& spectrum);
};

#endif // CIE1976PAGEWIDGET_H
