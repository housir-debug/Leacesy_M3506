#include "auxiliary/global_static.h"
#include "auxiliary/signal_bus.h"
#include "uart_protocol.h"
#include "uart_channel.h"
#include "scpi_handle.h"
// --- QT package ---
#include <QSerialPort>
#include <QtEndian>
#include <QTimer>

const QVector<UartChannelManager::Command> UartChannelManager::m_commands =
{
    {QCmd(JV6xxx::UartCMD::RVoltage), "",false},// voltage
    {QCmd(JV6xxx::UartCMD::RCurrent), "",false},// current
    {QCmd(JV6xxx::UartCMD::StaREG), "",false},// status
};
const QVector<UartChannelManager::Command> UartChannelManager::m_initCommands =
{
    {QCmd(JV6xxx::UartCMD::SV), "",false},// query software
    {QCmd(JV6xxx::UartCMD::HV), "",false},// query Hardware
    {JV6xxx::UartCMD::OUTP_OFF, "",false},// Turnoff output
    {JV6xxx::UartCMD::VOLT, QByteArray::fromHex("00 00 00 00"),false},// set cv =0
    {JV6xxx::UartCMD::CURR, QByteArray::fromHex("3f 80 00 00"),false},// set cc =1
    {JV6xxx::UartCMD::IMP, QByteArray::fromHex("00 00 00 00"),false},// set Output impedance =0 -> cv model
    {JV6xxx::UartCMD::OVP, QByteArray::fromHex("41 00 00 00"),false},// set ovp =8
    {JV6xxx::UartCMD::NPLC, QByteArray::fromHex("3f 80 00 00"),false},// set NPLC =1
    {JV6xxx::UartCMD::CurrRANG, QByteArray::fromHex("00"),false},// set A Unit
    {JV6xxx::UartCMD::AverCount, QByteArray::fromHex("01"),false},// set measure average =1
    {JV6xxx::UartCMD::SwePOIN, QByteArray::fromHex("00 01"),false},// set Output impedance step =1
    {JV6xxx::UartCMD::SweTINT, QByteArray::fromHex("37 82 dc bf"),false},// set Tint =1.56e-05
    {JV6xxx::UartCMD::CurrRangTIM, QByteArray::fromHex("42 03 11 68"),false},// set relarge =32.767
};

void UartChannelManager::initUartchannel(quint8 ch, const QString &portName,qint32 baudRate)
{
    m_serialPort = new QSerialPort(this);
    // Set the data bit to 8 bits, For example: Data5 - Data8
    m_serialPort->setDataBits(QSerialPort::Data8);
    // Not use parity check bits, the upper-level protocol ensures data integrity.
    m_serialPort->setParity(QSerialPort::NoParity);
    // Use 1 stop bit, Mark the end of A data byte
    m_serialPort->setStopBits(QSerialPort::OneStop);
    // HardwareControl: Requires wiring support | SoftwareControl: Applicable only to written text
    m_serialPort ->setFlowControl(QSerialPort::NoFlowControl);
    // setting QSerialPort::Baud38400 and QSerialPort - name
    m_serialPort->setBaudRate(baudRate);
    m_serialPort->setPortName(portName);
    m_channel = ch;

    if (m_serialPort->open(QIODevice::ReadWrite)) {
        connect(m_serialPort, &QSerialPort::readyRead, this, &UartChannelManager::handleReadyRead, Qt::DirectConnection);
        connect(m_serialPort, &QSerialPort::errorOccurred, this, [this]() {
            qCWarning(uartCh)<<"(initUartchannel)Channel_"<<m_channel<<" Error: "<<m_serialPort->errorString();
            }, Qt::DirectConnection);

        m_refreshTimer = new QTimer(this);
        m_refreshTimer->setInterval(180); // ms -> 60ms < target < at will
        connect(m_refreshTimer,&QTimer::timeout,this,[this]{
            m_timeindex = (m_timeindex + 1) % m_commands.size();
            const Command &cmd = m_commands[m_timeindex];
            writeFrame(cmd.cmds, cmd.param, false);
            }, Qt::DirectConnection);

        m_outTimer = new QTimer(this);
        m_outTimer->setSingleShot(true);
        m_outTimer->setInterval(JV6xxx::RES_WAIT_TIME);
        connect(m_outTimer, &QTimer::timeout, this, [this]{
            qCDebug(uartCh)<<"(initUartchannel)Ch_"<<m_channel<<" wait-Response outTime";
            m_waitR = false;
            sendNextCmd();
            }, Qt::DirectConnection);

        // Channel initialization command
        sendInitCommand();
        return;
    }
}
void UartChannelManager::sendInitCommand()
{
    if (m_initindex < m_initCommands.size()) {
       const Command& cmd = m_initCommands[m_initindex];
       writeFrame(cmd.cmds, cmd.param, false);

       // disable direct call this function, will block reception
       QTimer::singleShot(36, this, &UartChannelManager::sendInitCommand); // 36 ms
       m_initindex += 1;
   }else if(isExist){
       //m_refreshTimer->start();
       m_initindex = 0;
   }
}

/*
 * 1，队列等待-控制发送速度（收一发一）+ 丢包超时（跳过），保证正常状态一收一发
 * 2，超时-过期响应，加入校验码，跳过过期响应，避免污染应答-可以延长超时限制，去包含它
 * 3，状态判断是否发送远程模块
 *
 * */

void UartChannelManager::writeFrame(quint16 cmds, const QByteArray& param,bool isScpi)
{
    if (m_txWaitQueue.size() <= JV6xxx::MAX_QUEUE_CNT){
        qCDebug(uartCh)<<"(writeFrame)Quene up: "<<m_txWaitQueue.size();
        m_txWaitQueue.enqueue({cmds, param, isScpi});
        sendNextCmd();
        return;
    }

    qCWarning(uartCh)<<"(writeFrame)sendQuene overflow!";
}
UartChannelManager::UartChannelManager(QObject *parent):QObject(parent)
{}
void UartChannelManager::sendNextCmd()
{
    if (!m_waitR && !m_txWaitQueue.isEmpty()) {
        const Command& cmd = m_txWaitQueue.dequeue();
        quint8 cmdh = static_cast<quint8>(cmd.cmds >> 8);
        quint8 func  = static_cast<quint8>(cmd.cmds & 0xFF);

        quint8 length = 4 + cmd.param.size();  //  Command+Function+Channel+CheckSum  + Parameter
        quint8 checksum = length + cmdh + func + m_channel; // The check code is taken from the lowest 8 bits.
        for (char byte : cmd.param) {checksum += static_cast<quint8>(byte);}

        m_responsebuffer.clear();
        m_responsebuffer.reserve(length + 4);  // Pre-allocation enhances performance
        m_responsebuffer.append(JV6xxx::HEADER_HIGH);
        m_responsebuffer.append(JV6xxx::HEADER_LOW);
        m_responsebuffer.append(length);
        m_responsebuffer.append(cmdh);
        m_responsebuffer.append(func);
        m_responsebuffer.append(m_channel);
        m_responsebuffer.append(cmd.param);
        m_responsebuffer.append(checksum);
        m_responsebuffer.append(JV6xxx::END_MARKER);
        m_serialPort->write(m_responsebuffer);
        qCDebug(uartCh)<<"(sendNextCmd)Ch_"<<m_channel<<" Send: "<<m_responsebuffer.toHex(' ');

        m_waitR = true;
        m_outTimer->start();
        m_isScpi = cmd.isScpi;
        m_resChecked = cmd.cmds;
    }
}

// accept response

void UartChannelManager::handleReadyRead()
{
    m_readbuffer.append(m_serialPort->readAll());

    // Normal response processing of the protocol
    if (m_readbuffer.size() >= 3){
        if(static_cast<quint8>(m_readbuffer[0]) == JV6xxx::HEADER_LOW && static_cast<quint8>(m_readbuffer[1]) == JV6xxx::HEADER_HIGH){
            qCDebug(uartCh)<<"(handleReadyRead)Ch_"<<m_channel<<" Received: "<<m_readbuffer.toHex(' ');
            quint8 lengthB = static_cast<quint8>(m_readbuffer[2]);

            if (m_readbuffer.size() >= lengthB + 4){
                if(lengthB >= 4 && static_cast<quint8>(m_readbuffer[lengthB + 3]) == JV6xxx::END_MARKER){
                    quint8 cmd = static_cast<quint8>(m_readbuffer[3]);
                    quint8 func = static_cast<quint8>(m_readbuffer[4]);
                    quint8 ch = static_cast<quint8>(m_readbuffer[5]);
                    m_readparam = m_readbuffer.mid(6, lengthB - 4);// length - Command+Function+Channel+CheckSum

                    quint8 Checksum = lengthB + cmd + func + ch; // The check code is taken from the lowest 8 bits.
                    for (char byte : qAsConst(m_readparam)) {Checksum += static_cast<quint8>(byte);}

                    if(static_cast<quint8>(m_readbuffer[lengthB + 2]) == Checksum){
                        quint16 selfCheck = (static_cast<quint16>(cmd) << 8) | func;

                        if (!m_isScpi){
                            handleNotScpicmd(selfCheck);
                        }else if (ConfigManager::control_Mode == InterSt::CAN){
                            if (selfCheck == JV6xxx::UartCMD::OUTP_OFF || selfCheck == JV6xxx::UartCMD::OUTP_ON){
                                QByteArray param(1,static_cast<char>(func));
                                emit ChannelBus::instance().to_CanServer(m_channel,0x01,param);
                            }else{
                                emit ChannelBus::instance().to_CanServer(m_channel, selfCheck, m_readparam);
                            }
                        }else if(m_resChecked == selfCheck && func >= JV6xxx::UartCMD::READ){
                            handleScpiQcmd(selfCheck); // response -> SCPI
                        }else if (static_cast<quint16>(cmd << 8) == JV6xxx::UartCMD::CalStepOffset) {
                            handleCalibrationcmd(func); // Calresp -> SCPI
                        }else if (cmd == JV6xxx::Err::Error) {
                            handleErrorcmd(func); // err QString -> SCPI
                        }else{
                            ScpiManager::instance().processCHVoidResponse(m_channel);
                        }

                        m_readbuffer.remove(0,lengthB + 4);
                        if(!m_readbuffer.isEmpty()){
                            handleReadyRead();
                        }

                        m_outTimer->stop();
                        m_resChecked = 0;
                        m_waitR = false;
                        sendNextCmd();
                        return; // finish.
                    }
                }

                qCDebug(uartCh)<<"(handleReadyRead)Ch_"<<m_channel<<" check_end-Error: "<<m_readbuffer;
                m_readbuffer.clear();
            }

            return; // Insufficient length; wait to be completed.
        }

        qCDebug(uartCh)<<"(handleReadyRead)Ch_"<<m_channel<<" head-Error: "<<m_readbuffer;
        m_readbuffer.clear();
    }
}

void UartChannelManager::handleNotScpicmd(quint16 cmds)
{
    if (cmds == JV6xxx::UartCMD::OUTP_OFF){
        emit ChannelBus::instance().CH_isOutputChanged(m_channel,false);
        return;
    }else if(cmds == JV6xxx::UartCMD::OUTP_ON){
        emit ChannelBus::instance().CH_isOutputChanged(m_channel,true);
        return;
    }

    float shf = m_readparam.size()==4 ? qFromBigEndian<float>(m_readparam.constData()) : 0.0f;

    if (cmds == JV6xxx::UartCMD::VOLT){
        qCDebug(uartCh)<<"(handleNotScpicmd)Ch_"<<m_channel<<" float: "<<shf;
        emit ChannelBus::instance().CH_cvChanged(m_channel,shf);
        return;
    }else if(cmds == JV6xxx::UartCMD::CURR){
        qCDebug(uartCh)<<"(handleNotScpicmd)Ch_"<<m_channel<<" float: "<<shf;
        emit ChannelBus::instance().CH_ccChanged(m_channel,shf);
        return;
    }else if (cmds == JV6xxx::UartCMD::IMP) {
        qCDebug(uartCh)<<"(handleNotScpicmd)Ch_"<<m_channel<<" float: "<<shf;
        emit ChannelBus::instance().CH_impChanged(m_channel,shf);
        return;
    }else if (cmds == JV6xxx::UartCMD::OVP){
        qCDebug(uartCh)<<"(handleNotScpicmd)Ch_"<<m_channel<<" float: "<<shf;
        emit ChannelBus::instance().CH_ovpChanged(m_channel,shf);
        return;
    }else if (cmds == QCmd(JV6xxx::UartCMD::RVoltage)) {
        qCDebug(uartCh)<<"(handleNotScpicmd)Ch_"<<m_channel<<" float: "<<shf;
        emit ChannelBus::instance().CH_VoltageChanged(m_channel,shf);
        return;
    }else if(cmds == QCmd(JV6xxx::UartCMD::RCurrent)){
        qCDebug(uartCh)<<"(handleNotScpicmd)Ch_"<<m_channel<<" float: "<<shf;
        emit ChannelBus::instance().CH_CurrentAndUnitChanged(m_channel,shf);
        return;
    }

    int uit = 0; // quint +  / qint +-
    if (m_readparam.size() == 2){
        uit = qFromBigEndian<qint16>(m_readparam.constData());
    }else if (m_readparam.size() == 1){
        uit = static_cast<qint8>(m_readparam[0]);
    }

    if (cmds == JV6xxx::UartCMD::CurrRANG) {
        qCDebug(uartCh)<<"(handleNotScpicmd)Ch_"<<m_channel<<" int: "<<uit;
        emit ChannelBus::instance().CH_RangeChanged(m_channel,uit);
        return;
    }else if(cmds == QCmd(JV6xxx::UartCMD::StaREG)){
        qCDebug(uartCh)<<"(handleNotScpicmd)Ch_"<<m_channel<<" int: "<<uit;
        emit ChannelBus::instance().CH_StatusChanged(m_channel,uit);
        return;
    }

    QString str = ""; QTextStream ss(&str);
    if (m_readparam.size() == 4){
        const quint8* b = reinterpret_cast<const quint8*>(m_readparam.constData());
        ss << b[0] << '.' << b[1] << '.' << b[2] << '.' << b[3];
    }

    if (cmds == QCmd(JV6xxx::UartCMD::SV)) {
        qCDebug(uartCh)<<"(handleNotScpicmd)Ch_"<<m_channel<<" String: "<<str;
        emit ChannelBus::instance().CH_svChanged(m_channel,str);
        isExist = true;
        return;
    }else if(cmds == QCmd(JV6xxx::UartCMD::HV)){
        qCDebug(uartCh)<<"(handleNotScpicmd)Ch_"<<m_channel<<" String: "<<str;
        emit ChannelBus::instance().CH_hvChanged(m_channel,str);
        isExist = true;
        return;
    }
}
void UartChannelManager::handleScpiQcmd(quint16 cmds)
{
    switch (cmds) {
        // ---------- output -------------
        case QCmd(JV6xxx::UartCMD::OUTP_OFF): // OUPT_STAT
        case QCmd(JV6xxx::UartCMD::OUTPBAND):
        {
            int uit = m_readparam.size() == 1 ? static_cast<qint8>(m_readparam[0]) : 0;
            qCDebug(uartCh)<<"(handleScpiQcmd)Ch_"<<m_channel<<" int: "<<uit;
            ScpiManager::instance().processCHBoolResponse(m_channel,uit!=0);
            break;
        }
        case QCmd(JV6xxx::UartCMD::OUTPMODE):
        case QCmd(JV6xxx::UartCMD::OUTPCOMP):
        // ---------- control -------------
        case QCmd(JV6xxx::UartCMD::EnOVP):
        case QCmd(JV6xxx::UartCMD::EnOCP):
        case QCmd(JV6xxx::UartCMD::SetPON):
        case QCmd(JV6xxx::UartCMD::LoadINDE):
        case QCmd(JV6xxx::UartCMD::LoadOCP):
        case QCmd(JV6xxx::UartCMD::LoadCurrSta):
        case QCmd(JV6xxx::UartCMD::SetOvpMod):
        // ---------- Measure -------------
        case QCmd(JV6xxx::UartCMD::RFSpeed):
        case QCmd(JV6xxx::UartCMD::CurrRANG):
        case QCmd(JV6xxx::UartCMD::SetFUNC):
        case QCmd(JV6xxx::UartCMD::SwePOIN):
        case QCmd(JV6xxx::UartCMD::SweOffPin):
        // ---------- Register -------------
        case QCmd(JV6xxx::UartCMD::StaREG):
        case QCmd(JV6xxx::UartCMD::EnREG):
        case QCmd(JV6xxx::UartCMD::CondREG):
        case QCmd(JV6xxx::UartCMD::ERR):
        case QCmd(JV6xxx::UartCMD::CONF):
        // ---------- calibrate -------------
        case QCmd(JV6xxx::UartCMD::CalStarALL):
        // ---------- Trigger -------------
        case QCmd(JV6xxx::UartCMD::ACQTRIGSOUR):
        case QCmd(JV6xxx::UartCMD::COUN_COMM):
        case QCmd(JV6xxx::UartCMD::SLOP_COMM):
        {
            int uit = 0; // quint +  / qint +-
            if (m_readparam.size() == 4){
                uit = qFromBigEndian<qint32>(m_readparam.constData());
            }else if (m_readparam.size() == 2){
                uit = qFromBigEndian<qint16>(m_readparam.constData());
            }else if (m_readparam.size() == 1){
                uit = static_cast<qint8>(m_readparam[0]);
            }
            qCDebug(uartCh)<<"(handleScpiQcmd)Ch_"<<m_channel<<" int: "<<uit;
            ScpiManager::instance().processCHIntResponse(m_channel,uit);
            break;
        }
        // ---------- setting -------------
        case QCmd(JV6xxx::UartCMD::VOLT):
        case QCmd(JV6xxx::UartCMD::CURR):
        case QCmd(JV6xxx::UartCMD::IMP):
        case QCmd(JV6xxx::UartCMD::OVP):
        case QCmd(JV6xxx::UartCMD::OTP):
        case QCmd(JV6xxx::UartCMD::SetLoadCurr):
        // ---------- control -------------
        case QCmd(JV6xxx::UartCMD::VoltTime):
        case QCmd(JV6xxx::UartCMD::CurrTime):
        case QCmd(JV6xxx::UartCMD::TherTime):
        // ---------- Measure -------------
        case QCmd(JV6xxx::UartCMD::RVoltage):
        case QCmd(JV6xxx::UartCMD::RCurrent):
        case QCmd(JV6xxx::UartCMD::RSCurrent):
        case QCmd(JV6xxx::UartCMD::RBT):
        case QCmd(JV6xxx::UartCMD::RHT):
        case QCmd(JV6xxx::UartCMD::RDVMACDC):
        case QCmd(JV6xxx::UartCMD::RDVMDC):
        case QCmd(JV6xxx::UartCMD::RDVMAC):
        case QCmd(JV6xxx::UartCMD::TMP1):
        case QCmd(JV6xxx::UartCMD::TMP2):
        case QCmd(JV6xxx::UartCMD::TMP3):
        case QCmd(JV6xxx::UartCMD::PCurrHIGH):
        case QCmd(JV6xxx::UartCMD::PCurrLOW):
        case QCmd(JV6xxx::UartCMD::PCurrMAX):
        case QCmd(JV6xxx::UartCMD::PCurrMIN):
        case QCmd(JV6xxx::UartCMD::PDvmHIGH):
        case QCmd(JV6xxx::UartCMD::PDvmLOW):
        case QCmd(JV6xxx::UartCMD::PDvmMAX):
        case QCmd(JV6xxx::UartCMD::PDvmMIN):
        case QCmd(JV6xxx::UartCMD::PVoltHIGH):
        case QCmd(JV6xxx::UartCMD::PVoltLOW):
        case QCmd(JV6xxx::UartCMD::PVoltMAX):
        case QCmd(JV6xxx::UartCMD::PVoltMIN):
        case QCmd(JV6xxx::UartCMD::SweTINT):
        case QCmd(JV6xxx::UartCMD::CurrRangTIM):
        case QCmd(JV6xxx::UartCMD::RArrVOLT):
        case QCmd(JV6xxx::UartCMD::RArrCURR):
        case QCmd(JV6xxx::UartCMD::RArrDVM):
        case QCmd(JV6xxx::UartCMD::RVoltADC):
        case QCmd(JV6xxx::UartCMD::RCurrADC):
        case QCmd(JV6xxx::UartCMD::RSCurrADC):
        case QCmd(JV6xxx::UartCMD::RDvmADC):
        // ---------- Trigger -------------
        case QCmd(JV6xxx::UartCMD::HYST_COMM):
        case QCmd(JV6xxx::UartCMD::LEV_COMM):
        case QCmd(JV6xxx::UartCMD::SOUR_VOLT_TRIG):
        case QCmd(JV6xxx::UartCMD::SOUR_CURR_TRIG):
        case QCmd(JV6xxx::UartCMD::SOUR_RES_TRIG):
        {
            float shf = m_readparam.size()==4 ? qFromBigEndian<float>(m_readparam.constData()) : 0.0f;
            qCDebug(uartCh)<<"(handleScpiQcmd)Ch_"<<m_channel<<" float: "<<shf;
            ScpiManager::instance().processCHFloatResponse(m_channel,shf);
            break;
        }
        // ---------- Register -------------
        case QCmd(JV6xxx::UartCMD::SV):
        case QCmd(JV6xxx::UartCMD::HV):
        case QCmd(JV6xxx::UartCMD::COMREG):
        {
            QString str = ""; QTextStream ss(&str);
            if (m_readparam.size() == 4){
                const quint8* b = reinterpret_cast<const quint8*>(m_readparam.constData());
                ss << b[0] << '.' << b[1] << '.' << b[2] << '.' << b[3];
            }
            qCDebug(uartCh)<<"(handleScpiQcmd)Ch_"<<m_channel<<" String: "<<str;
            ScpiManager::instance().processCHStringResponse(m_channel,str);
            break;
        }
        default:
            qCWarning(uartCh)<<"(handleScpiQcmd)Ch_"<<m_channel<<" not expect cmd: "<<cmds;
            ScpiManager::instance().processCHVoidResponse(m_channel);
            break;
    }
}

void UartChannelManager::handleCalibrationcmd(quint8 func)
{
    if (m_readparam.size()==4 && m_isScpi){ // // Query and Set
        float shf = qFromBigEndian<float>(m_readparam.constData()); // step = func
        ScpiManager::instance().processCHFloatResponse(m_channel,shf);

        emit ChannelBus::instance().to_CanServer(m_channel,0x07,m_readparam);
        qCDebug(uartCh)<<"(handleCalibrationcmd)Ch_"<<m_channel<<" step: "<<func<<" CD: "<<shf;
    }
}
void UartChannelManager::handleErrorcmd(quint8 func)
{
    QString errstr;
    switch (func){
        case JV6xxx::Err::CheckSum_Error:
            errstr = "JV6xxx::CheckSum Error";
            break;
        case JV6xxx::Err::Unknow_Command:
            errstr = "JV6xxx::Unknow Command";
            break;
        case JV6xxx::Err::Unknow_Function:
            errstr = "JV6xxx::Unknow Function";
            break;
        case JV6xxx::Err::Error_Length:
            errstr = "JV6xxx::Error Length";
            break;
        case JV6xxx::Err::Invalid_Parameter:
            errstr = "JV6xxx::Invalid Parameter";
            break;
        case JV6xxx::Err::Illegal_Command:
            errstr = "JV6xxx::Illegal Command";
            break;
        case JV6xxx::Err::Unsupported_Command:
            errstr = "JV6xxx::Unsupported Command";
            break;
        default:
            errstr = "JV6xxx::Unknow Error";
            break;
    }

    qCDebug(uartCh)<<"(handleErrorcmd)Ch_"<<m_channel<<" Error: "<<errstr;
    ScpiManager::instance().processCHStringResponse(m_channel,errstr);
}
