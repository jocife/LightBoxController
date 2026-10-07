#ifndef CONNECTIONPAGEWIDGET_H
#define CONNECTIONPAGEWIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QImage>

namespace Ui {
class ConnectionPageWidget;
}

class ConnectionPageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionPageWidget(const QString& imagesFolderAbsolutePath, const QString& hostAddress,
                                  qint16 hostPort, const QString& wifiSsid, const QString& wifiPassword,
                                  QWidget *parent = nullptr);
    ~ConnectionPageWidget();

    // Used by LightBoxController to forward data from other tabs to the socket
    void sendData(const QByteArray& data);
    void setConnectionSettings(const QString& hostAddress, qint16 hostPort,
                               const QString& wifiSsid, const QString& wifiPassword);

signals:
    // Emitted when connection state changes, so LightBoxController knows if it's connected
    void connectionStateChanged(bool isConnected);
    
    // Forwards log messages to the main controller's log panel
    void logMessage(const QString& source, const QString& message);

public slots:
    void pushButtonConnectIsClicked();

private slots:
    void pushButtonDisconnectIsClicked();
    void tcpSocketStateHasChanged(QAbstractSocket::SocketState socketState);

private:
    Ui::ConnectionPageWidget *ui;

    QTcpSocket *tcpSocket;

    // For testing purposes, use the following settings to connect to a local TCP server:
    //const QString HostIP = "127.0.0.1";
    //const qint16 HostPORT = 1234;

    // For actual hardware connection, use the following settings:
    QString hostIp;
    qint16 hostPort;
    QString wifiSsid;
    QString wifiPassword;

    QLabel *labelWifiSSID;
    QLabel *labelWifiPassword;
    QLabel *labelTcpSocketStateSymbol;
    QLabel *labelTcpSocketState;
    QLabel *labelDisplayLightBox;

    QImage *imageLightBox;
    QGridLayout *gridLayoutTcpSocketState;

    QPushButton *pushButtonConnect;
    QPushButton *pushButtonDisconnect;
};

#endif // CONNECTIONPAGEWIDGET_H
