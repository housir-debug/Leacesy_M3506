#pragma once
#include <QThread>
#include <QObject>
#include <rpc/rpc.h>

class QTcpServer;
class QTcpSocket;
class QUdpSocket;
class vxi11_server : public QObject
{
    Q_OBJECT
private:
    class RpcThread : public QThread {
    protected:
        void run() override {
            svc_run();
        }};
    RpcThread* m_rpcThread{nullptr};

    QUdpSocket* m_sockUdp{nullptr};
    QTcpServer* m_sockTcp{nullptr};
    QList<QTcpSocket*> m_clients;

    QByteArray m_respbuffer;
    QByteArray m_readbuffer;

    explicit vxi11_server(QObject *parent = nullptr);
    ~vxi11_server() = default;
    Q_DISABLE_COPY(vxi11_server)

public:
    static vxi11_server& instance();
    void stop();

    SVCXPRT* m_transpUdp = nullptr;
    SVCXPRT* m_transpTcp = nullptr;

    QByteArray m_tempbuffer;
    QList<int> m_activeLinks;
    int m_lockLinkId = -1;
    int m_newLinkId = 0;
};
