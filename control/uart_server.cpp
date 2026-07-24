#include "uart_server.h"
#include <QtCore>

Q_LOGGING_CATEGORY(uart_server, "UART_SERVER:")

UartServerManager::UartServerManager(QObject *parent): QObject(parent) {}
UartServerManager::~UartServerManager()
{
    if (m_uartServer) {
        m_uartServer->close();
        delete m_uartServer;
        m_uartServer= nullptr;
    }

    if (m_serverThread) {
        m_serverThread->quit();
        m_serverThread->wait(1000); // wait 1s
        m_serverThread->deleteLater();
        delete m_serverThread;
        m_serverThread = nullptr;
    }
}

bool UartServerManager::startServer()
{
    if (!m_serverThread && !m_uartServer){
        m_uartServer  = new QSerialPort(this);
        // Set the data bit to 8 bits, For example: Data5 - Data8
        m_uartServer->setDataBits(QSerialPort::Data8);
        // Not use parity check bits, the upper-level protocol ensures data integrity.
        m_uartServer->setParity(QSerialPort::NoParity);
        // Use 1 stop bit, Mark the end of A data byte
        m_uartServer->setStopBits(QSerialPort::OneStop);
        // HardwareControl: Requires wiring support | SoftwareControl: Applicable only to written text
        m_uartServer ->setFlowControl(QSerialPort::NoFlowControl);
        // QSerialPort::Baud115200
        m_uartServer->setBaudRate(QSerialPort::Baud38400);
        m_uartServer->setPortName("/dev/ttyWCH27");

        if (m_uartServer->open(QIODevice::ReadWrite)) {
            m_serverThread = new QThread(this);
            this->moveToThread(m_serverThread);
            m_uartServer->moveToThread(m_serverThread);

            connect(m_serverThread, &QThread::started, this, [this]() {
                connect(m_uartServer, &QSerialPort::readyRead, this, &UartServerManager::handleReadyRead, Qt::DirectConnection);
                connect(m_uartServer, &QSerialPort::errorOccurred, this, [this]() {
                    qCWarning(uart_server)<<"[startServer]:Uartserver Occur Error: "<<m_uartServer->errorString();
                }, Qt::DirectConnection);
            });

            m_serverThread->setObjectName("UartServer");
            m_serverThread->start();
            return true;
        }
    }

    qCDebug(uart_server)<<"[startServer]:already exist!";
    return false;
}

void UartServerManager::handleReadyRead()
{
    // Test progressing
    /*if (m_readbuffer.size() >= 1024) { // 1KB
        qint64 elapsed = m_testTimer.elapsed(); // ms
        double speedKBps =  (1024 * 1000.0) / (elapsed * 1024);
        double speedBps = 1024 * 1000.0 / elapsed;

        qCDebug(uart_server) << "\n" << QString(
            "Loopback Test Result:"
            "Time elapsed: %1 ms"
            "Speed: %2 KB/s (%3 bps)"
        ).arg(elapsed).arg(speedKBps, 0, 'f', 2).arg(speedBps * 8, 0, 'f', 0);

        m_readbuffer.clear();
    }*/

    if (ConfigManager::s_remoteSt.load()==3 || ConfigManager::s_remoteSt.load()==0){
        if (ConfigManager::s_remoteSt.load()==0){emit isRemote(3);}

        m_readbuffer = m_uartServer->readAll();
        qCDebug(uart_server)<<"[handleReadyRead]:UartServer SCPI-Commend: "<< m_readbuffer;

        m_responsebuffer = m_scpiManager->processCommand(m_readbuffer);
        if (!m_responsebuffer.isEmpty()){m_uartServer->write(m_responsebuffer);}
        qCDebug(uart_server)<<"[handleReadyRead]:UartServer SCPI Response: "<<m_responsebuffer;

        return;
    }

    QByteArray errMsg = "Other instrument interfaces are currently in remote mode.";
    qCDebug(uart_server)<<"[handleReadyRead]: "<<errMsg;
    m_uartServer->write(errMsg);
}

void UartServerManager::startLoopbackTest()
{
    qCDebug(uart_server)<<"[startLoopbackTest]:Starting Loopback Test.";
    QByteArray testData(1024, 0);

    m_testTimer.start();
    m_uartServer->write(testData);
}
