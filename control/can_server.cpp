#include "auxiliary/global_static.h"
#include "auxiliary/signal_bus.h"
#include "can_server.h"
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <fcntl.h>
#include <QSocketNotifier>

constexpr std::array<quint32, 9> CanServerManager::m_validBaudRates =
{
    1000000,//0
    800000,//1
    500000,//2
    250000,//3
    125000,//4
    100000,//5
    50000,//6
    20000,//7
    10000,//8
};
const QHash<quint32, quint16> CanServerManager::m_canToUart =
{
    // output
    {0x036001, 0x0180},
    {0x106001, 0x01},
    {0x107006, 0x0108},
    {0x037006, 0x0188},
    {0x107007, 0x0109},
    {0x037007, 0x0189},
    // setting
    {0x036002, 0x0280},
    {0x016002, 0x0200},
    {0x036003, 0x0281},
    {0x106003, 0x0201},
    {0x036004, 0x0282},
    {0x106004, 0x0202},
    {0x036005, 0x0283},
    {0x106005, 0x0203},
    {0x036006, 0x0284},
    {0x106006, 0x0204},
    {0x036040, 0x0285},
    {0x106040, 0x0205},
    // control
    {0x036007, 0x0380},
    {0x106007, 0x0300},
    {0x036008, 0x0381},
    {0x106008, 0x0301},
    {0x036009, 0x0382},
    {0x106009, 0x0302},
    {0x03600A, 0x0383},
    {0x10600A, 0x0303},
    {0x03600B, 0x0384},
    {0x10600B, 0x0304},
    {0x03600C, 0x0385},
    {0x10600C, 0x0305},
    {0x036041, 0x0388},
    {0x106041, 0x0308},
    {0x036042, 0x0389},
    {0x106042, 0x0309},
    // measurement
    {0x03600D, 0x048F},
    {0x10600D, 0x040F},

    {0x03600E, 0x0480},
    {0x03600F, 0x0481},
    {0x036010, 0x0482},
    {0x036011, 0x0483},
    {0x036012, 0x0484},
    {0x036013, 0x0485},
    {0x036014, 0x0486},
    {0x036015, 0x0487},

    {0x036029, 0x048A},
    {0x03602A, 0x048B},
    {0x03602B, 0x048D},

    {0x03601E, 0x0489},

    {0x106030, 0x0410},
    {0x036031, 0x0491},
    {0x036032, 0x0492},
    {0x036033, 0x0493},
    {0x036034, 0x0494},
    {0x036035, 0x0495},
    {0x036036, 0x0496},
    {0x036037, 0x0497},
    {0x036038, 0x0498},
    {0x036039, 0x0499},
    {0x03603a, 0x049A},
    {0x03603b, 0x049B},
    {0x03603c, 0x049C},
    // register
    {0x036016, 0x0580},
    {0x036017, 0x0581},
    {0x036018, 0x0582},
    {0x036019, 0x0583},
    {0x10601A, 0x0503},
    {0x03601B, 0x0584},
    {0x03601C, 0x0585},
    // trigger
    {0x107000, 0x0800},
    {0x107001, 0x0801},
    {0x107002, 0x0802},
    {0x107003, 0x0803},
    {0x107004, 0x0804},
    {0x107005, 0x0805},
    {0x037005, 0x0885},
    {0x107011, 0x0806},
    {0x037011, 0x0886},
    {0x107021, 0x0807},
    {0x037021, 0x0887},
    {0x107031, 0x0808},
    {0x037031, 0x0888},
    {0x107041, 0x0809},
    {0x037041, 0x0889},
    {0x037050, 0x088A},
    {0x107050, 0x080A},
    {0x037051, 0x088B},
    {0x107051, 0x080B},
    {0x037052, 0x088C},
    {0x107052, 0x080C},
    // calibrate
    {0x106102, 0x0601},
    {0x106103, 0x0602},
    {0x106104, 0x0603},
    {0x036105, 0x0684},
    {0x106106, 0x0604},
    {0x106107, 0x0605},
    {0x106108, 0x0606},
    {0x1062, 0x07},
    // error
    {0xffffff,0xffff},
};
const QHash<quint16, quint32> CanServerManager::m_uartToCan = []
{
    QHash<quint16, quint32> reverse;
    for (auto it = m_canToUart.constBegin(); it != m_canToUart.constEnd(); ++it) {
        reverse.insert(it.value(), it.key());
    }
    return reverse;
}();

void CanServerManager::processFrame(const QByteArray &data)
{
    quint8 channel = static_cast<quint8>(data[0]);
    quint32 caninf = (static_cast<quint32>(data[1]) << 8) |(static_cast<quint32>(data[2]));
    if (caninf != 0x1062){
        caninf = (static_cast<quint32>(data[1]) << 16) |(static_cast<quint32>(data[2]) << 8)
                |(static_cast<quint32>(data[3]));
    }

    quint16 cmf = m_canToUart.value(caninf);
    quint8  cmd = (cmf >> 8) & 0xFF; // restrict 8 bit ,remain discard
    quint8 func = cmf & 0xFF;// restrict 8 bit ,remain discard

    // param extract
    QByteArray one  = data.mid(7);
    QByteArray two  = data.mid(6);
    QByteArray four = data.mid(4);

    switch (cmf){
    // output
        case 0x01:{
            quint8 func = static_cast<quint8>(data[7]);
            ChannelBus::instance().to_Channel(channel,func,func,"",true);
            return;
        }
        case 0x0108:
        case 0x0109:
            ChannelBus::instance().to_Channel(channel,cmd,func,one,true);
            return;
        case 0x0180:
        case 0x0188:
        case 0x0189:
            ChannelBus::instance().to_Channel(channel,cmd,func,"",true);
            return;
    // setting
        case 0x0200:
        case 0x0201:
        case 0x0202:
        case 0x0203:
        case 0x0204:
        case 0x0205:
            ChannelBus::instance().to_Channel(channel,cmd,func,four,true);
            return;
        case 0x0280:
        case 0x0281:
        case 0x0282:
        case 0x0283:
        case 0x0284:
        case 0x0285:
            ChannelBus::instance().to_Channel(channel,cmd,func,"",true);
            return;
    // control
        case 0x0300:
        case 0x0301:
        case 0x0302:
        case 0x0303:
        case 0x0304:
        case 0x0305:
        case 0x0308:
        case 0x0309:
            ChannelBus::instance().to_Channel(channel,cmd,func,one,true);
            return;
        case 0x0380:
        case 0x0381:
        case 0x0382:
        case 0x0383:
        case 0x0384:
        case 0x0385:
        case 0x0388:
        case 0x0389:
            ChannelBus::instance().to_Channel(channel,cmd,func,"",true);
            return;
    // measurement
        case 0x040F:
            ChannelBus::instance().to_Channel(channel,cmd,func,four,true);
            return;
        case 0x0410:
            ChannelBus::instance().to_Channel(channel,cmd,func,one,true);
            return;
        case 0x048F:
        case 0x0480:
        case 0x0481:
        case 0x0482:
        case 0x0483:
        case 0x0484:
        case 0x0485:
        case 0x0486:
        case 0x0487:
        case 0x048A:
        case 0x048B:
        case 0x048C:
        case 0x0489:
        case 0x0491:
        case 0x0492:
        case 0x0493:
        case 0x0494:
        case 0x0495:
        case 0x0496:
        case 0x0497:
        case 0x0498:
        case 0x0499:
        case 0x049A:
        case 0x049B:
        case 0x049C:
            ChannelBus::instance().to_Channel(channel,cmd,func,"",true);
            return;
    // register
        case 0x0580:
        case 0x0581:
        case 0x0582:
        case 0x0583:
        case 0x0503:
        case 0x0584:
        case 0x0585:
            ChannelBus::instance().to_Channel(channel,cmd,func,"",true);
            return;
    // trigger
        case 0x0801:
        case 0x0802:
        case 0x0805:
        case 0x0809:
            ChannelBus::instance().to_Channel(channel,cmd,func,one,true);
            return;
        case 0x0806:
            ChannelBus::instance().to_Channel(channel,cmd,func,two,true);
            return;
        case 0x0807:
        case 0x0808:
        case 0x080A:
        case 0x080B:
        case 0x080C:
            ChannelBus::instance().to_Channel(channel,cmd,func,four,true);
            return;
        case 0x0800:
        case 0x0803:
        case 0x0804:
        case 0x0885:
        case 0x0886:
        case 0x0887:
        case 0x0888:
        case 0x0889:
        case 0x088A:
        case 0x088B:
        case 0x088C:
            ChannelBus::instance().to_Channel(channel,cmd,func,"",true);
            return;
    // calibrate
        case 0x07:{
            m_calibrate_step = static_cast<quint8>(data[3]);
            ChannelBus::instance().to_Channel(channel,func,m_calibrate_step,four,true);
            return;
        };
        case 0x0601:
        case 0x0602:
        case 0x0603:
        case 0x0684:
        case 0x0604:
        case 0x0605:
        case 0x0606:
            ChannelBus::instance().to_Channel(channel,cmd,func,"",true);
            return;

        default:qCWarning(canSer)<<"[processFrame]error case";return;
    }
}
void CanServerManager::changebaud()
{
    if (OpeningCanN()) {
        ConfigManager::saveNewValue(ConfAddress::CanBaud,ConfigManager::s_canBaudIndex);
        emit ChannelBus::instance().interAcUpdate();
    }
}
void CanServerManager::sendFrame(quint8 ch,quint16 uart,const QByteArray &param)
{
    // Queue rate limiting (to prevent infinite growth)
    if (ConfigManager::control_Mode.load()==InterSt::CAN && m_sendQueue.size() <= 9999){
        quint32 cmf = m_uartToCan.value(uart); // default 0

        if (cmf != 0){
            if (cmf < 0x10000) {
                cmf = (cmf << 8) | static_cast<quint32>(m_calibrate_step);
            }else{
                cmf = (cmf & 0x00FFFFFF) | (static_cast<quint32>(ch) << 24);
            }

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

        qCWarning(canSer)<<"[sendFrame]:ERROR uart command!!!";
        return;
    }

    qCWarning(canSer)<<"[sendFrame]:Send queue overflow!!!";
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
                        qCDebug(canSer)<<"[CanServerManager]:Received Data: "<<data.toHex();

                        switch (ConfigManager::control_Mode.load()) {
                            case InterSt::LOCAL:emit ChannelBus::instance().switchRemote(InterSt::CAN);// fall through
                            case InterSt::CAN:processFrame(data); break;
                            default:
                                qCDebug(canSer)<<"[CanServerManager]: remoteMode[!=can-1]: "<<ConfigManager::control_Mode.load();
                                quint8 channel = static_cast<quint8>(data[0]);
                                sendFrame(channel,0xffff,""); break;
                        }
                    }
                }

                qCWarning(canSer)<<"[CanServerManager]:Read error: "<<strerror(errno);
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
                    qCDebug(canSer)<<"[CanServerManager]:Sent Data: "<<data.toHex()<<", QueueRemain: "<<m_sendQueue.size();
                    break;
                }

                qCWarning(canSer)<<"[CanServerManager]:Write error: "<<strerror(errno);
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

   delete m_readNotifier;
   delete m_writeNotifier;
}

bool CanServerManager::OpeningCanN()
{
    QStringList commands;
    commands << QString("ip link set %1 down").arg(m_interface);
    commands << QString("ip link set %1 txqueuelen 1000").arg(m_interface);
    //commands << QString("ip link set %1 type can bitrate 500000 restart-ms 18 dbitrate 2000000 fd on loopback on").arg(interface);
    commands << QString("ip link set %1 type can bitrate %2 restart-ms 18 fd off loopback on")
                .arg(m_interface).arg(m_validBaudRates[ConfigManager::s_canBaudIndex]);
    commands << QString("ip link set %1 up").arg(m_interface);

    for (const QString &cmd : qAsConst(commands)) {
        if (system(cmd.toUtf8().constData()) != 0) {
            qCCritical(canSer)<<"[OpeningCanN]:Command failed: "<<cmd;
            return false;
        }
    }

    return true;
}
bool CanServerManager::createSocket()
{
    m_socketFd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (m_socketFd < 0) {
        qCCritical(canSer)<<"[createSocket]:Failed to create CAN socket: "<<strerror(errno);
        return false;
    }

    struct ifreq ifr;
    strncpy(ifr.ifr_name, m_interface.toUtf8().constData(), IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    if (ioctl(m_socketFd, SIOCGIFINDEX, &ifr) < 0) {
        qCCritical(canSer)<<"[createSocket]:Failed to get interface index: "<<strerror(errno);
        close(m_socketFd);
        m_socketFd = -1;
        return false;
    }

    struct sockaddr_can addr{};
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(m_socketFd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        qCCritical(canSer)<<"[createSocket]:Failed to bind socket: "<<strerror(errno);
        close(m_socketFd);
        m_socketFd = -1;
        return false;
    }

    // Set non-blocking mode
    int flags = fcntl(m_socketFd, F_GETFL, 0);
    if (fcntl(m_socketFd, F_SETFL, flags | O_NONBLOCK) < 0) {
        qCCritical(canSer)<<"[createSocket]:Failed to set non-blocking mode: "<<strerror(errno);
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
