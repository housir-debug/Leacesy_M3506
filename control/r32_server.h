#pragma once
#include <QSerialPort>
#include <QElapsedTimer>

class UartServerManager : public QObject
{
    Q_OBJECT

public:
    explicit UartServerManager(QObject *parent = nullptr);
    ~UartServerManager();

    void changeBaudRate();

private:
    void startLoopbackTest();
    QElapsedTimer m_testTimer;

    QByteArray m_readbuffer;
    QByteArray m_responsebuffer;

    QSerialPort *m_uartServer{nullptr};
    QThread *m_serverThread{nullptr};
};
