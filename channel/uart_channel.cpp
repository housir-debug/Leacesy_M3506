#include "uart_channel.h"
#include <QtCore>

UartChannelManager::UartChannelManager(QObject *parent):QObject(parent){}
UartChannelManager::~UartChannelManager()
{
    qCDebug(uart_channel)<<"destruct Chuart"<<m_channel<< "thread:" << QThread::currentThread()->objectName();
    if (m_serialPort && m_serialPort->isOpen()) { m_serialPort->close(); }
    if (m_refreshtimer) { m_refreshtimer->stop(); }
}

Q_LOGGING_CATEGORY(uart_channel, "UART_CHANNEL:")


bool UartChannelManager::initSerialPort(quint8 ch, const QString &portName,qint32 baudRate)
{
    if (!m_serialPort && !m_refreshtimer){
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

        if (m_serialPort->open(QIODevice::ReadWrite)) {
            m_channel = ch;
            m_commands = {
                {0x04, 0x80, ""},// voltage
                {0x04, 0x81, ""},// current
                {0x05, 0x80, ""},// status
            };

            m_refreshtimer = new QTimer(this);
            m_refreshtimer->setInterval(180); // ms -> 60ms < target < at will

            QThread* worker = getWorkerThread(ch);
            this->moveToThread(worker); // The child object will follow the parent class.

            QMetaObject::invokeMethod(this, [this]() {
                connect(m_serialPort, &QSerialPort::readyRead, this, &UartChannelManager::handleReadyRead, Qt::DirectConnection);
                connect(m_serialPort, &QSerialPort::errorOccurred, this, [this]() {
                    qCWarning(uart_channel)<<"[initSerialPort]:Channel_"<<m_channel<<" Error: "<<m_serialPort->errorString();
                    }, Qt::DirectConnection);
                connect(m_refreshtimer,&QTimer::timeout,this,[this]{
                    if (!m_commands.isEmpty()){
                        m_timeindex = (m_timeindex + 1) % m_commands.size();
                        const Command &cmd = m_commands[m_timeindex];
                        writeFrame(cmd.cmd, cmd.func, cmd.param, false);
                    }}, Qt::DirectConnection);

                // Channel initialization command
                sendInitCommand();
            });

            return true;
        }

        qCWarning(uart_channel)<<"[initSerialPort]:failed Open SerialPort: "<<portName;
        return false;
    }

    qCDebug(uart_channel)<<"[initSerialPort]:SerialPort already initial: "<<portName;
    return false;
}

QThread* UartChannelManager::getWorkerThread(quint32 channel) {
    static constexpr int THREAD_COUNT = 4;
    static QThread threads[THREAD_COUNT];
    static std::once_flag flag;

    std::call_once(flag, [] {
        for (int i = 0; i < THREAD_COUNT; i++) {
            threads[i].setObjectName(QString("UartChThread_%1").arg(i));
            threads[i].start();
        }
    });

    return &threads[channel % THREAD_COUNT];
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
   }else if(ConfigManager::s_enableDisplay && isExist){
        m_refreshtimer->start();
        m_initindex = 0;
   }
}


const QVector<Command> UartChannelManager::m_initCommands = {
    {0x05, 0x84, ""},// query software
    {0x05, 0x85, ""},// query Hardware
    {0x01, 0x00, ""},// Turnoff output
    {0x04, 0x0e, QByteArray::fromHex("00")},// set A Unit
    {0x04, 0x0c, QByteArray::fromHex("3f 80 00 00")},// set NPLC =1
    {0x04, 0x1e, QByteArray::fromHex("37 82 dc bf")},// set Tint =1.56e-05
    {0x04, 0x0f, QByteArray::fromHex("01")},// set measure average =1
    {0x02, 0x00, QByteArray::fromHex("00 00 00 00")},// set cv =0
    {0x02, 0x01, QByteArray::fromHex("3f 80 00 00")},// set cc =1
    {0x02, 0x03, QByteArray::fromHex("41 00 00 00")},// set ovp =8
    {0x04, 0x1d, QByteArray::fromHex("00 01")},// set Output impedance step =1
    {0x02, 0x02, QByteArray::fromHex("00 00 00 00")},// set Output impedance =0 -> cv model
    //{0x04, 0x1f, QByteArray::fromHex("ff ff ff ff")},// set relarge, not vaild parameter
};


void UartChannelManager::writeFrame(quint8 cmd, quint8 func, const QByteArray& param,bool isScpi) {
    if (m_waitR) {
        QTimer::singleShot(18, this, [this, cmd, func, param, isScpi] { // 18 ms
            m_waitR = false;
            writeFrame(cmd, func, param, isScpi);
        });

        return;
    }

    quint8 length = 4 + param.size();  //  Command+Function+Channel+CheckSum  + Parameter
    quint8 checksum = length + cmd + func + m_channel; // The check code is taken from the lowest 8 bits.
    for (char byte : param) {checksum += static_cast<quint8>(byte);}

    m_responsebuffer.clear();
    m_responsebuffer.reserve(length + 4);  // Pre-allocation enhances performance
    m_responsebuffer.append(HEADER_HIGH);
    m_responsebuffer.append(HEADER_LOW);
    m_responsebuffer.append(length);
    m_responsebuffer.append(cmd);
    m_responsebuffer.append(func);
    m_responsebuffer.append(m_channel);
    m_responsebuffer.append(param);
    m_responsebuffer.append(checksum);
    m_responsebuffer.append(END_MARKER);

    m_waitR = true;
    m_serialPort->write(m_responsebuffer);
    m_scpiCommand = isScpi ? (static_cast<quint16>(cmd) << 8) | func : 0; // 01 | 02 = 0x0102
    qCDebug(uart_channel)<<"[writeFrame]:Ch_"<<m_channel<<" Send: "<<m_responsebuffer.toHex(' ');
}

void UartChannelManager::handleReadyRead()
{
    m_readbuffer.append(m_serialPort->readAll());

    // Normal response processing of the protocol
    if (m_readbuffer.size() >= 3){
        if(static_cast<quint8>(m_readbuffer[0]) == HEADER_LOW && static_cast<quint8>(m_readbuffer[1]) == HEADER_HIGH){
            qCDebug(uart_channel)<<"[handleReadyRead]:Ch_"<<m_channel<<" Received: "<<m_readbuffer.toHex(' ');
            quint8 lengthB = static_cast<quint8>(m_readbuffer[2]);

            if (m_readbuffer.size() >= lengthB + 4){
                if(static_cast<quint8>(m_readbuffer[lengthB + 3]) == END_MARKER){
                    quint8 cmd = static_cast<quint8>(m_readbuffer[3]);
                    quint8 func = static_cast<quint8>(m_readbuffer[4]);
                    quint8 ch = static_cast<quint8>(m_readbuffer[5]);
                    m_readparam = m_readbuffer.mid(6, lengthB - 4);   // length - Command+Function+Channel+CheckSum

                    quint8 Checksum = lengthB + cmd + func + ch; // The check code is taken from the lowest 8 bits.
                    for (char byte : qAsConst(m_readparam)) {Checksum += static_cast<quint8>(byte);}

                    if(static_cast<quint8>(m_readbuffer[lengthB + 2]) == Checksum){
                        quint16 scpiId = (static_cast<quint16>(cmd) << 8) | func;
                        switch (cmd) {
                            case 0x01:handleOutputcmd          (func,scpiId);break;
                            case 0x02:handleSettingcmd         (func,scpiId);break;
                            case 0x03:handleControlcmd         (func,scpiId);break;
                            case 0x04:handleMeasurementcmd     (func,scpiId);break;
                            case 0x05:handleRegistercmd        (func,scpiId);break;
                            case 0x06:handleCalibratecmd       (func,scpiId);break;
                            case 0x07:handleCalibrationcmd     (func,scpiId);break;
                            case 0x08:handleTriggercmd         (func,scpiId);break;
                            case 0x09:handleISPcmd             (func,scpiId);break;
                            case 0x10:handleSNcmd              (func,scpiId);break;
                            case 0x11:handleIDcmd              (func,scpiId);break;
                            case 0xFF:handleErrorcmd           (func);       break;
                            default:qCDebug(uart_channel)<<"[handleReadyRead]:Ch_"<<m_channel<<" UndefCommand: "<<cmd;
                        }

                        if (m_scpiCommand == scpiId){
                            emit to_CanServer(m_channel, scpiId, m_readparam);
                            m_scpiCommand = 0;
                        }

                        m_readbuffer.remove(0,lengthB + 4);
                        if(!m_readbuffer.isEmpty()){handleReadyRead();}

                        m_waitR = false;
                        return; // finish.
                    }
                }

                qCDebug(uart_channel)<<"[handleReadyRead]:Ch_"<<m_channel<<" end/check format Error!";
                m_readbuffer.clear();
            }

            return; // Insufficient length; wait to be completed.
        }

        qCDebug(uart_channel)<<"[handleReadyRead]:Ch_"<<m_channel<<" head format Error!";
        m_readbuffer.clear();
    }
}

// ************************* Specific command processing *****************************

void UartChannelManager::handleOutputcmd(quint8 func,quint16 scpiIdentify){
    quint32 sh = m_readparam.size()==1 ? static_cast<quint32>(m_readparam[0]) : (m_readparam.size()==4) ? qFromBigEndian<quint32>(m_readparam.constData()) : 0;
    qCDebug(uart_channel)<<"[handleOutputcmd]:Ch_"<<m_channel<<" ["<<func<<"] "<<sh;

    switch (func){
        case 0x80: // :OUTP[:STAT]?
        case 0x88: // :OUTPut[ch]:BAND?
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHStateResponse(sh!=0);}
            return;
        case 0x82: // :OUTPut[ch]:MODE?
        case 0x89: // :OUTPut[ch]:COMP:MODE?
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHIntResponse(sh);}
            return;

        case 0x00: // :OUTP[:STAT] {OFF|0}
            emit CH_isOutputChanged(m_channel,false);
            if (m_scpiCommand == scpiIdentify){
                emit to_CanServer(m_channel,0x01,QByteArray::fromHex("00"));
                m_scpiManager->processCHVoidResponse();
            }
            return;
        case 0x01: // :OUTP[:STAT] {ON|1}
            emit CH_isOutputChanged(m_channel,true);
            if (m_scpiCommand == scpiIdentify){
                emit to_CanServer(m_channel,0x01,QByteArray::fromHex("01"));
                m_scpiManager->processCHVoidResponse();
            }
            return;
        case 0x02: // :OUTPut[ch]:MODE <n>
        case 0x08: // :OUTPut[ch]:BAND:HIGH|LOW
        case 0x09: // :OUTPut[ch]:COMP:MODE type
        case 0x90:case 0x10:case 0x91:case 0x11: // not SCPI cmd
        default:
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHVoidResponse();}
            return;
    }
}

void UartChannelManager::handleSettingcmd(quint8 func,quint16 scpiIdentify){
    float shf = m_readparam.size()==4 ? qFromBigEndian<float>(m_readparam.constData()) : 0.0f;
    qCDebug(uart_channel)<<"[handleSettingcmd]:Ch_"<<m_channel<<" ["<<func<<"] "<<shf;

    switch (func){
        case 0x80: // :SOUR:VOLT[:LEV][:AMPL]?
        case 0x81: // :SOUR:CURR[:LIM][:VAL]?
        case 0x82: // :OUTP:IMP?
        case 0x83: // :SOUR:VOLT:PROT?
        case 0x84: // :THER[:PROT][:TEMP]?
        case 0x85: // :LOAD:CURR[:LIM][:VAL]?
        case 0x86: case 0x87: case 0x88: // SCPI cmd Repeat
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHFloatResponse(shf);}
            return;

        case 0x00: // :SOUR:VOLT[:LEV][:AMPL] <NRf>
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHVoidResponse();}
            emit CH_cvChanged(m_channel,shf);
            return;
        case 0x01: // :SOUR:CURR[:LIM][:VAL] <NRf>
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHVoidResponse();}
            emit CH_ccChanged(m_channel,shf);
            return;
        case 0x02: // :OUTP:IMP <NRf>
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHVoidResponse();}
            emit CH_impChanged(m_channel,shf);
            return;
        case 0x03: // :SOUR:VOLT:PROT <NRf>
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHVoidResponse();}
            emit CH_ovpChanged(m_channel,shf);
            return;
        case 0x04: // :THER[:PROT][:TEMP] <NRf>
        case 0x05: // :LOAD:CURR[:LIM][:VAL] <NRf>
        case 0x06: case 0x07: case 0x08: // SCPI cmd Repeat
        case 0x89: case 0x09: // not SCPI cmd
        default:
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHVoidResponse();}
            return;
    }
}

void UartChannelManager::handleControlcmd(quint8 func,quint16 scpiIdentify){
    quint16 sh = m_readparam.size()==1 ? static_cast<quint16>(m_readparam[0]) : (m_readparam.size()==2) ? qFromBigEndian<quint16>(m_readparam.constData()) : 0;
    float  shf = m_readparam.size()==4 ? qFromBigEndian<float>(m_readparam.constData())  :  0.0f;
    qCDebug(uart_channel)<<"[handleControlcmd]:Ch_"<<m_channel<<" ["<<func<<"] "<<sh<<","<<shf;

    switch (func){
        case 0x81: // :SOUR:VOLT:PROT:CLAM?
        case 0x82: // :SOUR:CURR[:LIM]:TYPE?
        case 0x85: // :SYST:POS?
        case 0x88: // :LOAD:INDE:STAT?
        case 0x89: // :LOAD:CURR[:LIM]:TYPE?
        case 0x8a: // :SOUR:CURR[:LIM]:PRIO?
        case 0x8b: // :SOUR:VOLT:PROT:ABS?
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHIntResponse(sh);}
            return;
        case 0x91: // :SOUR:VOLT:PROT:TIME?
        case 0x92: // :SOUR:CURR[:LIM]:TIME?
        case 0x93: // :THER[:PROT]:TIME?
        case 0x94: // :THER[:PROT]:TIME?
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHFloatResponse(shf);}
            return;

        case 0x01: // :SOUR:VOLT:PROT:CLAM <b>
        case 0x02: // :SOUR:CURR[:LIM]:TYPE <name>
        case 0x05: // :SYST:POS <name>
        case 0x06: // *SAV <NRf>
        case 0x07: // *RCL <NRf>
        case 0x08: // :LOAD:INDE:STAT <b>
        case 0x09: // :LOAD:CURR[:LIM]:TYPE <name>
        case 0x0a: // :SOUR:CURR[:LIM]:PRIO <b>
        case 0x0b: // :SOUR:VOLT:PROT:ABS <b>
        case 0x11: // :SOUR:VOLT:PROT:TIME <NRf>
        case 0x12: // :SOUR:CURR[:LIM]:TIME <NRf>
        case 0x13: // :THER[:PROT]:TIME <NRf>
        case 0x14: // SCPI cmd Repeat
        case 0x80:case 0x00:case 0x83:case 0x03:case 0x84:case 0x04:case 0x8c:case 0x0c:case 0x90: // not SCPI cmd
        default:
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHVoidResponse();}
            return;
    }
}

void UartChannelManager::handleMeasurementcmd(quint8 func,quint16 scpiIdentify){ // new protocol Needs to be completed
    quint16 sh = m_readparam.size()==1 ? static_cast<quint16>(m_readparam[0]) : (m_readparam.size()==2) ? qFromBigEndian<quint16>(m_readparam.constData()) : 0;
    float  shf = m_readparam.size()==4 ? qFromBigEndian<float>(m_readparam.constData())  :  0.0f;
    qCDebug(uart_channel)<<"[handleMeasurementcmd]:Ch_"<<m_channel<<" ["<<func<<"] "<<sh<<","<<shf;

    switch (func){
        case 0x80: // :MEAS:VOLT[:DC]?
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHFloatResponse(shf);} // Otherwise, conflicts.
            emit CH_VoltageChanged(m_channel,shf);
            return;
        case 0x81: // :MEAS:CURR[:DC]?
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHFloatResponse(shf);} // Otherwise, conflicts.
            emit CH_CurrentAndUnitChanged(m_channel,shf);
            return;
        case 0xa3:{ // :SENS:SWE:OFFS:POIN?
            int sht = m_readparam.size()==4 ? qFromBigEndian<int>(m_readparam.constData())  :  0;
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHIntResponse(sht);}
            return;
        }
        case 0x82: // :MEAS:SCUR[:DC]?
        case 0x83: // :MEAS:BTMP?
        case 0x84: // :MEAS:HTMP?
        case 0x85: // :MEASure:DVMeter:ACDC?
        case 0x86: // :MEASure:DVMeter?
        case 0x89: // :MEASure:DVMeter:AC?
        case 0x8A: // :MEAS:TMP1?
        case 0x8B: // :MEAS:TMP2?
        case 0x8D: // :MEAS:TMP3?
        case 0xb0: // :MEASure:ADcOFfset:VOLTage?
        case 0xb1: // :MEASure:ADcOFfset:CURRent?
        case 0xb2: // :MEASure:ADcOFfset:SmallCURrent?
        case 0xb6: // :MEASure:ADcOFfset:DVMeter?
        case 0x9F: // :SENS:CURR:RANG:TIMe?
        case 0x91: // :FETC:CURR:HIGH?
        case 0x92: // :FETC:CURR:LOW?
        case 0x93: // :FETC:CURR:MAX?
        case 0x94: // :FETC:CURR:MIN?
        case 0x95: // :FETC:DVM:HIGH?
        case 0x96: // :FETC:DVM:LOW?
        case 0x97: // :FETC:DVM:MAX?
        case 0x98: // :FETC:DVM:MIN?
        case 0x99: // :FETC:VOLT:HIGH?
        case 0x9A: // :FETC:VOLT:LOW?
        case 0x9B: // :FETC:VOLT:MAX?
        case 0x9C: // :FETC:VOLT:MIN?
        case 0x9e: // :SENS:SWE:TINT?
        case 0xa0: // :MEAS:ARR:VOLT?
        case 0xa1: // :MEAS:ARR:CURR?
        case 0xa2: // :MEASure:ARR:DVMeter?
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHFloatResponse(shf);}
            return;
        case 0x87: // :THER[:PROT]:FAN?
        case 0x9D: // :THER[:PROT]:DUTY?
        //case 0x9d:  :SENS:SWE:POIN?
        case 0x8E: // :SENS:CURR[:DC]:RANG[:UPP]?
        case 0x90: // :SENS:FUNC?
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHIntResponse(sh);}
            return;
        case 0x8C: // :SENS:NPLC?
        case 0x8F: // :SENS:AVER?
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHVoidResponse();}
            return;


        case 0x0E: // :SENS:CURR[:DC]:RANG[:UPP] <NRf>
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHVoidResponse();}
            emit CH_RangeChanged(m_channel,sh);
            return;
        case 0x0C: // :SENS:NPLC <NRf>
        case 0x1F: // :SENS:CURR:RANG:TIMe n
        case 0x0F: // :SENS:AVER <NRf>
        case 0x10: // :SENS:FUNC "{CURR|DVM|VOLT}"
        case 0x23: // :SENS:SWE:OFFS:POIN <NRf>
        case 0x1d: // :SENS:SWE:POIN <NRf>
        case 0x1e: // :SENS:SWE:TINT <NRf>
        default:
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHVoidResponse();}
            return;
    }
}

void UartChannelManager::handleRegistercmd(quint8 func,quint16 scpiIdentify){
    QString str; QTextStream ss(&str);
    if (m_readparam.size() == 4){
        const quint8* b = reinterpret_cast<const quint8*>(m_readparam.constData());
        ss << b[0] << '.' << b[1] << '.' << b[2] << '.' << b[3];
    }

    quint16 sh = m_readparam.size()==1 ? static_cast<quint16>(m_readparam[0]) : (m_readparam.size()==2) ? qFromBigEndian<quint16>(m_readparam.constData()) : 0;
    qCDebug(uart_channel)<<"[handleRegistercmd]:Ch_"<<m_channel<<" ["<<func<<"] "<<sh<<","<<str;;

    switch (func){
        case 0x80: // :STAT:OPER[:EVEN]?
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHIntResponse(sh);}
            emit CH_StatusChanged(m_channel,sh);
            return;
        case 0x81: // :SYST:ENAB?
        case 0x82: // :STAT:OPER:COND?
        case 0x86: // :SYST:CONF?
        case 0x83: // :STAT:QUE[:NEXT]?
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHIntResponse(sh);}
            return;
        case 0x84: // :SYST:SWVersion?
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHStringResponse(str);}
            emit CH_svChanged(m_channel,str);
            isExist = true;
            return;
        case 0x85: // :SYST:HWVersion?
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHStringResponse(str);}
            emit CH_hvChanged(m_channel,str);
            isExist = true;
            return;
        case 0xff: // :SYST:REG?<addr>
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHStringResponse(str);}
            return;

        case 0x03: // :STAT:QUE:CLE
        case 0x00: case 0x02: // not SCPI cmd
        default:
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHVoidResponse();}
            return;
    }
}

void UartChannelManager::handleCalibratecmd(quint8 func,quint16 scpiIdentify){
    switch (func){
        case 0x84:{ // :CALI:STAR[:ALL]?
            quint8 sh = m_readparam.size() == 1 ? static_cast<quint8>(m_readparam[0]) : 0;
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHIntResponse(sh);}
            qCDebug(uart_channel)<<"[handleCalibratecmd]:Ch_"<<m_channel<<" | "<<sh;
            return;
        }

        case 0x00: // :CALI:EXIT
        case 0x01: // :CALI:INIT
        case 0x02: // :CALI:REST
        case 0x03: // :CALI:SAVE
        case 0x04: // :CALI:STAR[:ALL]
        case 0x05: // :CALI:STAR:ADC
        case 0x06: // :CALI:STAR:DAC
        case 0x07: // :CALI:STAR:ENABle
        case 0x08: // :CALI:STAR:IMPedance
        case 0x10: // :CALI:STAR:DCPositiveOffset
        case 0x11: // :CALI:STAR:DCNegativeOffset
        case 0x16: // :CALI:CLearTC
        case 0x17: // :CALI:SAveTC
        case 0x18: // :CALI:UPdateTC
        default:
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHVoidResponse();}
            return;
    }
}

void UartChannelManager::handleCalibrationcmd(quint8 func,quint16 scpiIdentify){
    if (m_readparam.size()==4 && m_scpiCommand == scpiIdentify){ // // Query and Set
        float shf = qFromBigEndian<float>(m_readparam.constData()); // step = func
        m_scpiManager->processCHFloatResponse(shf);

        emit to_CanServer(m_channel,0x07,m_readparam);
        qCDebug(uart_channel)<<"[handleCalibrationcmd]:Ch_"<<m_channel<<" step: "<<func<<" CD: "<<shf;
    }
}

void UartChannelManager::handleTriggercmd(quint8 func,quint16 scpiIdentify){ // new protocol Needs to be change
    quint16 sh = m_readparam.size()==1 ? static_cast<quint16>(m_readparam[0]) : (m_readparam.size()==2) ? qFromBigEndian<quint16>(m_readparam.constData()) : 0;
    float  shf = m_readparam.size()==4 ? qFromBigEndian<float>(m_readparam.constData())  :  0.0f;
    qCDebug(uart_channel)<<"[handleTriggercmd]:Ch_"<<m_channel<<" ["<<func<<"] "<<sh<<","<<shf;

    switch (func){
        case 0x85: // :TRIG:SEQ2:SOUR?
        case 0x86: // :TRIG:SEQ2:COUN:{CURR|DVM|VOLT}?
        case 0x89: // :TRIG:SEQ2:SLOP:{CURR|DVM|VOLT}?
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHIntResponse(sh);}
            return;
        case 0x87: // :TRIG:SEQ2:HYST:{CURR|DVM|VOLT}?
        case 0x88: // :TRIG:SEQ2:LEV:{CURR|DVM|VOLT}?
        case 0x8A: // :SOUR:VOLT:AMPL:TRIG?
        case 0x8B: // :SOUR:CURR:TRIG?
        case 0x8C: // :SOUR:RES:TRIG?
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHFloatResponse(shf);}
            return;

        case 0x00: // :ABORt[ch]
        case 0x01: // :INITiate[ch][:IMMediate]:SEQuence[<n>]
        case 0x02: // :INITiate:CONTinuous:SEQuence1
        case 0x03: // :TRIG:[SEQ1]:[IMM]
        case 0x04: // :TRIG:SEQ2:[IMM]
        case 0x05: // :TRIG:SEQ2:SOUR <source>
        case 0x06: // :TRIG:SEQ2:COUN:{CURR|DVM|VOLT} n
        case 0x07: // :TRIG:SEQ2:HYST:{CURR|DVM|VOLT} n
        case 0x08: // :TRIG:SEQ2:LEV:{CURR|DVM|VOLT} n
        case 0x09: // :TRIG:SEQ2:SLOP:{CURR|DVM|VOLT} Type
        case 0x0A: // :SOUR:VOLT:AMPL:TRIG <NRf>
        case 0x0B: // :SOUR:CURR:TRIG <NRf>
        case 0x0C: // :SOUR:RES:TRIG <NRf>
        default:
            if (m_scpiCommand == scpiIdentify){m_scpiManager->processCHVoidResponse();}
            return;
    }
}

// new protocol needs to be complete other section - battery

void UartChannelManager::handleISPcmd(quint8 func,quint16 scpiIdentify){
    Q_UNUSED(scpiIdentify);
    switch (func){
        case 0x80: case 0x00: case 0x01: case 0x02: // not SCPI cmd
        default:
            m_scpiManager->processCHVoidResponse();
            return;
    }
}

void UartChannelManager::handleSNcmd(quint8 func,quint16 scpiIdentify){
    Q_UNUSED(scpiIdentify);
    switch (func){
        case 0x80: case 0x00: case 0x81: case 0x01: case 0x82: case 0x02: case 0x83: case 0x03: case 0x84: case 0x04: case 0x05: // not SCPI cmd
        default:
            m_scpiManager->processCHVoidResponse();
            return;
    }
}

void UartChannelManager::handleIDcmd(quint8 func,quint16 scpiIdentify){
    Q_UNUSED(scpiIdentify);
    switch (func){
        case 0x81: case 0x82: case 0x83: case 0x84: // not SCPI cmd
        default:
            m_scpiManager->processCHVoidResponse();
            return;
    }
}

void UartChannelManager::handleErrorcmd(quint8 func){
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

    qCDebug(uart_channel)<<"[handleErrorcmd]:Ch_"<<m_channel<<" Error: "<<errstr;
    m_scpiManager->processCHStringResponse(errstr);
}

// new protocol needs to be complete other section - test
