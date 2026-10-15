#pragma once
#include <QWidget>
namespace Ui {class Mainwindow;}

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
    std::vector<std::unique_ptr<UartChannelManager>> channels;
    UartServerManager* uartServer{nullptr};
    CanServerManager* canServer{nullptr};
    VxiServerManager* vxiServer{nullptr};
    WebServerManager* webServer{nullptr};
    QThread* m_serThread{nullptr};

    remoteoverlay* m_RemoteOverlay{nullptr};
    devicesetpage* m_DeviceSetPage{nullptr};
    channelsetting* m_ChSetPage{nullptr};
    channelpage* m_ChPage{nullptr};
    Ui::Mainwindow *ui;

    QGraphicsScene* scene{nullptr};
    QGraphicsView* view{nullptr};
};
