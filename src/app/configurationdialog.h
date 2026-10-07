#ifndef CONFIGURATIONDIALOG_H
#define CONFIGURATIONDIALOG_H

#include <QDialog>
#include <QString>

struct ConfigurationValues
{
    QString ledSpdFolder;
    QString calibrationFile;
    QString wifiSsid;
    QString wifiPassword;
    QString hostAddress;
    int hostPort = 5001;
};

class QLineEdit;
class QSpinBox;

class ConfigurationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigurationDialog(const ConfigurationValues& values, QWidget* parent = nullptr);

    ConfigurationValues values() const;

private:
    QLineEdit* ledSpdFolderEdit;
    QLineEdit* calibrationFileEdit;
    QLineEdit* wifiSsidEdit;
    QLineEdit* wifiPasswordEdit;
    QLineEdit* hostAddressEdit;
    QSpinBox* hostPortSpinBox;
};

#endif // CONFIGURATIONDIALOG_H