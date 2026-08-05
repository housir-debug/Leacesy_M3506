#include <QDir>
#include <QThread>
#include <QWidget>
#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include "auxiliary/simple_logger.h"
#include "channel/uart_channel.h"
#include "control/vxi_server.h"
#include "control/web_server.h"
#include "control/can_server.h"
#include "control/uart_server.h"
#include "widgets/mainwindow.h"
#include "widgets/test.h"

Q_LOGGING_CATEGORY(application, "APP")

using CanSign_toUartCh = void (CanServerManager::*)(quint8 cmd, quint8 func, const QByteArray& param,bool isScpi);
std::vector<CanSign_toUartCh> can_signal = {
    #define CHANNEL(n) static_cast<CanSign_toUartCh>(&CanServerManager::to_UartChannel##n),
    CHANNEL_COUNT
    #undef CHANNEL
};

using QWidgetSign_toUartCh = void (Mainwindow::*)(quint8 cmd, quint8 func, const QByteArray& param,bool isScpi);
std::vector<QWidgetSign_toUartCh> qml_signal = {
    #define CHANNEL(n) static_cast<QWidgetSign_toUartCh>(&Mainwindow::to_UartChannel##n),
    CHANNEL_COUNT
    #undef CHANNEL
};

using ScpiSign_toUartCh = void (ScpiManager::*)(quint8 cmd, quint8 func, const QByteArray& param,bool isScpi);
std::vector<ScpiSign_toUartCh> scpi_signal = {
    #define CHANNEL(n) static_cast<ScpiSign_toUartCh>(&ScpiManager::to_UartChannel##n),
    CHANNEL_COUNT
    #undef CHANNEL
};

using ScpiRSTSign_toUartCh = void (ScpiManager::*)();
std::vector<ScpiRSTSign_toUartCh> scpi_rst_signal = {
    #define CHANNEL(n) static_cast<ScpiRSTSign_toUartCh>(&ScpiManager::to_UartChannel##n##Reset),
    CHANNEL_COUNT
    #undef CHANNEL
};


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("Leacesy_Ryan");
    QApplication::setOverrideCursor(Qt::BlankCursor);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    // QApplication::setAttribute(Qt::AA_EnableHighDpiScaling); -* Warning: It will deform.

    // load config
    const QString configPath = QDir(QApplication::applicationDirPath()).filePath("..");
    if (!QDir(configPath).exists() || !ConfigManager::init(configPath)) {
        qCCritical(application) << "Application initialization failed!";
        return 1;
    }

    // setting log config and batterymodel create
    loggermanage(ConfigManager::s_loglevel, configPath);
    QObject::connect(&app, &QApplication::aboutToQuit, &shutdownLogger);

    // CAN Server create
    std::unique_ptr<CanServerManager> canServer;
    if (ConfigManager::s_enableCANServer){canServer = std::make_unique<CanServerManager>();}

    // UART Server create and SCPI parser
    std::unique_ptr<UartServerManager> uartServer;
    std::shared_ptr<ScpiManager> Scpi_share = std::make_shared<ScpiManager>();
    if (ConfigManager::s_enableUARTServer){uartServer = std::make_unique<UartServerManager>();uartServer->m_scpiManager = Scpi_share;}

    // GPIB server create
    /*Not necessary for the time being.*/

    // LAN Server create
    std::unique_ptr<VxiServerManager> vxiServer;
    if (ConfigManager::s_enableLANServer){vxiServer = std::make_unique<VxiServerManager>();vxiServer->m_scpiManager = Scpi_share;}

    // screen show
    QGraphicsScene scene;
    QGraphicsView view(&scene);
    std::unique_ptr<test> testview;
    std::shared_ptr<Mainwindow> show_share;
    std::unique_ptr<WebServerManager> webServer;
    std::shared_ptr<BatteryModelManager> BatteryModel_share;
    if (ConfigManager::s_enableDisplay){
        BatteryModel_share = std::make_shared<BatteryModelManager>(configPath);

        show_share = std::make_shared<Mainwindow>();
        show_share->m_modelManager = BatteryModel_share;
        show_share->load_BatteryModel();

        webServer = std::make_unique<WebServerManager>();
        webServer->m_BatteryManager = BatteryModel_share;
        webServer->m_scpiManager = Scpi_share;
        webServer->m_qmlbridge = show_share;

        QObject::connect(show_share.get(),&Mainwindow::set_network,webServer.get(),&WebServerManager::set_network,Qt::QueuedConnection);
        QObject::connect(webServer.get(),&WebServerManager::isRemote,show_share.get(),&Mainwindow::update_remotemodel,Qt::QueuedConnection);
        QObject::connect(webServer.get(),&WebServerManager::networkrefresh,show_share.get(),&Mainwindow::update_setting,Qt::QueuedConnection);

        if (ConfigManager::s_enableCANServer){
            QObject::connect(canServer.get(),&CanServerManager::isRemote,show_share.get(),&Mainwindow::update_remotemodel,Qt::QueuedConnection);
            QObject::connect(show_share.get(),&Mainwindow::set_canbaud,canServer.get(),&CanServerManager::changebaudandid,Qt::QueuedConnection);
            QObject::connect(canServer.get(),&CanServerManager::baudrefresh,show_share.get(),&Mainwindow::update_setting,Qt::QueuedConnection);
        }

        if (ConfigManager::s_enableUARTServer){
            QObject::connect(uartServer.get(),&UartServerManager::isRemote,show_share.get(),&Mainwindow::update_remotemodel,Qt::QueuedConnection);
            QObject::connect(show_share.get(),&Mainwindow::set_RS232Baud,uartServer.get(),&UartServerManager::changeBaudRate,Qt::QueuedConnection);
            QObject::connect(uartServer.get(),&UartServerManager::baudrefresh,show_share.get(),&Mainwindow::update_setting,Qt::QueuedConnection);
        }

        if (ConfigManager::s_enableLANServer){
            QObject::connect(vxiServer.get(),&VxiServerManager::isRemote,show_share.get(),&Mainwindow::update_remotemodel,Qt::QueuedConnection);
        }

        QGraphicsProxyWidget* proxy = scene.addWidget(show_share.get());
        proxy->setFocusPolicy(Qt::NoFocus);
        proxy->setRotation(0);

        view.viewport()->setAttribute(Qt::WA_AcceptTouchEvents, false);
        view.setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
        view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view.setDragMode(QGraphicsView::NoDrag);
        view.setFrameShape(QFrame::NoFrame);
        //view.rotate(0);
        view.show();

        //show_share->show();

        //testview = std::make_unique<test>();
        //testview->show();
    }

    // can channel create
    /*Not necessary for the time being.*/

    // uart channel create
    std::vector<std::unique_ptr<UartChannelManager>> Channel_list;
    if (ConfigManager::s_enableUartMess){
        // config form config_manager
        for (const auto& config : configs) {
            auto channel = std::make_unique<UartChannelManager>();
            channel->initUartchannel(config.channel, config.port, config.baudRate);
            QObject::connect(Scpi_share.get(),scpi_signal[config.channel-1],channel.get(),&UartChannelManager::writeFrame,Qt::QueuedConnection);
            QObject::connect(Scpi_share.get(),scpi_rst_signal[config.channel-1],channel.get(),&UartChannelManager::sendInitCommand,Qt::QueuedConnection);

            if (ConfigManager::s_enableCANServer){
                QObject::connect(canServer.get(),can_signal[config.channel-1],channel.get(),&UartChannelManager::writeFrame,Qt::QueuedConnection);
                QObject::connect(channel.get(),&UartChannelManager::to_CanServer,canServer.get(),&CanServerManager::sendFrame,Qt::QueuedConnection);
            }

            if (ConfigManager::s_enableDisplay){
                QObject::connect(channel.get(),&UartChannelManager::CH_svChanged,show_share.get(),&Mainwindow::update_SoftVer,Qt::QueuedConnection);
                QObject::connect(channel.get(),&UartChannelManager::CH_hvChanged,show_share.get(),&Mainwindow::update_HardVer,Qt::QueuedConnection);
                QObject::connect(channel.get(),&UartChannelManager::CH_VoltageChanged,show_share.get(),&Mainwindow::update_Voltage,Qt::QueuedConnection);
                QObject::connect(channel.get(),&UartChannelManager::CH_CurrentAndUnitChanged,show_share.get(),&Mainwindow::update_CurrentAndUnit,Qt::QueuedConnection);
                QObject::connect(channel.get(),&UartChannelManager::CH_RangeChanged,show_share.get(),&Mainwindow::update_Range,Qt::QueuedConnection);
                QObject::connect(channel.get(),&UartChannelManager::CH_StatusChanged,show_share.get(),&Mainwindow::update_Status,Qt::QueuedConnection);
                QObject::connect(channel.get(),&UartChannelManager::CH_cvChanged,show_share.get(),&Mainwindow::update_Cv,Qt::QueuedConnection);
                QObject::connect(channel.get(),&UartChannelManager::CH_ccChanged,show_share.get(),&Mainwindow::update_Cc,Qt::QueuedConnection);
                QObject::connect(channel.get(),&UartChannelManager::CH_ovpChanged,show_share.get(),&Mainwindow::update_Ovp,Qt::QueuedConnection);
                QObject::connect(channel.get(),&UartChannelManager::CH_isOutputChanged,show_share.get(),&Mainwindow::update_IsOutput,Qt::QueuedConnection);
                QObject::connect(channel.get(),&UartChannelManager::CH_impChanged,show_share.get(),&Mainwindow::update_Imp,Qt::QueuedConnection);

                /* screen view slot function */
                QObject::connect(show_share.get(),qml_signal[config.channel-1],channel.get(),&UartChannelManager::writeFrame,Qt::QueuedConnection);
            }

            channel->m_scpiManager = Scpi_share;
            Channel_list.push_back(std::move(channel)); // move set <channel> can move
        }
    }

    if (ConfigManager::s_enableDisplay){QTimer::singleShot(36, show_share.get(), &Mainwindow::update_cardtest);}

    //QTimer::singleShot(9000, &app, &QGuiApplication::quit); // 9s
    return app.exec();
}

