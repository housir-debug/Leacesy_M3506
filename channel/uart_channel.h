#pragma once
#include <QQueue>
#include <QObject>

class QTimer;
class QSerialPort;
class UartChannelManager : public QObject
{
    Q_OBJECT
private:
    struct Command
    {
        quint16 cmds;
        QByteArray param;
        bool isScpi;
    };
    QQueue<Command> m_txWaitQueue;
    static const QVector<Command> m_commands;
    static const QVector<Command> m_initCommands;

    QSerialPort *m_serialPort{nullptr};
    QTimer *m_refreshTimer{nullptr};
    QTimer *m_outTimer{nullptr};

    quint8 m_channel{0};
    bool isExist{false};
    int m_timeindex{0};
    int m_initindex{0};

public:
    void initUartchannel(quint8 ch, const QString &portName,qint32 baudRate);
    void sendInitCommand();

    void writeFrame(quint16 cmds, const QByteArray& param,bool isScpi);
    explicit UartChannelManager(QObject *parent = nullptr);
    ~UartChannelManager() = default;;

private:
    void sendNextCmd();
    QByteArray m_responsebuffer;

    bool m_waitR{false};
    bool m_isScpi{false};
    quint16 m_resChecked{0};

    QByteArray m_readbuffer;
    void handleReadyRead();
    QByteArray m_readparam;

    void handleNotScpicmd(quint16 cmds);
    void handleScpiQcmd(quint16 cmds);

    void handleCalibrationcmd(quint8 func);
    void handleErrorcmd(quint8 func);
};

