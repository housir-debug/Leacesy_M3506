#include "appmain.h"
#include "ui_appmain.h"
#include "auxiliary/uartProtocol.h"
Q_LOGGING_CATEGORY(widget, "WIDGET:")


Mainwindow::Mainwindow(QWidget *parent) : QWidget(parent), ui(new Ui::Mainwindow)
{
    QString configPath = QDir(QApplication::applicationDirPath()).filePath("..");
    if (QDir(configPath).exists() && ConfigManager::init(configPath)) {
        QString rules; // if-else inside init configmanager

        if (ConfigManager::s_loglevel == "release"){
            rules = "*.debug=false\n*.info=false\n*.warning=false";
        }else if (ConfigManager::s_loglevel == "debug"){
            rules = "*.debug=true\n*.info=true\n*.warning=true";
        }else if (ConfigManager::s_loglevel == "self"){
            rules = "*.debug=false\n*.info=false\n*.warning=false\n"
                "SCPI:.debug=true\n";
        }

        QLoggingCategory::setFilterRules(rules);
        if (ConfigManager::s_enableDisplay) {
            batteryModel = std::make_shared<BatteryModelManager>(configPath);
            initUi();
        }

        if (ConfigManager::s_enableUartMess){
            initChannels();
            initControl();
        }
    }
}

Mainwindow::~Mainwindow()
{
    delete ui;
}


void Mainwindow::initChannels(){
    using CanSign_toUartCh = void (CanServerManager::*)(quint8 cmd, quint8 func, const QByteArray& param,bool isScpi);
    std::vector<CanSign_toUartCh> can_signal = {
        #define CHANNEL(n) static_cast<CanSign_toUartCh>(&CanServerManager::to_UartChannel##n),
        CHANNEL_COUNT
        #undef CHANNEL
    };
    using QWidgetSign_toUartCh = void (channelpage::*)(quint8 cmd, quint8 func, const QByteArray& param,bool isScpi);
    std::vector<QWidgetSign_toUartCh> qml_signal = {
        #define CHANNEL(n) static_cast<QWidgetSign_toUartCh>(&channelpage::to_UartChannel##n),
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

    for (const auto& config : configs) {
        auto channel = std::make_unique<UartChannelManager>();
        channel->initUartchannel(config.channel, config.port, config.baudRate);
        QObject::connect(Scpi_share.get(),scpi_signal[config.channel-1],channel.get(),&UartChannelManager::writeFrame,Qt::QueuedConnection);
        QObject::connect(Scpi_share.get(),scpi_rst_signal[config.channel-1],channel.get(),&UartChannelManager::sendInitCommand,Qt::QueuedConnection);

        if (canServer){
            QObject::connect(canServer.get(),can_signal[config.channel-1],channel.get(),&UartChannelManager::writeFrame,Qt::QueuedConnection);
            QObject::connect(channel.get(),&UartChannelManager::to_CanServer,canServer.get(),&CanServerManager::sendFrame,Qt::QueuedConnection);
        }

        if (m_ChPage){
            QObject::connect(channel.get(),&UartChannelManager::CH_svChanged,m_ChPage.get(),&channelpage::update_SoftVer,Qt::QueuedConnection);
            QObject::connect(channel.get(),&UartChannelManager::CH_hvChanged,m_ChPage.get(),&channelpage::update_HardVer,Qt::QueuedConnection);
            QObject::connect(channel.get(),&UartChannelManager::CH_VoltageChanged,m_ChPage.get(),&channelpage::update_Voltage,Qt::QueuedConnection);
            QObject::connect(channel.get(),&UartChannelManager::CH_CurrentAndUnitChanged,m_ChPage.get(),&channelpage::update_CurrentAndUnit,Qt::QueuedConnection);
            QObject::connect(channel.get(),&UartChannelManager::CH_RangeChanged,m_ChPage.get(),&channelpage::update_Range,Qt::QueuedConnection);
            QObject::connect(channel.get(),&UartChannelManager::CH_StatusChanged,m_ChPage.get(),&channelpage::update_Status,Qt::QueuedConnection);
            QObject::connect(channel.get(),&UartChannelManager::CH_cvChanged,m_ChPage.get(),&channelpage::update_Cv,Qt::QueuedConnection);
            QObject::connect(channel.get(),&UartChannelManager::CH_ccChanged,m_ChPage.get(),&channelpage::update_Cc,Qt::QueuedConnection);
            QObject::connect(channel.get(),&UartChannelManager::CH_ovpChanged,m_ChPage.get(),&channelpage::update_Ovp,Qt::QueuedConnection);
            QObject::connect(channel.get(),&UartChannelManager::CH_isOutputChanged,m_ChPage.get(),&channelpage::update_IsOutput,Qt::QueuedConnection);
            QObject::connect(channel.get(),&UartChannelManager::CH_impChanged,m_ChPage.get(),&channelpage::update_Imp,Qt::QueuedConnection);

            /* screen view slot function */
            QObject::connect(m_ChPage.get(),qml_signal[config.channel-1],channel.get(),&UartChannelManager::writeFrame,Qt::QueuedConnection);
        }

        channel->m_scpiManager = Scpi_share;
        channels.push_back(std::move(channel));
    }
}

void Mainwindow::initControl()
{
    if (ConfigManager::s_enableCANServer){
        canServer = std::make_unique<CanServerManager>();

        if(m_DeviceSetPage){
            QObject::connect(canServer.get(),&CanServerManager::isRemote,this,&Mainwindow::update_remotemodel,Qt::QueuedConnection);
            QObject::connect(canServer.get(),&CanServerManager::baudrefresh,m_DeviceSetPage,&devicesetpage::responseUpdate,Qt::QueuedConnection);
            QObject::connect(m_DeviceSetPage,&devicesetpage::set_canbaud,canServer.get(),&CanServerManager::changebaudandid,Qt::QueuedConnection);
        }
    }

    Scpi_share = std::make_shared<ScpiManager>();
    if (m_DeviceSetPage){
        QObject::connect(Scpi_share.get(),&ScpiManager::networkrefresh,m_DeviceSetPage,&devicesetpage::responseUpdate,Qt::QueuedConnection);
    }

    if (ConfigManager::s_enableLANServer){
        vxiServer = std::make_unique<VxiServerManager>();
        vxiServer->m_scpiManager = Scpi_share;

        if (m_DeviceSetPage){
            QObject::connect(vxiServer.get(),&VxiServerManager::isRemote,this,&Mainwindow::update_remotemodel,Qt::QueuedConnection);
        }
    }

    if (ConfigManager::s_enableUARTServer){
        uartServer = std::make_unique<UartServerManager>();
        uartServer->m_scpiManager = Scpi_share;

        if(m_DeviceSetPage){
            QObject::connect(uartServer.get(),&UartServerManager::isRemote,this,&Mainwindow::update_remotemodel,Qt::QueuedConnection);
            QObject::connect(uartServer.get(),&UartServerManager::baudrefresh,m_DeviceSetPage,&devicesetpage::responseUpdate,Qt::QueuedConnection);
            QObject::connect(m_DeviceSetPage,&devicesetpage::set_RS232Baud,uartServer.get(),&UartServerManager::changeBaudRate,Qt::QueuedConnection);
        }
    }
}

void Mainwindow::initUi()
{
    ui->setupUi(this);

    m_ChPage = std::make_shared<channelpage>(this);
    ui->stackedwidget->addWidget(m_ChPage.get());// 0
    m_ChPage->m_modelManager = batteryModel;
    m_ChSetPage = new channelsetting(this);
    ui->stackedwidget->addWidget(m_ChSetPage);// 1
    m_ChSetPage->m_modelManager = batteryModel;
    m_DeviceSetPage = new devicesetpage(this);
    ui->stackedwidget->addWidget(m_DeviceSetPage);// 2

    connect(m_ChPage.get(),&channelpage::jump_chsetpage,this,[this]{
        ui->stackedwidget->setCurrentIndex(1);
        });
    connect(m_ChPage.get(),&channelpage::jump_devicesetpage,this,[this]{
        ui->stackedwidget->setCurrentIndex(2);
        });
    connect(m_ChSetPage,&channelsetting::backRequested,this,[this]{
        ui->stackedwidget->setCurrentIndex(0);
        });
    connect(m_DeviceSetPage,&devicesetpage::backRequested,this,[this]{
        ui->stackedwidget->setCurrentIndex(0);
        });

    m_RemoteOverlay = new remoteoverlay(this);
    connect(m_RemoteOverlay, &remoteoverlay::exitRemote, this, []() {
        ConfigManager::s_remoteSt.store(InterSt::LOCAL);
        });

    scene = new QGraphicsScene(this);
    view  = new QGraphicsView(scene);
    QGraphicsProxyWidget* proxy = scene->addWidget(this);
    proxy->setFocusPolicy(Qt::NoFocus);
    proxy->setRotation(0);

    view->viewport()->setAttribute(Qt::WA_AcceptTouchEvents, false);
    view->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setDragMode(QGraphicsView::NoDrag);
    view->setFrameShape(QFrame::NoFrame);
    //view.rotate(0);
    view->show();
    //this->show();

    webServer = std::make_unique<WebServerManager>();
    webServer->m_BatteryManager = batteryModel;
    webServer->m_scpiManager = Scpi_share;
    webServer->m_qmlbridge = m_ChPage;

    QObject::connect(webServer.get(),&WebServerManager::isRemote,this,&Mainwindow::update_remotemodel,Qt::QueuedConnection);
    QObject::connect(webServer.get(),&WebServerManager::networkrefresh,m_DeviceSetPage,&devicesetpage::responseUpdate,Qt::QueuedConnection);

    m_ChPage->load_BatteryModel();
}

void Mainwindow::update_remotemodel(int reface){
    if (ConfigManager::s_remoteSt.load() != reface){
        ConfigManager::s_remoteSt.store(reface);
        if (reface == InterSt::LOCAL){
            m_RemoteOverlay->hide();
        }else{
            m_RemoteOverlay->show();
        }
    }
}

