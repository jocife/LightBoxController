#include "connectionpagewidget.h"
#include "ui_connectionpagewidget.h"

ConnectionPageWidget::ConnectionPageWidget(const QString& imagesFolderAbsolutePath, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ConnectionPageWidget)
    , tcpSocket(new QTcpSocket(this))
{
    ui->setupUi(this);

    // Lightcabinet display setup
    imageLightCabinet = new QImage(imagesFolderAbsolutePath + "/" + "light-cabinet.jpg");
    labelDisplayLightCabinet = new QLabel("");
    labelDisplayLightCabinet->setPixmap(QPixmap::fromImage(*imageLightCabinet));
    // labelDisplayLightCabinet->adjustSize(); // Wait for layout

    ui->gridLayoutPageConnection->addWidget(labelDisplayLightCabinet, 0, 0, -1, 1, Qt::AlignHCenter);

    // WIFI texts
    labelWifiSSID = new QLabel("SSID: " + WIFI_SSID);
    labelWifiSSID->setStyleSheet("font: 18pt;");
    labelWifiSSID->setTextInteractionFlags(Qt::TextSelectableByMouse);

    labelWifiPassword = new QLabel("Password: " + WIFI_PASSWORD);
    labelWifiPassword->setStyleSheet("font: 18pt;");
    labelWifiPassword->setTextInteractionFlags(Qt::TextSelectableByMouse);

    ui->gridLayoutPageConnection->addWidget(labelWifiSSID, 3, 1, Qt::AlignBottom);
    ui->gridLayoutPageConnection->addWidget(labelWifiPassword, 4, 1, -1, 1, Qt::AlignLeft);

    // Connection button
    pushButtonConnect = new QPushButton("Connect");
    pushButtonConnect->setMinimumHeight(50);
    pushButtonConnect->setStyleSheet("font: 18pt;");

    // Disconnect button
    pushButtonDisconnect = new QPushButton("Disconnect");
    pushButtonDisconnect->setMinimumHeight(50);
    pushButtonDisconnect->setStyleSheet("font: 18pt;");

    // Socket state indication layout
    gridLayoutTcpSocketState = new QGridLayout;

    labelTcpSocketStateSymbol = new QLabel("⬤");
    labelTcpSocketStateSymbol->setStyleSheet("font: 18pt; color: red;");

    labelTcpSocketState = new QLabel("Disconnected");
    labelTcpSocketState->setStyleSheet("font: 18pt;");

    gridLayoutTcpSocketState->addWidget(labelTcpSocketStateSymbol, 0, 0, Qt::AlignLeft);
    gridLayoutTcpSocketState->addWidget(labelTcpSocketState, 0, 1, Qt::AlignLeft);

    ui->gridLayoutPageConnection->addWidget(pushButtonConnect, 0, 2, Qt::AlignBottom);
    ui->gridLayoutPageConnection->addWidget(pushButtonDisconnect, 1, 2, Qt::AlignBottom);
    ui->gridLayoutPageConnection->addLayout(gridLayoutTcpSocketState, 2, 2, Qt::AlignLeft);

    // Networking Slots
    connect(tcpSocket, &QTcpSocket::stateChanged, this, &ConnectionPageWidget::tcpSocketStateHasChanged);
    connect(pushButtonConnect, SIGNAL(clicked()), this, SLOT(pushButtonConnectIsClicked()));
    connect(pushButtonDisconnect, SIGNAL(clicked()), this, SLOT(pushButtonDisconnectIsClicked()));
}

ConnectionPageWidget::~ConnectionPageWidget()
{
    if (tcpSocket) {
        tcpSocket->disconnect();
        tcpSocket->abort();
    }
    delete imageLightCabinet;
    delete ui;
}

void ConnectionPageWidget::sendData(const QByteArray& data)
{
    if (tcpSocket && tcpSocket->state() == QAbstractSocket::ConnectedState) {
        tcpSocket->write(data.constData());
        tcpSocket->flush(); // Force the packet to be sent immediately
        emit logMessage("ConnectionPage", "Sent " + QString::number(data.size()) + " bytes: " + QString(data));
    } else {
        emit logMessage("ConnectionPage", "Failed to send: Not connected.");
    }
}

void ConnectionPageWidget::pushButtonConnectIsClicked()
{
    emit logMessage("ConnectionPage", "Connecting to " + HostIP + ":" + QString::number(HostPORT) + "...");
    tcpSocket->connectToHost(HostIP, HostPORT);
}

void ConnectionPageWidget::pushButtonDisconnectIsClicked()
{
    emit logMessage("ConnectionPage", "Disconnecting...");
    tcpSocket->disconnectFromHost();
}

void ConnectionPageWidget::tcpSocketStateHasChanged(QAbstractSocket::SocketState socketState)
{
    switch (socketState) {
        case QAbstractSocket::UnconnectedState:
            labelTcpSocketStateSymbol->setText("⬤");
            labelTcpSocketStateSymbol->setStyleSheet("font: 18pt; color: red;");
            labelTcpSocketState->setText("Disconnected");
            emit logMessage("ConnectionPage", "TCP Socket: Disconnected");
            emit connectionStateChanged(false);
            break;
        case QAbstractSocket::HostLookupState:
            // Handled
            break;
        case QAbstractSocket::ConnectingState:
            labelTcpSocketStateSymbol->setText("⬤");
            labelTcpSocketStateSymbol->setStyleSheet("font: 18pt; color: orange;");
            labelTcpSocketState->setText("Connecting");
            emit logMessage("ConnectionPage", "TCP Socket: Connecting");
            break;
        case QAbstractSocket::ConnectedState:
            labelTcpSocketStateSymbol->setText("⬤");
            labelTcpSocketStateSymbol->setStyleSheet("font: 18pt; color: green;");
            labelTcpSocketState->setText("Connected");
            emit logMessage("ConnectionPage", "TCP Socket: Connected");
            emit connectionStateChanged(true);
            break;
        case QAbstractSocket::BoundState:
        case QAbstractSocket::ClosingState:
        case QAbstractSocket::ListeningState:
            break;
    }
}
