#pragma once
#include <QObject>

class QTcpServer;
class QTcpSocket;

class VxiServerManager : public QObject
{
    Q_OBJECT

private:
    struct DeviceLink {
        quint32 id;
        QTcpSocket* client;
        QByteArray VxiScpi_response;

        DeviceLink(): id(0),client(nullptr){}
    };
    quint32 m_nextLinkId{1};
    QVector<DeviceLink> m_deviceLinks;

    QByteArray m_readbuffer;
    QByteArray m_responsebuffer;

    QList<QTcpSocket*> m_clients;
    QTcpServer* m_tcpServer{nullptr};

public:
    explicit VxiServerManager(QObject *parent = nullptr);
    ~VxiServerManager();

private:
    void processClientData      (QTcpSocket* client);
    void handleCreateLink       (QTcpSocket* client,quint32 xid);
    DeviceLink* checkRequest    (QTcpSocket* client,quint32 xid,const uchar* address);

    void handleDeviceWrite      (QTcpSocket* client,quint32 xid,const uchar* address);
    void handleDeviceRead       (QTcpSocket* client,quint32 xid,const uchar* address);
    void handleDeviceReadStb    (QTcpSocket* client,quint32 xid,const uchar* address);
    void handleDeviceTrigger    (QTcpSocket* client,quint32 xid,const uchar* address);
    void handleDeviceClear      (QTcpSocket* client,quint32 xid,const uchar* address);
    void handleDeviceRemote     (QTcpSocket* client,quint32 xid,const uchar* address);
    void handleDeviceLocal      (QTcpSocket* client,quint32 xid,const uchar* address);
    void handleDeviceLock       (QTcpSocket* client,quint32 xid,const uchar* address);
    void handleDeviceUnlock     (QTcpSocket* client,quint32 xid,const uchar* address);
    void handleDeviceEnableSrq  (QTcpSocket* client,quint32 xid,const uchar* address);
    void handleDeviceDocmd      (QTcpSocket* client,quint32 xid,const uchar* address);
    void handleDestroyLink      (QTcpSocket* client,quint32 xid,const uchar* address);
    void handleCreateIntrChan   (QTcpSocket* client,quint32 xid,const uchar* address);
    void handleDestroyIntrChan  (QTcpSocket* client,quint32 xid,const uchar* address);
    void buildfoundResponse     (quint32 xid,quint32 error,quint32 extraleng = 0);
};

