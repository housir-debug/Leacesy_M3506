#include "uart_server.h"
#include <QtCore>

Q_LOGGING_CATEGORY(uart_server, "UART_SERVER:")

void UartServerManager::changeBaudRate()
{
    static const QList<int> validBaudRates = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};

    if (validBaudRates.contains(ConfigManager::s_rs232BaudRate) && m_uartServer->setBaudRate(static_cast<QSerialPort::BaudRate>(ConfigManager::s_rs232BaudRate))) {
        ConfigManager::setConfigValue("Device/RS232Baud",ConfigManager::s_rs232BaudRate);
        emit baudrefresh();
        return;
    }
}


UartServerManager::UartServerManager(QObject *parent): QObject(parent)
{
    m_serverThread = new QThread(this);
    this->moveToThread(m_serverThread);

    connect(m_serverThread, &QThread::started, this, [this]() {
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
        m_uartServer->setBaudRate(static_cast<QSerialPort::BaudRate>(ConfigManager::s_rs232BaudRate));
        m_uartServer->setPortName("/dev/ttyS1");

        if (m_uartServer->open(QIODevice::ReadWrite)) {
            connect(m_uartServer, &QSerialPort::errorOccurred, this, [this]() {
                qCWarning(uart_server)<<"[UartServerManager]:Uartserver Occur Error: "<<m_uartServer->errorString();
                }, Qt::DirectConnection);
            connect(m_uartServer, &QSerialPort::readyRead,  this, [this](){
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

                switch (ConfigManager::s_remoteSt.load()) {
                    case 0:emit isRemote(2);// fall through
                    case 2:
                        m_readbuffer = m_uartServer->readAll();
                        qCDebug(uart_server)<<"[UartServerManager]:UartServer SCPI-Commend: "<< m_readbuffer;

                        m_responsebuffer = m_scpiManager->processCommand(m_readbuffer);
                        if (!m_responsebuffer.isEmpty()){m_uartServer->write(m_responsebuffer);}
                        qCDebug(uart_server)<<"[UartServerManager]:UartServer SCPI Response: "<<m_responsebuffer;
                        break;
                    default:
                        QByteArray errMsg = "Other instrument interfaces are currently in remote mode.";
                        qCDebug(uart_server)<<"[UartServerManager]: "<<errMsg;
                        m_uartServer->write(errMsg);
                }

                return;
                }, Qt::DirectConnection);

            return;
        }

        qCCritical(uart_server)<<"[UartServerManager]:QSerialPort openning failed!";
        });

    m_serverThread->setObjectName("UartServer");
    m_serverThread->start();
}

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


void UartServerManager::startLoopbackTest()
{
    qCDebug(uart_server)<<"[startLoopbackTest]:Starting Loopback Test.";
    QByteArray testData(1024, 0);

    m_testTimer.start();
    m_uartServer->write(testData);
}
