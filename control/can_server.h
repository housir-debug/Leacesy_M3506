#pragma once
#include <QHash>
#include <QQueue>
#include <QObject>
#include <linux/can/raw.h>

class QSocketNotifier;

class CanServerManager : public QObject
{
    Q_OBJECT

private:
    static const std::array<quint32,9> m_validBaudRates;
    static const QHash<quint32, quint16> m_canToUart;
    static const QHash<quint16, quint32> m_uartToCan;

    void processFrame(const QByteArray &data);

public:
    void changebaud();
    void sendFrame(quint8 ch,quint16 uart,const QByteArray &param);

    explicit CanServerManager(QObject *parent = nullptr);
    ~CanServerManager();

private:
    bool OpeningCanN();
    bool createSocket();

    QSocketNotifier *m_readNotifier{nullptr};
    QSocketNotifier *m_writeNotifier{nullptr};
    QQueue<can_frame> m_sendQueue;

    QString m_interface{"can0"};
    quint8 m_calibrate_step{0};
    int m_socketFd{-1};
};






