#pragma once
#include <QTimer>
#include <QDataStream>
#include <QElapsedTimer>
#include <QLoggingCategory>
#include "auxiliary/config_manager.h"
#include "auxiliary/scpi_handle.h"

Q_DECLARE_LOGGING_CATEGORY(uart_channel)

struct Command {
    quint8 cmd;
    quint8 func;
    QByteArray param;
};

class UartChannelManager : public QObject
{
    Q_OBJECT

signals:
    void to_CanServer(quint8 ch,quint16 uart,const QByteArray &param);

    void CH_svChanged(int ch,const QString &ver);
    void CH_hvChanged(int ch,const QString &ver);

    void CH_VoltageChanged(int ch,float voltage);
    void CH_CurrentAndUnitChanged(int ch,float current);
    void CH_RangeChanged(int ch,quint8 range);
    void CH_StatusChanged(int ch,quint16 status);

    void CH_cvChanged(int ch,float cv);
    void CH_ccChanged(int ch,float cc);
    void CH_ovpChanged(int ch,float ovp);
    void CH_isOutputChanged(int ch,bool status);
    void CH_impChanged(int ch,float imp);

public:
    explicit UartChannelManager(QObject *parent = nullptr);
    ~UartChannelManager();

    void writeFrame(quint8 cmd, quint8 func, const QByteArray& param,bool isScpi);
    bool initSerialPort(quint8 ch, const QString &portName,qint32 baudRate);
    std::shared_ptr<ScpiManager> m_scpiManager{nullptr};

private:
    void sendInitCommand();
    void handleReadyRead();
    void handleOutputcmd         (quint8 func);
    void handleSettingcmd        (quint8 func);
    void handleControlcmd        (quint8 func);
    void handleMeasurementcmd    (quint8 func);
    void handleRegistercmd       (quint8 func);
    void handleCalibratecmd      (quint8 func);
    void handleCalibrationcmd    (quint8 func);
    void handleTriggercmd        (quint8 func);
    void handleISPcmd            (quint8 func);
    void handleSNcmd             (quint8 func);
    void handleIDcmd             (quint8 func);
    void handleErrorcmd          (quint8 func);

private:
    QByteArray m_readparam;
    QByteArray m_readbuffer;
    QByteArray m_responsebuffer;

    bool isExist{false};
    quint8 m_channel{0};
    quint8 m_initindex{0};
    quint8 m_timeindex{0};
    quint16 m_scpiCommand{0};
    QVector<Command> m_commands;
    std::atomic<bool> m_waitingForRes{false};

    QTimer *m_refreshtimer{nullptr};
    QThread *m_serialThread{nullptr};
    QSerialPort *m_serialPort{nullptr};

    static const QVector<Command> m_initCommands;
    static constexpr quint8 HEADER_HIGH = 0xAA;
    static constexpr quint8 HEADER_LOW = 0x55;
    static constexpr quint8 END_MARKER = 0xEE;
};

