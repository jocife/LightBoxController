#include "configurationdialog.h"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <functional>

namespace
{
QWidget* createPathEditor(QLineEdit* editor, const QString& buttonText, QObject* receiver,
                          const std::function<void()>& browse)
{
    QWidget* container = new QWidget;
    QHBoxLayout* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(editor);
    QPushButton* button = new QPushButton(buttonText, container);
    layout->addWidget(button);
    QObject::connect(button, &QPushButton::clicked, receiver, browse);
    return container;
}
}

ConfigurationDialog::ConfigurationDialog(const ConfigurationValues& values, QWidget* parent)
    : QDialog(parent)
    , ledSpdFolderEdit(new QLineEdit(values.ledSpdFolder, this))
    , calibrationFileEdit(new QLineEdit(values.calibrationFile, this))
    , wifiSsidEdit(new QLineEdit(values.wifiSsid, this))
    , wifiPasswordEdit(new QLineEdit(values.wifiPassword, this))
    , hostAddressEdit(new QLineEdit(values.hostAddress, this))
    , hostPortSpinBox(new QSpinBox(this))
{
    setWindowTitle(tr("Configuration"));
    setModal(true);

    wifiPasswordEdit->setEchoMode(QLineEdit::Password);
    hostPortSpinBox->setRange(1, 65535);
    hostPortSpinBox->setValue(values.hostPort);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->addRow(tr("LED SPD folder:"), createPathEditor(
        ledSpdFolderEdit, tr("Browse..."), this, [this]() {
            const QString path = QFileDialog::getExistingDirectory(this, tr("Select LED SPD folder"),
                                                                    ledSpdFolderEdit->text());
            if (!path.isEmpty()) ledSpdFolderEdit->setText(path);
        }));
    formLayout->addRow(tr("Calibration CSV:"), createPathEditor(
        calibrationFileEdit, tr("Browse..."), this, [this]() {
            const QString path = QFileDialog::getOpenFileName(this, tr("Select calibration CSV"),
                                                               calibrationFileEdit->text(),
                                                               tr("CSV files (*.csv);;All files (*)"));
            if (!path.isEmpty()) calibrationFileEdit->setText(path);
        }));
    formLayout->addRow(tr("WiFi SSID:"), wifiSsidEdit);
    formLayout->addRow(tr("WiFi password:"), wifiPasswordEdit);
    formLayout->addRow(tr("Controller IP address:"), hostAddressEdit);
    formLayout->addRow(tr("Controller TCP port:"), hostPortSpinBox);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
    setMinimumWidth(620);
    adjustSize();
}

ConfigurationValues ConfigurationDialog::values() const
{
    ConfigurationValues result;
    result.ledSpdFolder = ledSpdFolderEdit->text().trimmed();
    result.calibrationFile = calibrationFileEdit->text().trimmed();
    result.wifiSsid = wifiSsidEdit->text().trimmed();
    result.wifiPassword = wifiPasswordEdit->text();
    result.hostAddress = hostAddressEdit->text().trimmed();
    result.hostPort = hostPortSpinBox->value();
    return result;
}