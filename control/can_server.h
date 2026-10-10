#pragma once
#include <QQueue>
#include <linux/can/raw.h>
#include <QSocketNotifier>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(can_server);

class CanServerManager : public QObject
{
    Q_OBJECT

signals:
    void isRemote(quint8 reface);
    void baudrefresh();

public:
    void sendFrame(quint8 ch,quint16 uart,const QByteArray &param);
    void changebaudandid();

    explicit CanServerManager(QObject *parent = nullptr);
    ~CanServerManager();

private:
    bool OpeningCanN();
    bool createSocket();
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






