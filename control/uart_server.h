#pragma once
#include "auxiliary/scpi_handle.h"

Q_DECLARE_LOGGING_CATEGORY(uart_server)

class UartServerManager : public QObject
{
    Q_OBJECT

signals:
    void isRemote(quint8 reface);

public:
    explicit UartServerManager(QObject *parent = nullptr);
    ~UartServerManager();

    std::shared_ptr<ScpiManager> m_scpiManager{nullptr};
    bool startServer();

private:
    void startLoopbackTest();
    QElapsedTimer m_testTimer;

    void handleReadyRead();
    QByteArray m_readbuffer;
    QByteArray m_responsebuffer;

    QSerialPort *m_uartServer{nullptr};
    QThread *m_serverThread{nullptr};
};
