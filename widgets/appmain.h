#pragma once
#include <QWidget>
namespace Ui {class Mainwindow;}

class BatteryModelManager;

class UartChannelManager;
class UartServerManager;
class CanServerManager;
class VxiServerManager;
class WebServerManager;

class remoteoverlay;
class devicesetpage;
class channelsetting;
class channelpage;

class QGraphicsScene;
class QGraphicsView;

class Mainwindow : public QWidget
{
    Q_OBJECT

public:
    explicit Mainwindow(QWidget *parent = nullptr);
    ~Mainwindow();

    void initUi();
    void initO();
    void ishow();

private:
    std::shared_ptr<BatteryModelManager> batteryModel;

    std::vector<std::unique_ptr<UartChannelManager>> channels;
    std::unique_ptr<UartServerManager> uartServer;
    std::unique_ptr<CanServerManager> canServer;
    std::unique_ptr<VxiServerManager> vxiServer;
    std::unique_ptr<WebServerManager> webServer;

    remoteoverlay* m_RemoteOverlay;
    devicesetpage* m_DeviceSetPage;
    channelsetting* m_ChSetPage;
    channelpage* m_ChPage;
    Ui::Mainwindow *ui;

    QGraphicsScene* scene;
    QGraphicsView* view;
};
