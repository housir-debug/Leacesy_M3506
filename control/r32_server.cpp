#include "auxiliary/global_static.h"
#include "auxiliary/signal_bus.h"
#include "channel/scpi_handle.h"
#include "r32_server.h"
#include <QSerialPort>

void UartServerManager::startLoopbackTest()
{
    qCDebug(r32Ser)<<"[startLoopbackTest]:Starting Loopback Test.";
    QByteArray testData(1024, 0);

    m_testTimer.start();
    m_uartServer->write(testData);
}

constexpr std::array<quint32, 8> UartServerManager::m_validBaudRates =
{
    QSerialPort::Baud115200,//0
    QSerialPort::Baud57600,//1
    QSerialPort::Baud38400,//2
    QSerialPort::Baud19200,//3
    QSerialPort::Baud9600,//4
    QSerialPort::Baud4800,//5
    QSerialPort::Baud2400,//6
    QSerialPort::Baud1200,//7
};
void UartServerManager::changeBaudRate()
{
    if (m_uartServer->setBaudRate(m_validBaudRates[ConfigManager::s_rs232BaudIndex])) {
        ConfigManager::saveNewValue(ConfAddress::R23Baud,ConfigManager::s_rs232BaudIndex);
        emit ChannelBus::instance().interAcUpdate();
    }
}

UartServerManager::UartServerManager(QObject *parent): QObject(parent)
{
    m_uartServer  = new QSerialPort(this);
    // Set the data bit to 8 bits, For example: Data5 - Data8
    m_uartServer->setDataBits(QSerialPort::Data8);
    // Not use parity check bits, the upper-level protocol ensures data integrity.
    m_uartServer->setParity(QSerialPort::NoParity);
    // Use 1 stop bit, Mark the end of A data byte
    m_uartServer->setStopBits(QSerialPort::OneStop);
    // HardwareControl: Requires wiring support | SoftwareControl: Applicable only to written text
    m_uartServer ->setFlowControl(QSerialPort::NoFlowControl);
    m_uartServer->setBaudRate(m_validBaudRates[ConfigManager::s_rs232BaudIndex]);
    m_uartServer->setPortName("/dev/ttyS1");

    if (m_uartServer->open(QIODevice::ReadWrite)) {
        connect(m_uartServer, &QSerialPort::errorOccurred, this, [this]() {
            qCWarning(r32Ser)<<"[UartServerManager]:Uartserver Occur Error: "<<m_uartServer->errorString();
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

            switch (ConfigManager::control_Mode.load()) {
                case InterSt::LOCAL:emit ChannelBus::instance().switchRemote(InterSt::RS232);// fall through
                case InterSt::RS232:
                    m_readbuffer = m_uartServer->readAll();
                    qCDebug(r32Ser)<<"[UartServerManager]:UartServer SCPI-Commend: "<< m_readbuffer;

                    m_responsebuffer = ScpiManager::instance().processCommand(m_readbuffer);
                    if (!m_responsebuffer.isEmpty()){m_uartServer->write(m_responsebuffer);}
                    qCDebug(r32Ser)<<"[UartServerManager]:UartServer SCPI Response: "<<m_responsebuffer;
                    break;
                default:
                    QByteArray errMsg = "Other instrument interfaces are currently in remote mode.";
                    qCDebug(r32Ser)<<"[UartServerManager]: "<<errMsg;
                    m_uartServer->write(errMsg);
            }}, Qt::DirectConnection);

        return;
    }
}
