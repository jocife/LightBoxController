#ifndef LEDCONTROLLERWIDGET_H
#define LEDCONTROLLERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QSpinBox>
#include <QSlider>
#include <QGridLayout>
#include <QTableWidget>
#include <QFile>

class LEDControllerWidget : public QWidget {
    Q_OBJECT

public:
    LEDControllerWidget(QWidget *parent = nullptr, QString labelText = "LED Name");

    QString labelText() const;

    int leftSliderValue() const;
    int rightSliderValue() const;

    bool readSPDDataFromCSV(QString filePath, char sep = ';');

    QStringList getWaveLengths() const;
    QStringList getSPDValues() const;

public slots:
    void spinBoxValueHasChanged(int value);
    void spinBoxSetPrefix(QString prefix);
    void spinBoxRefreshValue();

    void leftSliderIsPressed();
    void rightSliderIsPressed();

    void leftSliderValueHasChanged(int value);
    void rightSliderValueHasChanged(int value);

    void emitSignalLeftSliderPressed();
    void emitSignalRightSliderPressed();

    void leftSliderSetValue(int value);
    void rightSliderSetValue(int value);

    void rightSliderSetEnabled(bool enabled);
    void resetValues();

signals:
    void valueChanged(int value);

    // 1. New signal for sliderReleased
    void sliderReleased();

private:
    QLabel *label;
    QSpinBox *spinBox;
    QSlider *leftSlider;
    QSlider *rightSlider;
    QGridLayout *gridLayout;

    QStringList waveLengths;
    QStringList spdValues;
};

#endif // LEDCONTROLLERWIDGET_H
