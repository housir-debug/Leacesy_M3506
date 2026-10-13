#pragma once
#include <QObject>

class QTimer;
class QSerialPort;

class UartChannelManager : public QObject
{
    Q_OBJECT

public:
    void writeFrame(quint8 cmd, quint8 func, const QByteArray& param,bool isScpi);
    void initUartchannel(quint8 ch, const QString &portName,qint32 baudRate);
    void sendInitCommand();

    explicit UartChannelManager(QObject *parent = nullptr);
    ~UartChannelManager();

private:
    void handleReadyRead();
    void handleOutputcmd         (quint8 func,quint16 scpiIdentify);
    void handleSettingcmd        (quint8 func,quint16 scpiIdentify);
    void handleControlcmd        (quint8 func,quint16 scpiIdentify);
    void handleMeasurementcmd    (quint8 func,quint16 scpiIdentify);
    void handleRegistercmd       (quint8 func,quint16 scpiIdentify);
    void handleCalibratecmd      (quint8 func,quint16 scpiIdentify);
    void handleCalibrationcmd    (quint8 func,quint16 scpiIdentify);
    void handleTriggercmd        (quint8 func,quint16 scpiIdentify);
    void handleErrorcmd          (quint8 func);

private:
    bool m_waitR{false};
    quint16 m_scpiCommand{0};

    QByteArray m_readparam;
    QByteArray m_readbuffer;
    QByteArray m_responsebuffer;

    quint8 m_channel{0};
    int m_timeindex{0};
    int m_initindex{0};
    bool isExist{false};

    struct Command
    {
        quint8 cmd;
        quint8 func;
        QByteArray param;
    };
    static const QVector<Command> m_commands;
    static const QVector<Command> m_initCommands;
    static QThread* getWorkerThread(quint32 channel);

    QTimer *m_refreshtimer{nullptr};
    QSerialPort *m_serialPort{nullptr};
};

