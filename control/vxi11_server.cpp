#include "auxiliary/global_static.h"
#include "auxiliary/signal_bus.h"
#include "channel/scpi_handle.h"
#include "vxi_protocol.h"
#include "vxi11_server.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>

extern "C" {
#include "vxi11core.h"
extern void device_core_1(struct svc_req* rqstp, SVCXPRT* transp);
extern void device_async_1(struct svc_req* rqstp, SVCXPRT* transp);
// =====================================================================
// RPC-VXI-11 过程号 跳板函数
// =====================================================================
Device_Error* device_abort_1_svc(Device_Link* argp, struct svc_req* rqstp)
{
    // Repeats the SCPI instruction. Not supported.
    qCDebug(vxiSer)<<"(device_abort): Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp;
    static Device_Error resp;
    resp.error = Vxi11::ErrorCode::NO_ERROR;
    return &resp;
}
Create_LinkResp* create_link_1_svc(Create_LinkParms* argp, struct svc_req* rqstp)
{
    qCDebug(vxiSer)<<"(create_link): Ser_Id "<<rqstp->rq_xprt;
    static Create_LinkResp resp;
    static int linkId = 0;

    if (argp->device != nullptr && strcmp(argp->device, "inst0") == 0){
        resp.error = Vxi11::ErrorCode::NO_ERROR;
        linkId += 1;

        if (rqstp->rq_xprt == vxi11_server::instance().m_transpTcp){
            resp.abortPort = vxi11_server::instance().m_transpTcp->xp_port;
        }else{
            resp.abortPort = vxi11_server::instance().m_transpUdp->xp_port;
        }
    }else{
        resp.error = Vxi11::ErrorCode::INVALID_LINK_IDENTIFIER;
    }

    resp.maxRecvSize = 1024 * 1024; // 1MB
    resp.lid = linkId;

    return &resp;
}
Device_WriteResp* device_write_1_svc(Device_WriteParms* argp, struct svc_req* rqstp)
{
    qCDebug(vxiSer)<<"(device_write): Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp->lid;
    static Device_WriteResp resp;

    if (vxi11_server::instance().m_lockLinkId == argp->lid || vxi11_server::instance().m_lockLinkId == -1) {
        qCDebug(vxiSer)<<"[device_write]: SCPI-Command: "<<argp->data.data_val;
        vxi11_server::instance().m_tempbuffer = ScpiManager::instance().processCommand(argp->data.data_val);
        qCDebug(vxiSer)<<"[device_write]: SCPI-Response: "<<vxi11_server::instance().m_tempbuffer;

        resp.error = Vxi11::ErrorCode::NO_ERROR;
        resp.size = argp->data.data_len;
    }else{
        resp.error = Vxi11::ErrorCode::DEVICE_LOCKED_BY_ANOTHER_LINK;
        resp.size = 0;
    }

    return &resp;
}
Device_ReadResp* device_read_1_svc(Device_ReadParms* argp, struct svc_req* rqstp)
{
    qCDebug(vxiSer)<<"(device_read): Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp->lid;
    static Device_ReadResp resp;

    if (vxi11_server::instance().m_lockLinkId == argp->lid || vxi11_server::instance().m_lockLinkId == -1) {
        resp.error = Vxi11::ErrorCode::NO_ERROR;
        resp.data.data_len = vxi11_server::instance().m_tempbuffer.size();
        resp.data.data_val = vxi11_server::instance().m_tempbuffer.data();
    }else{
        resp.error = Vxi11::ErrorCode::DEVICE_LOCKED_BY_ANOTHER_LINK;
        resp.data.data_len = 0;
        resp.data.data_val = nullptr;
    }

    resp.reason = 4; // END_FLAG (0: force read request-size Byte)
    return &resp;
}
Device_ReadStbResp* device_readstb_1_svc(Device_GenericParms* argp, struct svc_req* rqstp)
{
    qCDebug(vxiSer)<<"(device_readstb): Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp->lid;
    static Device_ReadStbResp resp;

    QByteArray result = ScpiManager::instance().processCommand("*STB?\n");
    resp.stb = static_cast<u_char>(result.trimmed().toInt());
    resp.error = Vxi11::ErrorCode::NO_ERROR;

    return &resp;
}
Device_Error* device_trigger_1_svc(Device_GenericParms* argp, struct svc_req* rqstp)
{
    // Repeats the SCPI instruction. Not supported.
    qCDebug(vxiSer)<<"(device_trigger): Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp->lid;
    static Device_Error resp;
    resp.error = Vxi11::ErrorCode::NO_ERROR;
    return &resp;
}
Device_Error* device_clear_1_svc(Device_GenericParms* argp, struct svc_req* rqstp)
{
    qCDebug(vxiSer)<<"(device_clear): Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp->lid;
    static Device_Error resp;

    ScpiManager::instance().processCommand("*CLS\n");
    vxi11_server::instance().m_tempbuffer.clear();

    resp.error = Vxi11::ErrorCode::NO_ERROR;
    return &resp;
}
Device_Error* device_remote_1_svc(Device_GenericParms* argp, struct svc_req* rqstp)
{
    qCDebug(vxiSer)<<"(device_remote): Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp->lid;
    static Device_Error resp;

    emit ChannelBus::instance().switchRemote(InterSt::LAN_VXI);

    resp.error = Vxi11::ErrorCode::NO_ERROR;
    return &resp;
}
Device_Error* device_local_1_svc(Device_GenericParms* argp, struct svc_req* rqstp)
{
    qCDebug(vxiSer)<<"(device_local): Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp->lid;
    static Device_Error resp;

    emit ChannelBus::instance().switchRemote(InterSt::LOCAL);

    resp.error = Vxi11::ErrorCode::NO_ERROR;
    return &resp;
}
Device_Error* device_lock_1_svc(Device_LockParms* argp, struct svc_req* rqstp)
{
    qCDebug(vxiSer)<<"(device_lock): Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp->lid;
    static Device_Error resp;

    if (vxi11_server::instance().m_lockLinkId == -1) {
        vxi11_server::instance().m_lockLinkId = argp->lid;
        resp.error = Vxi11::ErrorCode::NO_ERROR;
    }else{
        resp.error = Vxi11::ErrorCode::DEVICE_LOCKED_BY_ANOTHER_LINK;
    }

    return &resp;
}
Device_Error* device_unlock_1_svc(Device_Link* argp, struct svc_req* rqstp)
{
    qCDebug(vxiSer)<<"(device_unlock): Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp;
    static Device_Error resp;

    if (vxi11_server::instance().m_lockLinkId == *argp) {
       vxi11_server::instance().m_lockLinkId = -1;
       resp.error = Vxi11::ErrorCode::NO_ERROR;
   } else {
       resp.error = Vxi11::ErrorCode::DEVICE_LOCKED_BY_ANOTHER_LINK;
   }

    return &resp;
}
Device_Error* device_enable_srq_1_svc(Device_EnableSrqParms* argp, struct svc_req* rqstp)
{
    // Repeats the SCPI instruction. Not supported.
    qCDebug(vxiSer)<<"(device_enable_srq): Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp;
    static Device_Error resp;
    resp.error = Vxi11::ErrorCode::NO_ERROR;
    return &resp;
}
Device_DocmdResp* device_docmd_1_svc(Device_DocmdParms* argp, struct svc_req* rqstp)
{
    // Repeats the SCPI instruction. Not supported.
    qCDebug(vxiSer)<<"(device_docmd): Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp;
    static Device_DocmdResp resp;
    resp.error = Vxi11::ErrorCode::NO_ERROR;
    resp.data_out.data_out_len = 0;
    resp.data_out.data_out_val = nullptr;
    return &resp;
}
Device_Error* destroy_link_1_svc(Device_Link* argp, struct svc_req* rqstp)
{
    qCDebug(vxiSer)<<"(destroy_link): Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp;
    static Device_Error resp;

    resp.error = Vxi11::ErrorCode::NO_ERROR;
    return &resp;
}
Device_Error* create_intr_chan_1_svc(Device_RemoteFunc* argp, struct svc_req* rqstp)
{
    // Repeats the SCPI instruction. Not supported.
    qCDebug(vxiSer)<<"(create_intr_chan): Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp;
    static Device_Error resp;
    resp.error = Vxi11::ErrorCode::NO_ERROR;
    return &resp;
}
Device_Error* destroy_intr_chan_1_svc(void* argp, struct svc_req* rqstp)
{
    // Repeats the SCPI instruction. Not supported.
    qCDebug(vxiSer)<<"(destroy_intr_chan): Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp;
    static Device_Error resp;
    resp.error = Vxi11::ErrorCode::NO_ERROR;
    return &resp;
}
}

vxi11_server& vxi11_server::instance()
{
    static vxi11_server inst;
    return inst;
}
void vxi11_server::stop() {
    for (QTcpSocket* client : qAsConst(m_clients)) {
        if (client) {
            client->disconnect(this);
            client->deleteLater();
        }
    }
    m_clients.clear();
    svc_exit();

    if (m_transpUdp) {
        svc_destroy(m_transpUdp);
        m_transpUdp = nullptr;
    }
    if (m_transpTcp) {
        svc_destroy(m_transpTcp);
        m_transpTcp = nullptr;
    }
    if (m_rpcThread) {
        m_rpcThread->wait(3000);
        delete m_rpcThread;
        m_rpcThread = nullptr;
    }
}
vxi11_server::vxi11_server(QObject *parent) : QObject(parent)
{
    // VXI-11 Raw Socket
    m_sockUdp = new QUdpSocket(this);
    connect(m_sockUdp, &QUdpSocket::readyRead, this, [this]() {
        while (m_sockUdp->hasPendingDatagrams()) {
            quint16 senderPort;
            QHostAddress sender;
            m_readbuffer.clear();
            m_readbuffer.resize(m_sockUdp->pendingDatagramSize());
            m_sockUdp->readDatagram(m_readbuffer.data(), m_readbuffer.size(), &sender, &senderPort);
            qCDebug(vxiSer)<<"[vxi11_server]:SOCKET Request: "<<m_readbuffer;

            if (isVxiRemote()){
                if (m_readbuffer.startsWith("*") || m_readbuffer.startsWith(":")) {
                    m_respbuffer = ScpiManager::instance().processCommand(m_readbuffer);
                }else{
                    m_respbuffer = "SCPI cmd format Error!";
            }}else{
                m_respbuffer = "Device on Other inter_Remote mode!";
            }

            if (!m_respbuffer.isEmpty()){
                qCDebug(vxiSer)<<"[vxi11_server]:SOCKET Response: "<<m_respbuffer;
                m_sockUdp->writeDatagram(m_respbuffer, sender, senderPort);
            }
        }}, Qt::DirectConnection);

    m_sockTcp = new QTcpServer(this);
    connect(m_sockTcp,&QTcpServer::newConnection,this,[this](){
        QTcpSocket* client = m_sockTcp->nextPendingConnection();
        if (m_clients.size() < 9) { // restrict connect number = 9
            connect(client, &QTcpSocket::errorOccurred,this, [client](QAbstractSocket::SocketError error){
                qCWarning(vxiSer)<<"[vxi11_server]:("<< client->objectName()<<")"<<error<<client->errorString();
                }, Qt::DirectConnection);
            connect(client, &QTcpSocket::disconnected,this,[this,client](){
                qCDebug(vxiSer)<<"[vxi11_server]: "<<client->objectName()<<" disconnected";
                m_clients.removeOne(client);
                client->deleteLater();
                }, Qt::DirectConnection);
            connect(client, &QTcpSocket::readyRead,this, [this, client](){
                m_readbuffer = client->readAll();
                qCDebug(vxiSer)<<"[vxi11_server]:SOCKET Request: "<<m_readbuffer;

                if (isVxiRemote()){
                    if (m_readbuffer.startsWith("*") || m_readbuffer.startsWith(":")) {
                        m_respbuffer = ScpiManager::instance().processCommand(m_readbuffer);
                    }else{
                        m_respbuffer = "SCPI cmd format Error!";
                }}else{
                    m_respbuffer = "Device on Other inter_Remote mode!";
                }

                if (!m_respbuffer.isEmpty()){
                    qCDebug(vxiSer)<<"[vxi11_server]:SOCKET Response: "<<m_respbuffer;
                    client->write(m_respbuffer);
                }}, Qt::DirectConnection);

            client->setObjectName(QString("%1:%2").arg(client->peerAddress().toString()).arg(client->peerPort()));
            qCDebug(vxiSer)<<"[vxi11_server]:New Connect-Client: "<<client->objectName();
            m_clients.append(client);
            return;
        }

        client->disconnectFromHost();
        client->deleteLater();
        }, Qt::DirectConnection);

    if (m_sockTcp->listen(QHostAddress::Any, Vxi11::VXI_PORT) &&
            m_sockUdp->bind(QHostAddress::Any, Vxi11::VXI_PORT, QUdpSocket::ShareAddress)) {

        // VXI-11 LAN Instrument
        pmap_unset(DEVICE_CORE,DEVICE_CORE_VERSION);
        pmap_unset(DEVICE_ASYNC,DEVICE_ASYNC_VERSION);

        m_transpUdp = svcudp_create(RPC_ANYSOCK);
        if (m_transpUdp != NULL) {
            svc_register(m_transpUdp , DEVICE_CORE, DEVICE_CORE_VERSION, device_core_1, IPPROTO_UDP);
            svc_register(m_transpUdp , DEVICE_ASYNC, DEVICE_ASYNC_VERSION, device_async_1, IPPROTO_UDP);

            m_transpTcp = svctcp_create(RPC_ANYSOCK, 0, 0);
            if (m_transpTcp != NULL) {
                svc_register(m_transpTcp, DEVICE_CORE, DEVICE_CORE_VERSION, device_core_1, IPPROTO_TCP);
                svc_register(m_transpTcp, DEVICE_ASYNC, DEVICE_ASYNC_VERSION, device_async_1, IPPROTO_TCP);

                qCDebug(vxiSer)<<"VXI-11 UDP "<<m_transpUdp<<",Port "<<m_transpUdp->xp_port<<", TCP "<<m_transpTcp<<",Port "<<m_transpTcp->xp_port;
                m_rpcThread = new RpcThread();
                m_rpcThread->setObjectName("vxiSubThread");
                m_rpcThread->start();
                return;
            }

            qCCritical(vxiSer)<<"[vxi11_server]:cannot create TCP_Server!!!";
            return;
        }

        qCCritical(vxiSer)<<"[vxi11_server]:cannot create UDP_Server!!!";
        return;
    }

    qCritical(vxiSer)<<"[vxi11_server]: Fail bind_Socket UDP or TCP!!!";
}
bool vxi11_server::isVxiRemote()
{
    if(ConfigManager::control_Mode.load() == InterSt::LAN_VXI){
        return true;
    }else if (ConfigManager::control_Mode.load() == InterSt::LOCAL){
        emit ChannelBus::instance().switchRemote(InterSt::LAN_VXI);
        return true;
    }

    qCWarning(vxiSer)<<"[isVxiRemote]: Device on Other inter_Remote mode!";
    return false;
}
