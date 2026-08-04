#pragma once
#include <QQueue>
#include <linux/can/raw.h>
#include <QSocketNotifier>
#include <QLoggingCategory>
#include "auxiliary/config_manager.h"

Q_DECLARE_LOGGING_CATEGORY(can_server);

class CanServerManager : public QObject
{
    Q_OBJECT

signals:
    void isRemote(quint8 reface);
    void baudrefresh();

    #define CHANNEL(n) void to_UartChannel##n(quint8 cmd, quint8 func, const QByteArray& param,bool isScpi);

    CHANNEL_COUNT
    #undef CHANNEL

public:
    void sendFrame(quint8 ch,quint16 uart,const QByteArray &param);
    void changebaudandid();

    explicit CanServerManager(QObject *parent = nullptr);
    ~CanServerManager();

private:
    void to_Channel(int channel,quint8 cmd,quint8 func,const QByteArray& param);
    bool createSocket(const QString &interface);
    void processFrame(const QByteArray &data);

    int m_socketFd{-1};
    QString m_interface{"can0"};
    quint8 m_calibrate_step{0};

    QQueue<can_frame> m_sendQueue;
    QHash<quint16, quint32> m_uartToCan;
    static const QHash<quint32, quint16> m_canToUart;

    QSocketNotifier *m_readNotifier{nullptr};
    QSocketNotifier *m_writeNotifier{nullptr};
    QThread *m_serverThread{nullptr};
};






