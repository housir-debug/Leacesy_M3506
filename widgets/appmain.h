#pragma once
#include <QWidget>
#include <QGraphicsView>
#include <QLoggingCategory>
#include <QGraphicsProxyWidget>
#include "control/web_server.h"
#include "control/can_server.h"
#include "control/vxi_server.h"
#include "control/uart_server.h"
#include "channel/uart_channel.h"
#include "component/remoteoverlay.h"
#include "interaction/devicesetpage.h"
#include "interaction/channelsetting.h"
Q_DECLARE_LOGGING_CATEGORY(widget)

namespace Ui {class Mainwindow;}

class Mainwindow : public QWidget
{
    Q_OBJECT

public:
    explicit Mainwindow(QWidget *parent = nullptr);
    ~Mainwindow();

    void initChannels();
    void initControl();
    void initUi();

    void update_remotemodel(int reface);

private:
    Ui::Mainwindow *ui;
    QGraphicsView* view;
    QGraphicsScene* scene;
    channelsetting* m_ChSetPage;
    devicesetpage* m_DeviceSetPage;
    remoteoverlay* m_RemoteOverlay;
    std::shared_ptr<channelpage> m_ChPage;
    std::unique_ptr<WebServerManager> webServer;
    std::shared_ptr<BatteryModelManager> batteryModel;

    std::shared_ptr<ScpiManager> Scpi_share;
    std::unique_ptr<CanServerManager> canServer;
    std::unique_ptr<VxiServerManager> vxiServer;
    std::unique_ptr<UartServerManager> uartServer;
    std::vector<std::unique_ptr<UartChannelManager>> channels;
};
