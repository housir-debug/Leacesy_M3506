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
        quint8 cmd;
        quint8 func;
        QByteArray param;
        bool isScpi;
    };
    static const QVector<Command> m_commands;
    static const QVector<Command> m_initCommands;

public:
    void initUartchannel(quint8 ch, const QString &portName,qint32 baudRate);
    void sendInitCommand();

    void writeFrame(quint8 cmd, quint8 func, const QByteArray& param,bool isScpi);
    explicit UartChannelManager(QObject *parent = nullptr);
    ~UartChannelManager() = default;;

private:
    QSerialPort *m_serialPort{nullptr};
    QTimer *m_refreshTimer{nullptr};
    QTimer *m_outTimer{nullptr};

    quint8 m_channel{0};
    int m_timeindex{0};
    int m_initindex{0};
    bool isExist{false};

    QByteArray m_readparam;
    QByteArray m_readbuffer;
    QByteArray m_responsebuffer;

    bool m_waitR{false};
    bool m_isScpi{false};
    quint16 m_resChecked{0};
    QQueue<Command> m_txWaitQueue;

    void sendNextCmd();
    void handleReadyRead();
    void handleOutputcmd         (quint8 func);
    void handleSettingcmd        (quint8 func);
    void handleControlcmd        (quint8 func);
    void handleMeasurementcmd    (quint8 func);
    void handleRegistercmd       (quint8 func);
    void handleCalibratecmd      (quint8 func);
    void handleCalibrationcmd    (quint8 func);
    void handleTriggercmd        (quint8 func);
    void handleErrorcmd          (quint8 func);
};

