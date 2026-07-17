#pragma once
#include <QMutex>
#include <QQueue>
#include <QElapsedTimer>
#include <QSocketNotifier>
#include <QLoggingCategory>
#include <linux/can/raw.h>
#include "auxiliary/config_manager.h"

Q_DECLARE_LOGGING_CATEGORY(can);

class CanServerManager : public QObject
{
    Q_OBJECT

signals:
    void isRemote(quint8 reface);
    #define CHANNEL(n) void to_UartChannel##n(quint8 cmd, quint8 func, const QByteArray& param,bool isScpi);

    CHANNEL_COUNT
    #undef CHANNEL

public:
    explicit CanServerManager(QObject *parent = nullptr);
    ~CanServerManager();

    bool startServer();
    void sendFrame(quint8 ch,quint16 uart,const QByteArray &param);

private:
    void to_Channel(int channel,quint8 cmd,quint8 func,const QByteArray& param);
    bool createSocket(const QString &interface);
    void processFrame(const QByteArray &data);

    int m_socketFd{-1};
    quint8 m_calibrate_step{0};

    QQueue<struct can_frame> m_sendQueue;
    QHash<quint32, quint16> m_canToUart;
    QHash<quint16, quint32> m_uartToCan;

    QSocketNotifier *m_readNotifier{nullptr};
    QSocketNotifier *m_writeNotifier{nullptr};
    QThread *m_serverThread{nullptr};
};






