#include "auxiliary/global_static.h"
#include "auxiliary/signal_bus.h"
#include "uart_protocol.h"
#include "uart_channel.h"
#include "scpi_handle.h"
#include <QSerialPort>
#include <QtEndian>
#include <QTimer>

const QVector<UartChannelManager::Command> UartChannelManager::m_commands =
{
    {0x04, 0x80, "",false},// voltage
    {0x04, 0x81, "",false},// current
    {0x05, 0x80, "",false},// status
};
const QVector<UartChannelManager::Command> UartChannelManager::m_initCommands =
{
    {0x05, 0x84, "",false},// query software
    {0x05, 0x85, "",false},// query Hardware
    {0x01, 0x00, "",false},// Turnoff output
    {0x04, 0x0e, QByteArray::fromHex("00"),false},// set A Unit
    {0x04, 0x0c, QByteArray::fromHex("3f 80 00 00"),false},// set NPLC =1
    {0x04, 0x1e, QByteArray::fromHex("37 82 dc bf"),false},// set Tint =1.56e-05
    {0x04, 0x0f, QByteArray::fromHex("01"),false},// set measure average =1
    {0x02, 0x00, QByteArray::fromHex("00 00 00 00"),false},// set cv =0
    {0x02, 0x01, QByteArray::fromHex("3f 80 00 00"),false},// set cc =1
    {0x02, 0x03, QByteArray::fromHex("41 00 00 00"),false},// set ovp =8
    {0x04, 0x1d, QByteArray::fromHex("00 01"),false},// set Output impedance step =1
    {0x02, 0x02, QByteArray::fromHex("00 00 00 00"),false},// set Output impedance =0 -> cv model
    //{0x04, 0x1f, QByteArray::fromHex("ff ff ff ff")},// set relarge, not vaild parameter
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
            qCWarning(uartCh)<<"[initUartchannel]:Channel_"<<m_channel<<" Error: "<<m_serialPort->errorString();
            }, Qt::DirectConnection);

        m_refreshTimer = new QTimer(this);
        m_refreshTimer->setInterval(180); // ms -> 60ms < target < at will
        connect(m_refreshTimer,&QTimer::timeout,this,[this]{
            m_timeindex = (m_timeindex + 1) % m_commands.size();
            const Command &cmd = m_commands[m_timeindex];
            writeFrame(cmd.cmd, cmd.func, cmd.param, false);
            }, Qt::DirectConnection);

        m_outTimer = new QTimer(this);
        m_outTimer->setSingleShot(true);
        m_outTimer->setInterval(120); // 120 ms
        connect(m_outTimer, &QTimer::timeout, this, [this]{
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
       writeFrame(cmd.cmd, cmd.func, cmd.param, false);

       // disable direct call this function, will block reception
       QTimer::singleShot(36, this, &UartChannelManager::sendInitCommand); // 36 ms
       m_initindex++;
       return;
   }else if(isExist){
       m_refreshTimer->start();
       m_initindex = 0;
   }
}

void UartChannelManager::writeFrame(quint8 cmd, quint8 func, const QByteArray& param,bool isScpi)
{
     m_txWaitQueue.enqueue({cmd, func, param, isScpi});
     sendNextCmd();
}
UartChannelManager::UartChannelManager(QObject *parent):QObject(parent)
{}

/*
 * 1，队列等待-控制发送速度（收一发一）+ 丢包超时（跳过），保证正常状态一收一发
 * 2，超时-过期响应，加入校验码，跳过过期响应，避免污染应答-可以延长超时限制，去包含它
 * 3，状态判断是否发送远程模块
 *
 * */

void UartChannelManager::sendNextCmd()
{
    if (!m_waitR && !m_txWaitQueue.isEmpty()) {
        const Command& cmd = m_txWaitQueue.dequeue();

        quint8 length = 4 + cmd.param.size();  //  Command+Function+Channel+CheckSum  + Parameter
        quint8 checksum = length + cmd.cmd + cmd.func + m_channel; // The check code is taken from the lowest 8 bits.
        for (char byte : cmd.param) {checksum += static_cast<quint8>(byte);}

        m_responsebuffer.clear();
        m_responsebuffer.reserve(length + 4);  // Pre-allocation enhances performance
        m_responsebuffer.append(JV6xxx::HEADER_HIGH);
        m_responsebuffer.append(JV6xxx::HEADER_LOW);
        m_responsebuffer.append(length);
        m_responsebuffer.append(cmd.cmd);
        m_responsebuffer.append(cmd.func);
        m_responsebuffer.append(m_channel);
        m_responsebuffer.append(cmd.param);
        m_responsebuffer.append(checksum);
        m_responsebuffer.append(JV6xxx::END_MARKER);
        m_serialPort->write(m_responsebuffer);

        m_waitR = true;
        m_outTimer->start();
        m_isScpi = cmd.isScpi;
        m_resChecked = (static_cast<quint16>(cmd.cmd) << 8) | cmd.func; // 01 | 02 = 0x0102
        qCDebug(uartCh)<<"[writeFrame]:Ch_"<<m_channel<<" Send: "<<m_responsebuffer.toHex(' ');
    }
}
void UartChannelManager::handleReadyRead()
{
    m_readbuffer.append(m_serialPort->readAll());

    // Normal response processing of the protocol
    if (m_readbuffer.size() >= 3){
        if(static_cast<quint8>(m_readbuffer[0]) == JV6xxx::HEADER_LOW && static_cast<quint8>(m_readbuffer[1]) == JV6xxx::HEADER_HIGH){
            qCDebug(uartCh)<<"[handleReadyRead]:Ch_"<<m_channel<<" Received: "<<m_readbuffer.toHex(' ');
            quint8 lengthB = static_cast<quint8>(m_readbuffer[2]);

            if (m_readbuffer.size() >= lengthB + 4){
                if(static_cast<quint8>(m_readbuffer[lengthB + 3]) == JV6xxx::END_MARKER){
                    quint8 cmd = static_cast<quint8>(m_readbuffer[3]);
                    quint8 func = static_cast<quint8>(m_readbuffer[4]);
                    quint8 ch = static_cast<quint8>(m_readbuffer[5]);
                    m_readparam = m_readbuffer.mid(6, lengthB - 4);   // length - Command+Function+Channel+CheckSum

                    quint8 Checksum = lengthB + cmd + func + ch; // The check code is taken from the lowest 8 bits.
                    for (char byte : qAsConst(m_readparam)) {Checksum += static_cast<quint8>(byte);}

                    quint16 selfCheck = (static_cast<quint16>(cmd) << 8) | func;
                    if(static_cast<quint8>(m_readbuffer[lengthB + 2]) == Checksum && m_resChecked == selfCheck){
                        switch (cmd) {
                            case JV6xxx::OUTPUT:               handleOutputcmd          (func);break;
                            case JV6xxx::SETTING:              handleSettingcmd         (func);break;
                            case JV6xxx::CONTROL:              handleControlcmd         (func);break;
                            case JV6xxx::MEASUREMENT:          handleMeasurementcmd     (func);break;
                            case JV6xxx::REGISTER:             handleRegistercmd        (func);break;
                            case JV6xxx::CALIBRATE:            handleCalibratecmd       (func);break;
                            case JV6xxx::CALIBRATION:          handleCalibrationcmd     (func);break;
                            case JV6xxx::TRIGGER:              handleTriggercmd         (func);break;
                            case JV6xxx::ERROR:                handleErrorcmd           (func);break;
                        }

                        if (ConfigManager::control_Mode == InterSt::CAN){
                            emit ChannelBus::instance().to_CanServer(m_channel, selfCheck, m_readparam);
                        }

                        m_readbuffer.remove(0,lengthB + 4);
                        if(!m_readbuffer.isEmpty()){handleReadyRead();}

                        m_outTimer->stop();
                        m_resChecked = 0;
                        m_waitR = false;
                        sendNextCmd();
                        return; // finish.
                    }
                }

                qCDebug(uartCh)<<"[handleReadyRead]:Ch_"<<m_channel<<" Read Error: "<<m_readbuffer;
                m_readbuffer.clear();
            }

            return; // Insufficient length; wait to be completed.
        }

        qCDebug(uartCh)<<"[handleReadyRead]:Ch_"<<m_channel<<" Read Error: "<<m_readbuffer;
        m_readbuffer.clear();
    }
}

void UartChannelManager::handleOutputcmd(quint8 func)
{
    quint32 sh = m_readparam.size()==1 ? static_cast<quint32>(m_readparam[0]) : (m_readparam.size()==4) ? qFromBigEndian<quint32>(m_readparam.constData()) : 0;
    qCDebug(uartCh)<<"[handleOutputcmd]:Ch_"<<m_channel<<" ["<<func<<"] "<<sh;

    switch (func){
        case JV6xxx::Output::QUERY_OUTPSTATUS:case JV6xxx::Output::QUERY_OUTPBAND:
            if (m_isScpi){ScpiManager::instance().processCHBoolResponse(sh!=0);}
            return;
        case JV6xxx::Output::QUERY_OUTPMODE:case JV6xxx::Output::QUERY_OUTPCOMP_MODE:
            if (m_isScpi){ScpiManager::instance().processCHIntResponse(sh);}
            return;

        case JV6xxx::Output::SET_OUTPSTATUS_OFF:case JV6xxx::Output::SET_OUTPSTATUS_ON:
            emit ChannelBus::instance().CH_isOutputChanged(m_channel,func == 1);

            if (m_isScpi){
                if (ConfigManager::control_Mode == InterSt::CAN){
                    QByteArray param(1,static_cast<char>(func));
                    emit ChannelBus::instance().to_CanServer(m_channel,0x01,param);
                }
                ScpiManager::instance().processCHVoidResponse();
            }
            return;
        case JV6xxx::Output::SET_OUTPMODE:case JV6xxx::Output::SET_OUTPBAND:
        case JV6xxx::Output::SET_OUTPCOMP_MODE:default:
            if (m_isScpi){ScpiManager::instance().processCHVoidResponse();}
            return;
    }
}
void UartChannelManager::handleSettingcmd(quint8 func)
{
    float shf = m_readparam.size()==4 ? qFromBigEndian<float>(m_readparam.constData()) : 0.0f;
    qCDebug(uartCh)<<"[handleSettingcmd]:Ch_"<<m_channel<<" ["<<func<<"] "<<shf;

    switch (func){
        case JV6xxx::Setting::QUERY_SOURVOLT:case JV6xxx::Setting::QUERY_SOURCURR:
        case JV6xxx::Setting::QUERY_SOURIMP:case JV6xxx::Setting::QUERY_SOURVOLT_PROT:
        case JV6xxx::Setting::QUERY_THERPROT:case JV6xxx::Setting::QUERY_LOADCURR:
            if (m_isScpi){ScpiManager::instance().processCHFloatResponse(shf);}
            return;

        case JV6xxx::Setting::SET_SOURVOLT:
            if (m_isScpi){ScpiManager::instance().processCHVoidResponse();}
            emit ChannelBus::instance().CH_cvChanged(m_channel,shf);
            return;
        case JV6xxx::Setting::SET_SOURCURR:
            if (m_isScpi){ScpiManager::instance().processCHVoidResponse();}
            emit ChannelBus::instance().CH_ccChanged(m_channel,shf);
            return;
        case JV6xxx::Setting::SET_SOURIMP:
            if (m_isScpi){ScpiManager::instance().processCHVoidResponse();}
            emit ChannelBus::instance().CH_impChanged(m_channel,shf);
            return;
        case JV6xxx::Setting::SET_SOURVOLT_PROT:
            if (m_isScpi){ScpiManager::instance().processCHVoidResponse();}
            emit ChannelBus::instance().CH_ovpChanged(m_channel,shf);
            return;
        case JV6xxx::Setting::SET_THERPROT:case JV6xxx::Setting::SET_LOADCURR:default:
            if (m_isScpi){ScpiManager::instance().processCHVoidResponse();}
            return;
    }
}
void UartChannelManager::handleControlcmd(quint8 func)
{
    quint16 sh = m_readparam.size()==1 ? static_cast<quint16>(m_readparam[0]) : (m_readparam.size()==2) ? qFromBigEndian<quint16>(m_readparam.constData()) : 0;
    float  shf = m_readparam.size()==4 ? qFromBigEndian<float>(m_readparam.constData())  :  0.0f;
    qCDebug(uartCh)<<"[handleControlcmd]:Ch_"<<m_channel<<" ["<<func<<"] "<<sh<<","<<shf;

    switch (func){
        case JV6xxx::Control::QUERY_SOURVOLT_CLAM:case JV6xxx::Control::QUERY_SOURCURR_TYPE:
        case JV6xxx::Control::QUERY_SYSTPOS:case JV6xxx::Control::QUERY_LOADINDE_STAT:
        case JV6xxx::Control::QUERY_LOADCURR_TYPE:case JV6xxx::Control::QUERY_SOURCURR_PRIO:
        case JV6xxx::Control::QUERY_SOURVOLT_ABS:
            if (m_isScpi){ScpiManager::instance().processCHIntResponse(sh);}
            return;
        case JV6xxx::Control::QUERY_SOURVOLT_TIME:case JV6xxx::Control::QUERY_SOURCURR_TIME:
        case JV6xxx::Control::QUERY_THERTIME:
            if (m_isScpi){ScpiManager::instance().processCHFloatResponse(shf);}
            return;

        case JV6xxx::Control::SET_SOURVOLT_CLAM:case JV6xxx::Control::SET_SOURCURR_TYPE:
        case JV6xxx::Control::SET_SYSTPOS:case JV6xxx::Control::SET_SAV:case JV6xxx::Control::SET_RCL:
        case JV6xxx::Control::SET_LOADINDE_STAT:case JV6xxx::Control::SET_LOADCURR_TYPE:
        case JV6xxx::Control::SET_SOURCURR_PRIO:case JV6xxx::Control::SET_SOURVOLT_ABS:
        case JV6xxx::Control::SET_SOURVOLT_TIME:case JV6xxx::Control::SET_SOURCURR_TIME:
        case JV6xxx::Control::SET_THERTIME:default:
            if (m_isScpi){ScpiManager::instance().processCHVoidResponse();}
            return;
    }
}
void UartChannelManager::handleMeasurementcmd(quint8 func)
{ // new protocol Needs to be completed
    quint16 sh = m_readparam.size()==1 ? static_cast<quint16>(m_readparam[0]) : (m_readparam.size()==2) ? qFromBigEndian<quint16>(m_readparam.constData()) : 0;
    float  shf = m_readparam.size()==4 ? qFromBigEndian<float>(m_readparam.constData())  :  0.0f;
    qCDebug(uartCh)<<"[handleMeasurementcmd]:Ch_"<<m_channel<<" ["<<func<<"] "<<sh<<","<<shf;

    switch (func){
        case JV6xxx::Measurement::QUERY_MEASVOLT:
            if (m_isScpi){ScpiManager::instance().processCHFloatResponse(shf);} // Otherwise, conflicts.
            emit ChannelBus::instance().CH_VoltageChanged(m_channel,shf);
            return;
        case JV6xxx::Measurement::QUERY_MEASCURR:
            if (m_isScpi){ScpiManager::instance().processCHFloatResponse(shf);} // Otherwise, conflicts.
            emit ChannelBus::instance().CH_CurrentAndUnitChanged(m_channel,shf);
            return;
        case JV6xxx::Measurement::QUERY_SENSSWE_OFFS_POIN:{
            int sht = m_readparam.size()==4 ? qFromBigEndian<int>(m_readparam.constData())  :  0;
            if (m_isScpi){ScpiManager::instance().processCHIntResponse(sht);}
            return;
        }
        case JV6xxx::Measurement::QUERY_MEASSCUR:case JV6xxx::Measurement::QUERY_MEASBTMP:
        case JV6xxx::Measurement::QUERY_MEASHTMP:case JV6xxx::Measurement::QUERY_MEASDVM_ACDC:
        case JV6xxx::Measurement::QUERY_MEASDVM:case JV6xxx::Measurement::QUERY_MEASDVM_AC:
        case JV6xxx::Measurement::QUERY_MEASTMP1:case JV6xxx::Measurement::QUERY_MEASTMP2:case JV6xxx::Measurement::QUERY_MEASTMP3:
        case JV6xxx::Measurement::QUERY_MEASADC_VOLT:case JV6xxx::Measurement::QUERY_MEASADC_CURR:
        case JV6xxx::Measurement::QUERY_MEASADC_SCUR:case JV6xxx::Measurement::QUERY_MEASADC_DVM:
        case JV6xxx::Measurement::QUERY_FETCCURR_HIGH:case JV6xxx::Measurement::QUERY_FETCCURR_LOW:
        case JV6xxx::Measurement::QUERY_FETCCURR_MAX:case JV6xxx::Measurement::QUERY_FETCCURR_MIN:
        case JV6xxx::Measurement::QUERY_FETCDVM_HIGH:case JV6xxx::Measurement::QUERY_FETCDVM_LOW:
        case JV6xxx::Measurement::QUERY_FETCDVM_MAX:case JV6xxx::Measurement::QUERY_FETCDVM_MIN:
        case JV6xxx::Measurement::QUERY_FETCVOLT_HIGH:case JV6xxx::Measurement::QUERY_FETCVOLT_LOW:
        case JV6xxx::Measurement::QUERY_FETCVOLT_MAX:case JV6xxx::Measurement::QUERY_FETCVOLT_MIN:
        case JV6xxx::Measurement::QUERY_MEASARR_VOLT:case JV6xxx::Measurement::QUERY_MEASARR_CURR:case JV6xxx::Measurement::QUERY_MEASARR_DVM:
        case JV6xxx::Measurement::QUERY_SENSSWE_TINT:case JV6xxx::Measurement::QUERY_SENSCURR_RANGE_TIME:
            if (m_isScpi){ScpiManager::instance().processCHFloatResponse(shf);}
            return;
        case JV6xxx::Measurement::QUERY_THERFAN:case JV6xxx::Measurement::QUERY_THERDUTY://case 0x9d:  :SENS:SWE:POIN?
        case JV6xxx::Measurement::QUERY_SENSCURR_RANGE:case JV6xxx::Measurement::QUERY_SENSFUNC:
            if (m_isScpi){ScpiManager::instance().processCHIntResponse(sh);}
            return;
        case JV6xxx::Measurement::QUERY_SENSNPLC:case JV6xxx::Measurement::QUERY_SENSAVER:
            if (m_isScpi){ScpiManager::instance().processCHVoidResponse();}
            return;


        case JV6xxx::Measurement::SET_SENSCURR_RANGE:
            if (m_isScpi){ScpiManager::instance().processCHVoidResponse();}
            emit ChannelBus::instance().CH_RangeChanged(m_channel,sh);
            return;
        case JV6xxx::Measurement::SET_SENSNPLC:case JV6xxx::Measurement::SET_SENSCURR_RANGE_TIM:
        case JV6xxx::Measurement::SET_SENSAVER:case JV6xxx::Measurement::SET_SENSFUNC:
        case JV6xxx::Measurement::SET_SENSSWE_OFFS_POIN:case JV6xxx::Measurement::SET_SENSSWE_POIN:
        case JV6xxx::Measurement::SET_SENSSWE_TINT:default:
            if (m_isScpi){ScpiManager::instance().processCHVoidResponse();}
            return;
    }
}
void UartChannelManager::handleRegistercmd(quint8 func)
{
    QString str; QTextStream ss(&str);
    if (m_readparam.size() == 4){
        const quint8* b = reinterpret_cast<const quint8*>(m_readparam.constData());
        ss << b[0] << '.' << b[1] << '.' << b[2] << '.' << b[3];
    }

    quint16 sh = m_readparam.size()==1 ? static_cast<quint16>(m_readparam[0]) : (m_readparam.size()==2) ? qFromBigEndian<quint16>(m_readparam.constData()) : 0;
    qCDebug(uartCh)<<"[handleRegistercmd]:Ch_"<<m_channel<<" ["<<func<<"] "<<sh<<","<<str;;

    switch (func){
        case JV6xxx::Register::QUERY_STATOPER_EVEN:
            if (m_isScpi){ScpiManager::instance().processCHIntResponse(sh);}
            emit ChannelBus::instance().CH_StatusChanged(m_channel,sh);
            return;
        case JV6xxx::Register::QUERY_SYSTENAB:case JV6xxx::Register::QUERY_STATOPER_COND:
        case JV6xxx::Register::QUERY_SYSTCONF:case JV6xxx::Register::QUERY_STATQUE_NEXT:
            if (m_isScpi){ScpiManager::instance().processCHIntResponse(sh);}
            return;
        case JV6xxx::Register::QUERY_SYSTSWV:
            if (m_isScpi){ScpiManager::instance().processCHStringResponse(str);}
            emit ChannelBus::instance().CH_svChanged(m_channel,str);
            isExist = true;
            return;
        case JV6xxx::Register::QUERY_SYSTHWV:
            if (m_isScpi){ScpiManager::instance().processCHStringResponse(str);}
            emit ChannelBus::instance().CH_hvChanged(m_channel,str);
            isExist = true;
            return;
        case JV6xxx::Register::QUERY_SYSTREG:
            if (m_isScpi){ScpiManager::instance().processCHStringResponse(str);}
            return;

        case JV6xxx::Register::SET_STATQUE_CLEAR:default:
            if (m_isScpi){ScpiManager::instance().processCHVoidResponse();}
            return;
    }
}
void UartChannelManager::handleCalibratecmd(quint8 func)
{
    switch (func){
        case JV6xxx::Calibrate::QUERY_CALISTAR_ALL:{
            quint8 sh = m_readparam.size() == 1 ? static_cast<quint8>(m_readparam[0]) : 0;
            if (m_isScpi){ScpiManager::instance().processCHIntResponse(sh);}
            qCDebug(uartCh)<<"[handleCalibratecmd]:Ch_"<<m_channel<<" | "<<sh;
            return;
        }

        case JV6xxx::Calibrate::SET_CALIEXIT:case JV6xxx::Calibrate::SET_CALIINIT:
        case JV6xxx::Calibrate::SET_CALIREST:case JV6xxx::Calibrate::SET_CALISAVE:
        case JV6xxx::Calibrate::SET_CALISTART_ALL:case JV6xxx::Calibrate::SET_CALISTART_ADC:
        case JV6xxx::Calibrate::SET_CALISTART_DAC:case JV6xxx::Calibrate::SET_CALITART_ENAB:
        case JV6xxx::Calibrate::SET_CALISTART_IMP:case JV6xxx::Calibrate::SET_CALISTART_DCP:
        case JV6xxx::Calibrate::SET_CALISTART_DCN:case JV6xxx::Calibrate::SET_CALICLEARTC:
        case JV6xxx::Calibrate::SET_CALISAVETC:case JV6xxx::Calibrate::SET_CALIUPDATETC:default:
            if (m_isScpi){ScpiManager::instance().processCHVoidResponse();}
            return;
    }
}
void UartChannelManager::handleCalibrationcmd(quint8 func)
{
    if (m_readparam.size()==4 && m_isScpi){ // // Query and Set
        float shf = qFromBigEndian<float>(m_readparam.constData()); // step = func
        ScpiManager::instance().processCHFloatResponse(shf);

        emit ChannelBus::instance().to_CanServer(m_channel,0x07,m_readparam);
        qCDebug(uartCh)<<"[handleCalibrationcmd]:Ch_"<<m_channel<<" step: "<<func<<" CD: "<<shf;
    }
}
void UartChannelManager::handleTriggercmd(quint8 func)
{ // new protocol Needs to be change
    quint16 sh = m_readparam.size()==1 ? static_cast<quint16>(m_readparam[0]) : (m_readparam.size()==2) ? qFromBigEndian<quint16>(m_readparam.constData()) : 0;
    float  shf = m_readparam.size()==4 ? qFromBigEndian<float>(m_readparam.constData())  :  0.0f;
    qCDebug(uartCh)<<"[handleTriggercmd]:Ch_"<<m_channel<<" ["<<func<<"] "<<sh<<","<<shf;

    switch (func){
        case JV6xxx::Trigger::QUERY_TRIGSEQ2_SOUR:case JV6xxx::Trigger::QUERY_TRIGSEQ2_COUN:
        case JV6xxx::Trigger::QUERY_TRIGSEQ2_SLOP:
            if (m_isScpi){ScpiManager::instance().processCHIntResponse(sh);}
            return;
        case JV6xxx::Trigger::QUERY_TRIGSEQ2_HYST:case JV6xxx::Trigger::QUERY_TRIGSEQ2_LEV:
        case JV6xxx::Trigger::QUERY_SOURVOLT_AMPL_TRIG:case JV6xxx::Trigger::QUERY_SOURCURR_TRIG:
        case JV6xxx::Trigger::QUERY_SOURRES_TRIG:
            if (m_isScpi){ScpiManager::instance().processCHFloatResponse(shf);}
            return;

        case JV6xxx::Trigger::SET_ABORT:case JV6xxx::Trigger::SET_INIT_IMM:
        case JV6xxx::Trigger::SET_INIT_CONT:case JV6xxx::Trigger::SET_TRIGSEQ1:
        case JV6xxx::Trigger::SET_TRIGSEQ2:case JV6xxx::Trigger::SET_TRIGSEQ2_SOUR:
        case JV6xxx::Trigger::SET_TRIGSEQ2_COUN:case JV6xxx::Trigger::SET_TRIGSEQ2_HYST:
        case JV6xxx::Trigger::SET_TRIGSEQ2_LEV:case JV6xxx::Trigger::SET_TRIGSEQ2_SLOP:
        case JV6xxx::Trigger::SET_SOURVOLT_AMPL_TRIG:case JV6xxx::Trigger::SET_SOURCURR_TRIG:
        case JV6xxx::Trigger::SET_SOURRES_TRIG:default:
            if (m_isScpi){ScpiManager::instance().processCHVoidResponse();}
            return;
    }
}
void UartChannelManager::handleErrorcmd(quint8 func)
{
    QString errstr;
    switch (func){
        case 0x00:errstr = "CheckSum Error";      break;
        case 0x01:errstr = "Unknow Command";      break;
        case 0x02:errstr = "Unknow Function";     break;
        case 0x03:errstr = "Error Length";        break;
        case 0x04:errstr = "Invalid Parameter";   break;
        case 0x05:errstr = "Illegal Command";     break;
        case 0x06:errstr = "Unsupported Command"; break;
        default:  errstr = "Unknow Error";        break;
    }

    qCDebug(uartCh)<<"[handleErrorcmd]:Ch_"<<m_channel<<" Error: "<<errstr;
    ScpiManager::instance().processCHStringResponse(errstr);
}
