#ifndef CIE1931PAGEWIDGET_H
#define CIE1931PAGEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QImage>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>

#include "optimization_runner.h"

namespace Ui {
class Cie1931PageWidget;
}

class Cie1931PageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit Cie1931PageWidget(const QString& imagesFolderAbsolutePath, QWidget *parent = nullptr);
    ~Cie1931PageWidget();

public slots:
    void triggerCalculate();

signals:
    void requestNetworkSend();
    void logMessage(const QString& source, const QString& message);
    
private:
    Ui::Cie1931PageWidget *ui;

    QString imagesFolderAbsolutePath;

    QSplineSeries *seriesCIE;
    QChart *chartCIE;
    QChartView *chartViewCIE;
    
    QImage *imageCie1931Diagram;
    QLabel *labelDisplayCie1931Diagram;
};

#endif // CIE1931PAGEWIDGET_H
