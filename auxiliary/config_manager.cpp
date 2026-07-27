#include "config_manager.h"
#include <QBigEndianStorageType>
#include <QNetworkInterface>
#include <QFile>

Q_LOGGING_CATEGORY(config, "CONFIG:")

std::vector<UartConfig> configs = {
    //{"/dev/ttyS3",    QSerialPort::Baud38400, 0x01}, // debug-Uart
    /*{"/dev/ttyS4",    QSerialPort::Baud38400, 0x01},
    {"/dev/ttyS5",    QSerialPort::Baud38400, 0x02},
    {"/dev/ttyS7",    QSerialPort::Baud38400, 0x03},
    {"/dev/ttyS8",    QSerialPort::Baud38400, 0x04},
    {"/dev/ttyS9",    QSerialPort::Baud38400, 0x05},
    {"/dev/ttyWCH0",  QSerialPort::Baud38400, 0x06},
    {"/dev/ttyWCH1",  QSerialPort::Baud38400, 0x07},
    {"/dev/ttyWCH2",  QSerialPort::Baud38400, 0x08},
    {"/dev/ttyWCH3",  QSerialPort::Baud38400, 0x09},
    {"/dev/ttyWCH4",  QSerialPort::Baud38400, 0x0a},
    {"/dev/ttyWCH5",  QSerialPort::Baud38400, 0x0b},
    {"/dev/ttyWCH6",  QSerialPort::Baud38400, 0x0c},
    {"/dev/ttyWCH7",  QSerialPort::Baud38400, 0x0d},
    {"/dev/ttyWCH8",  QSerialPort::Baud38400, 0x0e},
    {"/dev/ttyWCH9",  QSerialPort::Baud38400, 0x0f},
    {"/dev/ttyWCH10", QSerialPort::Baud38400, 0x10},
    {"/dev/ttyWCH11", QSerialPort::Baud38400, 0x11},
    {"/dev/ttyWCH12", QSerialPort::Baud38400, 0x12},
    {"/dev/ttyWCH13", QSerialPort::Baud38400, 0x13},
    {"/dev/ttyWCH14", QSerialPort::Baud38400, 0x14},
    {"/dev/ttyWCH15", QSerialPort::Baud38400, 0x15},
    {"/dev/ttyWCH16", QSerialPort::Baud38400, 0x16},
    {"/dev/ttyWCH17", QSerialPort::Baud38400, 0x17},
    {"/dev/ttyWCH18", QSerialPort::Baud38400, 0x18},
    {"/dev/ttyWCH19", QSerialPort::Baud38400, 0x19},
    {"/dev/ttyWCH20", QSerialPort::Baud38400, 0x1a},
    {"/dev/ttyWCH21", QSerialPort::Baud38400, 0x1b},
    {"/dev/ttyWCH22", QSerialPort::Baud38400, 0x1c},
    {"/dev/ttyWCH23", QSerialPort::Baud38400, 0x1d},
    {"/dev/ttyWCH24", QSerialPort::Baud38400, 0x1e},
    {"/dev/ttyWCH25", QSerialPort::Baud38400, 0x1f},
    {"/dev/ttyWCH26", QSerialPort::Baud38400, 0x20},
    {"/dev/ttyWCH27", QSerialPort::Baud38400, 0x21},   // 33*/
    {"/dev/ttyS1",    QSerialPort::Baud38400, 0x01},   // test
};

// log config
QString ConfigManager::s_loglevel = "release";
bool ConfigManager::s_enablelogfile = false;
// channel switch
bool ConfigManager::s_enableUartMess   = true;
bool ConfigManager::s_enableCanMess    = true;
// control switch
bool ConfigManager::s_enableLANServer  = true;
bool ConfigManager::s_enableWEBServer  = false;
bool ConfigManager::s_enableCANServer  = true;
bool ConfigManager::s_enableUARTServer = true;
bool ConfigManager::s_enableDisplay    = true;

std::atomic<int> ConfigManager::s_remoteSt{0};
QSettings* ConfigManager::s_settings = nullptr;

// global variable - Internal fixation
QString ConfigManager::s_firmwareVersion = "1.0.0";
QString ConfigManager::s_hardwareVersion = "1.0.0";

QString ConfigManager::s_manufacturer = "Leacesy";
QString ConfigManager::s_serialNumber = "SN-66004";
QString ConfigManager::s_model = "66004";

// global variable - System reading
QString ConfigManager::s_IP = "";
QString ConfigManager::s_SM = "";
QString ConfigManager::s_MAC = "";
QString ConfigManager::s_Gateway = "";
bool ConfigManager::s_isDHCP = false;

quint8 ConfigManager::s_GPIBid = 0;
quint8 ConfigManager::s_CANid = 0;

bool ConfigManager::init(const QString &configDir)
{
    if (getNetworkConfig()){
        QString fullPath = configDir + "/instrument_config.ini";

        if (QFile::exists(fullPath) && !s_settings) {
            s_settings = new QSettings(fullPath, QSettings::IniFormat);

            s_loglevel = s_settings->value("Logger/logLevel").toString();
            s_enablelogfile = s_settings->value("Logger/EnablelogFile").toBool();

            s_enableUartMess = s_settings->value("Channel/EnableUartMess").toBool();
            s_enableCanMess = s_settings->value("Channel/EnableCanMess").toBool();

            s_enableLANServer = s_settings->value("Control/EnableLANServer").toBool();
            s_enableWEBServer = s_settings->value("Control/EnableWEBServer").toBool();
            s_enableCANServer = s_settings->value("Control/EnableCANServer").toBool();
            s_enableUARTServer = s_settings->value("Control/EnableUARTServer").toBool();
            s_enableDisplay = s_settings->value("Control/EnableDisplay").toBool();

            s_serialNumber = s_settings->value("Device/SerialNumber").toString();
            s_model = s_settings->value("Device/Model").toString();

            s_GPIBid = s_settings->value("Device/GPIBID").toUInt();
            s_CANid = s_settings->value("Device/CANID").toUInt();

            return true;
        }

        qCWarning(config) << "[init]:Cannot open config file for writing";
    }

    // getNetworkConfig have print information
    return false;
}

bool ConfigManager::getNetworkConfig() {
    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface& iface : qAsConst(interfaces)) {
        if (iface.name() == "eth0") {
            QFile file("/etc/network/interfaces");
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                int readcount = 0;
                QTextStream in(&file);
                QString line = in.readLine(); // get frist rows of headline

                while (!in.atEnd()) {
                    line = in.readLine().trimmed(); // Remove first and end spaces

                    if (line.startsWith("iface " + iface.name())) {
                        readcount += 1;
                        if (line.contains("dhcp", Qt::CaseInsensitive)) {
                            s_isDHCP = true;
                        }else{
                            s_isDHCP = false;
                        }
                    }

                    if (line.startsWith("gateway ")) {
                        readcount += 1;
                        s_Gateway = line.mid(8).trimmed();
                    }

                    if (readcount >= 2){break;}
                }

                s_MAC = iface.hardwareAddress();
                const auto entries = iface.addressEntries();
                for (const QNetworkAddressEntry& entry : qAsConst(entries)) {
                    if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                        s_IP = entry.ip().toString();
                        s_SM = entry.netmask().toString();
                        return true;
                    }
                }
            }

            qCWarning(config) << "[getNetworkConfig]:/etc/network/interfaces opening failed!";
            return false; // static
        }
    }

    qCWarning(config) << "[getNetworkConfig]:not find target iface!";
    return false;
}

bool ConfigManager::setinterfaces(bool isstatic,const QString& ip, const QString& netmask,const QString& gateway){
    QFile file("/etc/network/interfaces");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QString configContent;
        if(isstatic){
            configContent =
                "# interface file auto-generated by buildroot\n"
                "auto lo\n"
                "iface lo inet loopback\n\n"
                "auto eth0\n"
                "iface eth0 inet static\n"
                "    address " + ip + "\n"
                "    netmask " + netmask + "\n"
                "    gateway " + gateway + "\n\n"
                "auto eth1\n"
                "iface eth1 inet static\n"
                "    address 192.168.2.136\n"
                "    netmask 255.255.255.0\n"
                "    gateway 192.168.2.1\n";
        }else{
            configContent =
                "# interface file auto-generated by buildroot\n"
                "auto lo\n"
                "iface lo inet loopback\n\n"
                "auto eth0\n"
                "iface eth0 inet dhcp\n\n"
                "auto eth1\n"
                "iface eth1 inet static\n"
                "    address 192.168.2.136\n"
                "    netmask 255.255.255.0\n"
                "    gateway 192.168.2.1\n";
        }

        // WriteOnly -> Clear the original content
        QTextStream out(&file);
        out << configContent;
        file.close();

        QString restartCmd = "/etc/init.d/S40network restart";
        if (system(restartCmd.toStdString().c_str()) == 0) {
            getNetworkConfig();
            return true;
        }

        qCWarning(config) << "[refresh_interfaces]:/etc/init.d/S40network restart failed!";
        return false;
    }

    qCWarning(config) << "[refresh_interfaces]:Cannot open interfaces file for writing";
    return false;
}

bool ConfigManager::setConfigValue(const QString &key, const QVariant &value)
{
    if (s_settings) {
        // Automatically written at the end
        s_settings->setValue(key, value);
        // s_settings->sync();
        return true;
    }

    qCWarning(config) << "[setConfigValue]:not setting exist!";
    return false;
}
