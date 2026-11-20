#include "auxiliary/global_static.h"
#include "auxiliary/signal_bus.h"
#include "channel/uart_protocol.h"
#include "can_server.h"
// --- QT package ---
#include <QSocketNotifier>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <fcntl.h>

void CanServerManager::changebaud()
{
    if (OpeningCanN()) {
        ConfigManager::saveNewValue(ConfAddress::CanBaud,ConfigManager::s_canBaudIndex);
        emit ChannelBus::instance().interAcUpdate(InterSt::CAN);
    }
}

void CanServerManager::processFrame(const QByteArray &data)
{
    if (data.size() == 8){
        quint32 caninf = (static_cast<quint32>(data[1]) << 8) |(static_cast<quint32>(data[2]));
        if (caninf != 0x1062){
            caninf = (static_cast<quint32>(data[1]) << 16) |(static_cast<quint32>(data[2]) << 8)
                    |(static_cast<quint32>(data[3]));
        }

        quint16 cmds = m_canToUart.value(caninf); // default 0
        quint8 ch = static_cast<quint8>(data[0]);

        switch (cmds){
            // ---------- output -------------
            case JV6xxx::UartCMD::OUTP_OFF:
            {
                quint16 opcmds = JV6xxx::UartCMD::OUTP_OFF | static_cast<quint8>(data[7]); // 8 byte last byte
                qCDebug(canSer)<<"(processFrame)void-param, outpcmd: "<<opcmds;
                ChannelBus::instance().to_Channel(ch,opcmds,"",true);
                return;
            }
            case QCmd(JV6xxx::UartCMD::OUTP_OFF):
            case QCmd(JV6xxx::UartCMD::OUTPBAND):
            case QCmd(JV6xxx::UartCMD::OUTPCOMP):
            // ---------- setting -------------
            case QCmd(JV6xxx::UartCMD::VOLT):
            case QCmd(JV6xxx::UartCMD::CURR):
            case QCmd(JV6xxx::UartCMD::IMP):
            case QCmd(JV6xxx::UartCMD::OVP):
            case QCmd(JV6xxx::UartCMD::OTP):
            case QCmd(JV6xxx::UartCMD::SetLoadCurr):
            // ---------- control -------------
            case QCmd(JV6xxx::UartCMD::EnLOAD):
            case QCmd(JV6xxx::UartCMD::EnOVP):
            case QCmd(JV6xxx::UartCMD::EnOCP):
            case QCmd(JV6xxx::UartCMD::EnOTP):
            case QCmd(JV6xxx::UartCMD::EnIMPE):
            case QCmd(JV6xxx::UartCMD::SetPON):
            case QCmd(JV6xxx::UartCMD::LoadINDE):
            case QCmd(JV6xxx::UartCMD::LoadOCP):
            // ---------- Measure -------------
            case QCmd(JV6xxx::UartCMD::AverCount):
            case QCmd(JV6xxx::UartCMD::RVoltage):
            case QCmd(JV6xxx::UartCMD::RCurrent):
            case QCmd(JV6xxx::UartCMD::RSCurrent):
            case QCmd(JV6xxx::UartCMD::RBT):
            case QCmd(JV6xxx::UartCMD::RHT):
            case QCmd(JV6xxx::UartCMD::RDVMACDC):
            case QCmd(JV6xxx::UartCMD::RDVMDC):
            case QCmd(JV6xxx::UartCMD::RFSpeed):
            case QCmd(JV6xxx::UartCMD::TMP1):
            case QCmd(JV6xxx::UartCMD::TMP2):
            case QCmd(JV6xxx::UartCMD::TMP3):
            case QCmd(JV6xxx::UartCMD::RDVMAC):
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
            // ---------- Register -------------
            case QCmd(JV6xxx::UartCMD::StaREG):
            case QCmd(JV6xxx::UartCMD::EnREG):
            case QCmd(JV6xxx::UartCMD::CondREG):
            case QCmd(JV6xxx::UartCMD::ERR):
            case JV6xxx::UartCMD::ERR:
            case QCmd(JV6xxx::UartCMD::SV):
            case QCmd(JV6xxx::UartCMD::HV):
            // ---------- Trigger -------------
            case JV6xxx::UartCMD::ABOR:
            case JV6xxx::UartCMD::SEQ1TRIG:
            case JV6xxx::UartCMD::SEQ2TRIG:
            case QCmd(JV6xxx::UartCMD::ACQTRIGSOUR):
            case QCmd(JV6xxx::UartCMD::COUN_COMM):
            case QCmd(JV6xxx::UartCMD::HYST_COMM):
            case QCmd(JV6xxx::UartCMD::LEV_COMM):
            case QCmd(JV6xxx::UartCMD::SLOP_COMM):
            case QCmd(JV6xxx::UartCMD::SOUR_VOLT_TRIG):
            case QCmd(JV6xxx::UartCMD::SOUR_CURR_TRIG):
            case QCmd(JV6xxx::UartCMD::SOUR_RES_TRIG):
            // ---------- calibrate -------------
            case JV6xxx::UartCMD::CalInit:
            case JV6xxx::UartCMD::CalRestore:
            case JV6xxx::UartCMD::CalSave:
            case QCmd(JV6xxx::UartCMD::CalStarALL):
            case JV6xxx::UartCMD::CalStarALL:
            case JV6xxx::UartCMD::CalStarADC:
            case JV6xxx::UartCMD::CalStarDAC:
            {
                qCDebug(canSer)<<"(processFrame)void-param";
                ChannelBus::instance().to_Channel(ch,cmds,"",true);
                return;
            }
            // ---------- output -------------
            case JV6xxx::UartCMD::OUTPBAND:
            case JV6xxx::UartCMD::OUTPCOMP:
            // ---------- control -------------
            case JV6xxx::UartCMD::EnLOAD:
            case JV6xxx::UartCMD::EnOVP:
            case JV6xxx::UartCMD::EnOCP:
            case JV6xxx::UartCMD::EnOTP:
            case JV6xxx::UartCMD::EnIMPE:
            case JV6xxx::UartCMD::SetPON:
            case JV6xxx::UartCMD::LoadINDE:
            case JV6xxx::UartCMD::LoadOCP:
            // ---------- Measure -------------
            case JV6xxx::UartCMD::SetFUNC:
            // ---------- Trigger -------------
            case JV6xxx::UartCMD::INIT:
            case JV6xxx::UartCMD::CONT:
            case JV6xxx::UartCMD::ACQTRIGSOUR:
            case JV6xxx::UartCMD::SLOP_COMM:
            {
                QByteArray one  = data.mid(7);
                qCDebug(canSer)<<"(processFrame)one-param: "<<one;
                ChannelBus::instance().to_Channel(ch,cmds,one,true);
                return;
            }
            case JV6xxx::UartCMD::COUN_COMM:
            {
                QByteArray two  = data.mid(6);
                qCDebug(canSer)<<"(processFrame)two-param: "<<two;
                ChannelBus::instance().to_Channel(ch,cmds,two,true);
                return;
            }
            // ---------- setting -------------
            case JV6xxx::UartCMD::VOLT:
            case JV6xxx::UartCMD::CURR:
            case JV6xxx::UartCMD::IMP:
            case JV6xxx::UartCMD::OVP:
            case JV6xxx::UartCMD::OTP:
            case JV6xxx::UartCMD::SetLoadCurr:
            // ---------- Measure -------------
            case JV6xxx::UartCMD::AverCount:
            // ---------- Trigger -------------
            case JV6xxx::UartCMD::HYST_COMM:
            case JV6xxx::UartCMD::LEV_COMM:
            case JV6xxx::UartCMD::SOUR_VOLT_TRIG:
            case JV6xxx::UartCMD::SOUR_CURR_TRIG:
            case JV6xxx::UartCMD::SOUR_RES_TRIG:
            {
                QByteArray four = data.mid(4);
                qCDebug(canSer)<<"(processFrame)four-param: "<<four;
                ChannelBus::instance().to_Channel(ch,cmds,four,true);
                return;
            }
            // ---------- calibrate -------------
            case JV6xxx::UartCMD::CalStepOffset:
            {
                quint8 step = static_cast<quint8>(data[3]);
                if (step <= JV6xxx::UartCMD::CalStepItemMax){
                    QByteArray four = data.mid(4);
                    qCDebug(canSer)<<"(processFrame)four-param: "<<four<<",step: "<<step;
                    quint16 calcmds = JV6xxx::UartCMD::CalStepOffset | step; // calibrate_step
                    ChannelBus::instance().to_Channel(ch,calcmds,four,true);
                }
                return;
            };
            default:
            {
                qCWarning(canSer)<<"(processFrame)error case: "<<cmds;
                return;
            }
        }
    }
}
void CanServerManager::sendFrame(quint8 ch,quint16 uart,const QByteArray &param)
{
    // Queue rate limiting (to prevent infinite growth)
    if (ConfigManager::control_Mode.load()==InterSt::CAN && m_sendQueue.size() <= 9999){
        quint32 cmf = 0;
        quint16 caluart = uart & 0xFF00;

        if (caluart == JV6xxx::UartCMD::CalStepOffset){
            quint8 step = uart & 0xFF;
            cmf = (static_cast<quint32>(m_uartToCan.value(caluart)) << 8) | step;
        }else{
            cmf = m_uartToCan.value(uart); // default 0
            cmf = (cmf & 0x00FFFFFF) | (static_cast<quint32>(ch) << 24);
        }

        if (cmf != 0){
            QByteArray data;
            data.append(static_cast<char>((cmf >> 24) & 0xFF));
            data.append(static_cast<char>((cmf >> 16) & 0xFF));
            data.append(static_cast<char>((cmf >> 8) & 0xFF));
            data.append(static_cast<char>(cmf & 0xFF));

            QByteArray paddedParam(4, 0);
            int copyLen = qMin(param.size(), 4);
            memcpy(paddedParam.data() + (4 - copyLen), param.data(), copyLen);
            data.append(paddedParam);

            struct can_frame frame;
            frame.can_id = ConfigManager::s_CANid.load();
            frame.can_dlc = static_cast<quint8>(data.size());
            memcpy(frame.data, data.constData(), data.size());

            if (m_sendQueue.isEmpty()) {m_writeNotifier->setEnabled(true);}
            m_sendQueue.enqueue(frame);
            return;
        }

        qCWarning(canSer)<<"(sendFrame)not Support uart command!!!";
        return;
    }

    qCWarning(canSer)<<"(sendFrame)Send queue overflow!!!";
    return;
}

CanServerManager::CanServerManager(QObject *parent): QObject(parent)
{
    if (OpeningCanN() && createSocket()) {
        connect(m_readNotifier, &QSocketNotifier::activated,this, [this](){
            m_readNotifier->setEnabled(false);
            struct can_frame frame;

            while (true) {
                ssize_t nbytes = read(m_socketFd, &frame, sizeof(frame));

                if (nbytes == sizeof(frame)) { // always 16Bytes
                    if(frame.can_id == 0 || frame.can_id == ConfigManager::s_CANid.load()){
                        QByteArray data(reinterpret_cast<const char*>(frame.data), frame.can_dlc);
                        qCDebug(canSer)<<"(CanServerManager)Received Data: "<<data.toHex(' ');

                        if (ConfigManager::isInteRemote(InterSt::CAN)){
                            processFrame(data);
                        }else{
                            quint8 channel = static_cast<quint8>(data[0]);
                            sendFrame(channel,0xffff,"");
                        }
                    }

                    continue;
                }else if(nbytes < 0 && errno != EAGAIN && errno != EWOULDBLOCK){
                    qCWarning(canSer)<<"(CanServerManager)Read error: "<<strerror(errno);
                }

                break;
            }

            m_readNotifier->setEnabled(true);
        }, Qt::DirectConnection);
        connect(m_writeNotifier, &QSocketNotifier::activated,this, [this](){
            m_writeNotifier->setEnabled(false);

            while (!m_sendQueue.isEmpty()) {
                const struct can_frame &frame = m_sendQueue.head();
                ssize_t sent = write(m_socketFd, &frame, sizeof(frame));

                if (sent == sizeof(frame)) {
                    m_sendQueue.dequeue();
                    QByteArray data(reinterpret_cast<const char*>(frame.data), frame.can_dlc);
                    qCDebug(canSer)<<"(CanServerManager)Sent Data: "<<data.toHex()<<", QueueRemain: "<<m_sendQueue.size();
                    continue;
                }else if(sent < 0 && errno != EAGAIN && errno != EWOULDBLOCK){
                    qCWarning(canSer)<<"(CanServerManager)Write error: "<<strerror(errno);
                }

                break;
            }
            m_writeNotifier->setEnabled(true);
        }, Qt::DirectConnection);

        return;
    }
}
CanServerManager::~CanServerManager()
{
   shutdown(m_socketFd, SHUT_RDWR);
   close(m_socketFd);
}

bool CanServerManager::OpeningCanN()
{
    QStringList commands;
    commands << QString("ip link set %1 down").arg(m_interface);
    commands << QString("ip link set %1 txqueuelen 1000").arg(m_interface);
    commands << QString("ip link set %1 type can bitrate %2 restart-ms 18 fd off loopback off")
                .arg(m_interface).arg(ChannelBus::m_CanBaudRates[ConfigManager::s_canBaudIndex]);
    commands << QString("ip link set %1 up").arg(m_interface);

    for (const QString &cmd : qAsConst(commands)) {
        if (system(cmd.toUtf8().constData()) != 0) {
            qCCritical(canSer)<<"(OpeningCanN)Command failed: "<<cmd;
            return false;
        }
    }

    return true;
}
bool CanServerManager::createSocket()
{
    m_socketFd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (m_socketFd < 0) {
        qCCritical(canSer)<<"(createSocket)Failed to create CAN socket: "<<strerror(errno);
        return false;
    }

    struct ifreq ifr;
    strncpy(ifr.ifr_name, m_interface.toUtf8().constData(), IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    if (ioctl(m_socketFd, SIOCGIFINDEX, &ifr) < 0) {
        qCCritical(canSer)<<"(createSocket)Failed to get interface index: "<<strerror(errno);
        close(m_socketFd);
        m_socketFd = -1;
        return false;
    }

    struct sockaddr_can addr{};
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(m_socketFd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        qCCritical(canSer)<<"(createSocket)Failed to bind socket: "<<strerror(errno);
        close(m_socketFd);
        m_socketFd = -1;
        return false;
    }

    // Set non-blocking mode
    int flags = fcntl(m_socketFd, F_GETFL, 0);
    if (fcntl(m_socketFd, F_SETFL, flags | O_NONBLOCK) < 0) {
        qCCritical(canSer)<<"(createSocket)Failed to set non-blocking mode: "<<strerror(errno);
        close(m_socketFd);
        m_socketFd = -1;
        return false;
    }

    int sndbuf = 1024 * 1024;  // send buffer -> 1MB = 62500frame
    setsockopt(m_socketFd, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf));

    int rcvbuf = 1024 * 1024;  // receive buffer -> 1MB = 62500frame
    setsockopt(m_socketFd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));
    // The event loop's operation of handling other events also leads to timeout receive buffer overflow.

    int recv_own_msgs = 0;
    setsockopt(m_socketFd, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS,&recv_own_msgs, sizeof(recv_own_msgs));
    // {0|1} Return directly to the receiving buffer---Does not affect physical transmission

    m_readNotifier = new QSocketNotifier(m_socketFd, QSocketNotifier::Read, this);
    m_readNotifier->setEnabled(true);

    m_writeNotifier = new QSocketNotifier(m_socketFd, QSocketNotifier::Write, this);
    m_writeNotifier->setEnabled(false);
    // Initially disabled, enabled only when there is data.

    return true;
}
