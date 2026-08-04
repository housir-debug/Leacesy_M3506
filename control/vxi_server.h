#pragma once
#include <QTcpServer>
#include "auxiliary/scpi_handle.h"

Q_DECLARE_LOGGING_CATEGORY(vxi_server)

struct DeviceLink {
    quint32 id;
    QTcpSocket* client;
    QByteArray VxiScpi_response;

    DeviceLink(): id(0),client(nullptr){}
};

class VxiServerManager : public QObject
{
    Q_OBJECT

signals:
    void isRemote(quint8 reface);

public:
    explicit VxiServerManager(QObject *parent = nullptr);
    ~VxiServerManager();

    std::shared_ptr<ScpiManager> m_scpiManager{nullptr};

private:
    void processClientData      (QTcpSocket* client);
    void handleCreateLink       (QTcpSocket* client,quint32 xid);
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
    int  checkRequestlinkid     (QTcpSocket* client,quint32 xid,const uchar* address);
    void buildfoundResponse     (quint32 xid,quint32 error,quint32 extraleng = 0);

private:
    quint32 m_nextLinkId{1};
    QMap<quint8, DeviceLink> m_deviceLinks;

    QByteArray m_readbuffer;
    QByteArray m_responsebuffer;

    QList<QTcpSocket*> m_clients;
    QTcpServer* m_tcpServer{nullptr};
    QThread* m_serverThread{nullptr};
};

