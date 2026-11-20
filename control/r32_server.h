#pragma once
#include <QObject>
#include <QElapsedTimer>

class QSerialPort;
class UartServerManager : public QObject
{
    Q_OBJECT
private:
    void startLoopbackTest();
    QElapsedTimer m_testTimer;

    QByteArray m_readbuffer;
    QByteArray m_responsebuffer;

    QSerialPort *m_uartServer{nullptr};

public:
    void changeBaudRate();

    explicit UartServerManager(QObject *parent = nullptr);
    ~UartServerManager() = default;
};
