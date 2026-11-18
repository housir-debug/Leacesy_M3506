#pragma once
#include <QWidget>
// --- channel ---
class UartChannelManager;
// --- control ---
class UartServerManager;
class CanServerManager;
class WebServerManager;
// --- UI page ---
class remoteoverlay;
class devicesetpage;
class channelsetting;
class channelpage;
// --- UI view ---
class QGraphicsScene;
class QGraphicsView;
namespace Ui {class Mainwindow;}
class Mainwindow : public QWidget
{
    Q_OBJECT
private:
    // --- channel ---
    std::vector<UartChannelManager*> channels;
    std::vector<QThread*> m_chThreads;
    // --- control ---
    UartServerManager* uartServer{nullptr};
    CanServerManager* canServer{nullptr};
    WebServerManager* webServer{nullptr};
    QThread* m_serThread{nullptr};
    // --- UI page ---
    remoteoverlay* m_RemoteOverlay{nullptr};
    devicesetpage* m_DeviceSetPage{nullptr};
    channelsetting* m_ChSetPage{nullptr};
    channelpage* m_ChPage{nullptr};
    Ui::Mainwindow *ui;
    // --- UI view ---
    QGraphicsScene* scene{nullptr};
    QGraphicsView* view{nullptr};

public:
    explicit Mainwindow(QWidget *parent = nullptr);
    ~Mainwindow();

    void initUi();
    void testUi();
    void initO();
};
