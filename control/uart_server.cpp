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

    qCDebug(uart_server)<<"[~UartServerManager]:UartServerManager Destroyed!!!";
}

bool UartServerManager::startServer(const QString &portName,qint32 baudRate)
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
        m_uartServer->setBaudRate(baudRate);
        m_uartServer->setPortName(portName);

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

    qCWarning(uart_server)<<"[startServer]:already exist A certain member";
    return false;
}

void UartServerManager::handleReadyRead()
{
    if (ConfigManager::s_remoteSt.load()==3 || ConfigManager::s_remoteSt.load()==0){
        if (ConfigManager::s_remoteSt.load()==0){emit isRemote(3);}

        // read information
        m_readbuffer.clear();
        m_readbuffer.append(m_uartServer->readAll());
        QString message = QString::fromUtf8(m_readbuffer).trimmed();   // SOCKET ASCll Define(0x00-0x7F)
        qCDebug(uart_server)<<"[handleReadyRead]:Uart SCPI Request Commend: "<< message;

        // Return response
        m_responsebuffer.clear();
        m_responsebuffer = m_scpiManager->processCommand(m_readbuffer);
        if (!m_responsebuffer.isEmpty()){
            qCDebug(uart_server)<<"[handleReadyRead]:UartServer SCPI Response: "<<m_responsebuffer;
            m_uartServer->write(m_responsebuffer);
        }

        return;
    }

    QByteArray errMsg = QString("Other interfaces of the instrument are currently in operation").toUtf8();
    qCDebug(uart_server)<<"[handleReadyRead]:Currently in an alternative remote mode";
    m_uartServer->write(errMsg);

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
}

void UartServerManager::startLoopbackTest()
{
    qCDebug(uart_server)<<"[startLoopbackTest]:Starting Loopback Test.";
    QByteArray testData(1024, 0);

    m_testTimer.start();
    m_uartServer->write(testData);
}
