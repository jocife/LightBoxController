#include "ledcontrollerwidget.h"

LEDControllerWidget::LEDControllerWidget(QWidget *parent, QString labelText) : QWidget(parent)
    , label(new QLabel())
    , spinBox(new QSpinBox())
    , leftSlider(new QSlider())
    , rightSlider(new QSlider())
    , gridLayout(new QGridLayout())
{
    gridLayout->addWidget(label, 0, 0, 1, -1, Qt::AlignHCenter);
    gridLayout->addWidget(spinBox, 1, 0, 1, -1);
    gridLayout->addWidget(leftSlider, 2, 0);
    gridLayout->addWidget(rightSlider, 2, 1);
    setLayout(gridLayout);

    label->setText(labelText);

    spinBox->setAlignment(Qt::AlignHCenter);
    spinBox->setPrefix("L ");
    spinBox->setSuffix("%");
    spinBox->setRange(0, 100);
    spinBox->setKeyboardTracking(false);

    leftSlider->setRange(0, 100);
    rightSlider->setRange(0, 100);

    connect(spinBox, SIGNAL(valueChanged(int)), this, SLOT(spinBoxValueHasChanged(int)));

    connect(leftSlider, SIGNAL(sliderPressed()), this, SLOT(leftSliderIsPressed()));
    connect(rightSlider, SIGNAL(sliderPressed()), this, SLOT(rightSliderIsPressed()));

    connect(leftSlider, SIGNAL(valueChanged(int)), this, SLOT(leftSliderValueHasChanged(int)));
    connect(rightSlider, SIGNAL(valueChanged(int)), this, SLOT(rightSliderValueHasChanged(int)));

    // 2. Connect the sliders' sliderReleased signals to the widget's sliderReleased signal
    connect(leftSlider, SIGNAL(sliderReleased()), this, SIGNAL(sliderReleased()));
    connect(rightSlider, SIGNAL(sliderReleased()), this, SIGNAL(sliderReleased()));

}

/*
 *  Public functions
 */

QString LEDControllerWidget::labelText() const {
    return label->text();
}

int LEDControllerWidget::leftSliderValue() const {
    return leftSlider->value();
}

int LEDControllerWidget::rightSliderValue() const {
    return rightSlider->value();
}

bool LEDControllerWidget::readSPDDataFromCSV(QString filePath, char sep) {
    QFile csvFile(filePath);
    if (!csvFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error while loading file | Error: " << csvFile.error();
        return false;
    }

    while (!csvFile.atEnd()) {
        QByteArray line = csvFile.readLine();
        this->waveLengths.append(line.split(sep).first());
        this->spdValues.append(line.split(sep)[1]);
        if (spdValues.last().endsWith('\n')) spdValues.last().chop(1);
    }
    csvFile.close();

    this->waveLengths.remove(0); // Date: 2024.11.19.
    this->spdValues.remove(0); // Comments: e.g. instrument data

    this->waveLengths.remove(0); // Wavelength [nm]
    this->spdValues.remove(0); // Violet SPD [W/m^2/nm]

    return true;
}

QStringList LEDControllerWidget::getWaveLengths() const {
    return this->waveLengths;
}

QStringList LEDControllerWidget::getSPDValues() const {
    return this->spdValues;
}

/*
 *  Public slots
 */

void LEDControllerWidget::spinBoxValueHasChanged(int value) {
    if (spinBox->prefix() == "L " || spinBox->prefix() == "") {
        if (leftSlider->value() != value) {
            leftSlider->setValue(value);
            emit sliderReleased();
        }
    }
    else { // spinBox->prefix() == "R "
        if (rightSlider->value() != value) {
            rightSlider->setValue(value);
            emit sliderReleased();
        }
    }
    emit valueChanged(value);
}

void LEDControllerWidget::spinBoxSetPrefix(QString prefix) {
    spinBox->setPrefix(prefix);
}

void LEDControllerWidget::spinBoxRefreshValue() {
    spinBox->setValue(leftSlider->value());
}

void LEDControllerWidget::leftSliderIsPressed() {
    if (rightSlider->isEnabled()) {
        spinBox->setPrefix("L ");
    }
    else {
        spinBox->setPrefix("");
    }
    spinBox->setValue(leftSlider->value());
    // qDebug() << "sliderPressedLeft";
}

void LEDControllerWidget::rightSliderIsPressed() {
    spinBox->setPrefix("R ");
    spinBox->setValue(rightSlider->value());
    // qDebug() << "sliderPressedRight";
}

void LEDControllerWidget::leftSliderValueHasChanged(int value) {
    if (rightSlider->isEnabled()) {
        spinBox->setPrefix("L ");
    }
    else {
        spinBox->setPrefix("");
        rightSlider->setValue(leftSlider->value());
    }
    spinBox->setValue(value);
    // qDebug() << "valueChangedLeft";
    emit valueChanged(value);
}

void LEDControllerWidget::rightSliderValueHasChanged(int value) {
    if (rightSlider->isEnabled()) {
        spinBox->setPrefix("R ");
    }
    else {
        spinBox->setPrefix("");
    }
    spinBox->setValue(value);
    // qDebug() << "valueChangedRight";
    emit valueChanged(value);
}

void LEDControllerWidget::emitSignalLeftSliderPressed() {
    emit leftSlider->sliderPressed();
}

void LEDControllerWidget::emitSignalRightSliderPressed() {
    emit rightSlider->sliderPressed();
}

void LEDControllerWidget::leftSliderSetValue(int value) {
    leftSlider->setValue(value);
}

void LEDControllerWidget::rightSliderSetValue(int value) {
    rightSlider->setValue(value);
}

void LEDControllerWidget::rightSliderSetEnabled(bool enabled) {
    rightSlider->setEnabled(enabled);
}

void LEDControllerWidget::resetValues() {
    rightSlider->setValue(0);
    leftSlider->setValue(0);
}
