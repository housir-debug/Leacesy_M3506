#include "auxiliary/global_static.h"
#include "auxiliary/signal_bus.h"
#include "channel/scpi_handle.h"
#include "vxi_protocol.h"
#include "vxi11_server.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>

#ifdef __cplusplus
extern "C" {
#endif
#include "vxi11core.h"
extern void device_core_1(struct svc_req* rqstp, SVCXPRT* transp);
extern void device_async_1(struct svc_req* rqstp, SVCXPRT* transp);
// =====================================================================
// RPC - VXI-11 过程号 跳板函数
// =====================================================================
Device_Error* device_abort_1_svc(Device_Link* argp, struct svc_req* rqstp)
{
    static Device_Error resp;

    // Repeats the SCPI instruction. Not supported.
    if (argp != nullptr && rqstp != nullptr){
        qCDebug(vxiSer)<<"(device_abort)Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp;
        resp.error = Vxi11::ErrorCode::NO_ERROR;
    }else{
        qCWarning(vxiSer)<<"(device_abort)PARAMETER_ERROR";
        resp.error = Vxi11::ErrorCode::PARAMETER_ERROR;
    }

    return &resp;
}
Create_LinkResp* create_link_1_svc(Create_LinkParms* argp, struct svc_req* rqstp)
{
    static Create_LinkResp resp;

    if (argp != nullptr && rqstp != nullptr){
        qCDebug(vxiSer)<<"(create_link)Ser_Id "<<rqstp->rq_xprt;

        if (argp->device != nullptr && strcmp(argp->device, "inst0") == 0){
            if (!argp->lockDevice || vxi11_server::instance().m_lockLinkId == -1){
                vxi11_server::instance().m_newLinkId += 1;
                vxi11_server::instance().m_activeLinks.append(vxi11_server::instance().m_newLinkId);

                if (argp->lockDevice){ // implict condition(m_lockLinkId == -1)
                    vxi11_server::instance().m_lockLinkId = vxi11_server::instance().m_newLinkId;
                }

                if (rqstp->rq_xprt == vxi11_server::instance().m_transpTcp){
                    resp.abortPort = vxi11_server::instance().m_transpTcp->xp_port;
                }else{
                    resp.abortPort = vxi11_server::instance().m_transpUdp->xp_port;
                }

                resp.error = Vxi11::ErrorCode::NO_ERROR;
                resp.lid = vxi11_server::instance().m_newLinkId;
            }else{
                resp.error = Vxi11::ErrorCode::DEVICE_LOCKED_BY_ANOTHER_LINK;
                resp.abortPort = 0;
                resp.lid = 0;
            }
        }else{
            resp.error = Vxi11::ErrorCode::INVALID_LINK_IDENTIFIER;
            resp.abortPort = 0;
            resp.lid = 0;
        }
    }else{
        qCWarning(vxiSer)<<"(create_link)PARAMETER_ERROR";
        resp.error = Vxi11::ErrorCode::PARAMETER_ERROR;
        resp.abortPort = 0;
        resp.lid = 0;
    }

    resp.maxRecvSize = 1024 * 64; // Byte 64KB(At least 1KB=1024B)
    return &resp;
}
Device_WriteResp* device_write_1_svc(Device_WriteParms* argp, struct svc_req* rqstp)
{
    static Device_WriteResp resp;

    if (argp != nullptr && rqstp != nullptr){
        qCDebug(vxiSer)<<"(device_write)Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp->lid;

        if (ConfigManager::isInteRemote(InterSt::LAN_VXI)){
            if (argp->lid > 0 && vxi11_server::instance().m_activeLinks.contains(argp->lid)){
                if (vxi11_server::instance().m_lockLinkId == argp->lid || vxi11_server::instance().m_lockLinkId == -1) {
                    qCDebug(vxiSer)<<"[device_write]SCPI-Command: "<<argp->data.data_val;
                    vxi11_server::instance().m_tempbuffer = ScpiManager::instance().processCommand(argp->data.data_val);
                    qCDebug(vxiSer)<<"[device_write]SCPI-Response: "<<vxi11_server::instance().m_tempbuffer;

                    resp.error = Vxi11::ErrorCode::NO_ERROR;
                    resp.size = argp->data.data_len;
                }else{
                    resp.error = Vxi11::ErrorCode::DEVICE_LOCKED_BY_ANOTHER_LINK;
                    resp.size = 0;
                }
            }else{
                resp.error = Vxi11::ErrorCode::INVALID_LINK_IDENTIFIER;
                resp.size = 0;
            }
        }else{
            resp.error = Vxi11::ErrorCode::IO_ERROR;
            resp.size = 0;
        }
    }else{
        qCWarning(vxiSer)<<"(device_write)PARAMETER_ERROR";
        resp.error = Vxi11::ErrorCode::PARAMETER_ERROR;
        resp.size = 0;
    }

    return &resp;
}
Device_ReadResp* device_read_1_svc(Device_ReadParms* argp, struct svc_req* rqstp)
{
    static Device_ReadResp resp;

    if (argp != nullptr && rqstp != nullptr){
        qCDebug(vxiSer)<<"(device_read)Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp->lid;

        if (argp->lid > 0 && vxi11_server::instance().m_activeLinks.contains(argp->lid)){
            if (vxi11_server::instance().m_lockLinkId == argp->lid || vxi11_server::instance().m_lockLinkId == -1) {
                resp.error = Vxi11::ErrorCode::NO_ERROR;
                resp.data.data_val = vxi11_server::instance().m_tempbuffer.data();
                resp.data.data_len = vxi11_server::instance().m_tempbuffer.size();
            }else{
                resp.error = Vxi11::ErrorCode::DEVICE_LOCKED_BY_ANOTHER_LINK;
                resp.data.data_val = nullptr;
                resp.data.data_len = 0;
            }
        }else{
            resp.error = Vxi11::ErrorCode::INVALID_LINK_IDENTIFIER;
            resp.data.data_val = nullptr;
            resp.data.data_len = 0;
        }
    }else{
        qCWarning(vxiSer)<<"(device_read)PARAMETER_ERROR";
        resp.error = Vxi11::ErrorCode::PARAMETER_ERROR;
        resp.data.data_val = nullptr;
        resp.data.data_len = 0;
    }

    resp.reason = Vxi11::ReadReason::REASON_END; // END_FLAG
    return &resp;
}
Device_ReadStbResp* device_readstb_1_svc(Device_GenericParms* argp, struct svc_req* rqstp)
{
    static Device_ReadStbResp resp;

    if (argp != nullptr && rqstp != nullptr){
        qCDebug(vxiSer)<<"(device_readstb)Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp->lid;

        if (argp->lid > 0 && vxi11_server::instance().m_activeLinks.contains(argp->lid)){
            if (vxi11_server::instance().m_lockLinkId == argp->lid || vxi11_server::instance().m_lockLinkId == -1) {
                QByteArray result = ScpiManager::instance().processCommand("*STB?\n");
                resp.error = Vxi11::ErrorCode::NO_ERROR;
                resp.stb = static_cast<u_char>(result.trimmed().toInt());
            }else{
                resp.error = Vxi11::ErrorCode::DEVICE_LOCKED_BY_ANOTHER_LINK;
                resp.stb = 0;
            }
        }else{
            resp.error = Vxi11::ErrorCode::INVALID_LINK_IDENTIFIER;
            resp.stb = 0;
        }
    }else{
        qCWarning(vxiSer)<<"(device_readstb)PARAMETER_ERROR";
        resp.error = Vxi11::ErrorCode::PARAMETER_ERROR;
        resp.stb = 0;
    }

    return &resp;
}
Device_Error* device_trigger_1_svc(Device_GenericParms* argp, struct svc_req* rqstp)
{
    static Device_Error resp;

    // Repeats the SCPI instruction. Not supported.
    if (argp != nullptr && rqstp != nullptr){
        qCDebug(vxiSer)<<"(device_trigger)Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp->lid;
        resp.error = Vxi11::ErrorCode::NO_ERROR;
    }else{
        qCWarning(vxiSer)<<"(device_trigger)PARAMETER_ERROR";
        resp.error = Vxi11::ErrorCode::PARAMETER_ERROR;
    }

    return &resp;
}
Device_Error* device_clear_1_svc(Device_GenericParms* argp, struct svc_req* rqstp)
{
    static Device_Error resp;

    if (argp != nullptr && rqstp != nullptr){
        qCDebug(vxiSer)<<"(device_clear)Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp->lid;

        if (argp->lid > 0 && vxi11_server::instance().m_activeLinks.contains(argp->lid)){
            if (vxi11_server::instance().m_lockLinkId == argp->lid || vxi11_server::instance().m_lockLinkId == -1) {
                ScpiManager::instance().processCommand("*CLS\n");
                vxi11_server::instance().m_tempbuffer.clear();
                resp.error = Vxi11::ErrorCode::NO_ERROR;
            }else{
                resp.error = Vxi11::ErrorCode::DEVICE_LOCKED_BY_ANOTHER_LINK;
            }
        }else{
            resp.error = Vxi11::ErrorCode::INVALID_LINK_IDENTIFIER;
        }
    }else{
        qCWarning(vxiSer)<<"(device_clear)PARAMETER_ERROR";
        resp.error = Vxi11::ErrorCode::PARAMETER_ERROR;
    }

    return &resp;
}
Device_Error* device_remote_1_svc(Device_GenericParms* argp, struct svc_req* rqstp)
{
    static Device_Error resp;

    if (argp != nullptr && rqstp != nullptr){
        qCDebug(vxiSer)<<"(device_remote)Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp->lid;

        if (argp->lid > 0 && vxi11_server::instance().m_activeLinks.contains(argp->lid)){
            if (vxi11_server::instance().m_lockLinkId == argp->lid || vxi11_server::instance().m_lockLinkId == -1) {
                emit ChannelBus::instance().switchRemote(InterSt::LAN_VXI);
                resp.error = Vxi11::ErrorCode::NO_ERROR;
            }else{
                resp.error = Vxi11::ErrorCode::DEVICE_LOCKED_BY_ANOTHER_LINK;
            }
        }else{
            resp.error = Vxi11::ErrorCode::INVALID_LINK_IDENTIFIER;
        }
    }else{
        qCWarning(vxiSer)<<"(device_remote)PARAMETER_ERROR";
        resp.error = Vxi11::ErrorCode::PARAMETER_ERROR;
    }

    return &resp;
}
Device_Error* device_local_1_svc(Device_GenericParms* argp, struct svc_req* rqstp)
{
    static Device_Error resp;

    if (argp != nullptr && rqstp != nullptr){
        qCDebug(vxiSer)<<"(device_local)Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp->lid;

        if (argp->lid > 0 && vxi11_server::instance().m_activeLinks.contains(argp->lid)){
            if (vxi11_server::instance().m_lockLinkId == argp->lid || vxi11_server::instance().m_lockLinkId == -1) {
                emit ChannelBus::instance().switchRemote(InterSt::LOCAL);
                resp.error = Vxi11::ErrorCode::NO_ERROR;
            }else{
                resp.error = Vxi11::ErrorCode::DEVICE_LOCKED_BY_ANOTHER_LINK;
            }
        }else{
            resp.error = Vxi11::ErrorCode::INVALID_LINK_IDENTIFIER;
        }
    }else{
        qCWarning(vxiSer)<<"(device_local)PARAMETER_ERROR";
        resp.error = Vxi11::ErrorCode::PARAMETER_ERROR;
    }

    return &resp;
}
Device_Error* device_lock_1_svc(Device_LockParms* argp, struct svc_req* rqstp)
{
    static Device_Error resp;

    if (argp != nullptr && rqstp != nullptr){
        qCDebug(vxiSer)<<"(device_lock)Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp->lid;

        if (argp->lid > 0 && vxi11_server::instance().m_activeLinks.contains(argp->lid)){
            if (vxi11_server::instance().m_lockLinkId == argp->lid || vxi11_server::instance().m_lockLinkId == -1) {
                if (vxi11_server::instance().m_lockLinkId == -1){
                    vxi11_server::instance().m_lockLinkId = argp->lid;
                }

                resp.error = Vxi11::ErrorCode::NO_ERROR;
            }else{
                resp.error = Vxi11::ErrorCode::DEVICE_LOCKED_BY_ANOTHER_LINK;
            }
        }else{
            resp.error = Vxi11::ErrorCode::INVALID_LINK_IDENTIFIER;
        }
    }else{
        qCWarning(vxiSer)<<"(device_lock)PARAMETER_ERROR";
        resp.error = Vxi11::ErrorCode::PARAMETER_ERROR;
    }

    return &resp;
}
Device_Error* device_unlock_1_svc(Device_Link* argp, struct svc_req* rqstp)
{
    static Device_Error resp;

    if (argp != nullptr && rqstp != nullptr){
        qCDebug(vxiSer)<<"(device_unlock)Ser_Id "<<rqstp->rq_xprt<<",Link "<<*argp;

        if (*argp > 0 && vxi11_server::instance().m_activeLinks.contains(*argp)){
            if (vxi11_server::instance().m_lockLinkId == *argp || vxi11_server::instance().m_lockLinkId == -1) {
                if (vxi11_server::instance().m_lockLinkId == *argp){
                    vxi11_server::instance().m_lockLinkId = -1;
                }

                resp.error = Vxi11::ErrorCode::NO_ERROR;
            } else {
               resp.error = Vxi11::ErrorCode::DEVICE_LOCKED_BY_ANOTHER_LINK;
            }
        }else{
            resp.error = Vxi11::ErrorCode::INVALID_LINK_IDENTIFIER;
        }
    }else{
        qCWarning(vxiSer)<<"(device_unlock)PARAMETER_ERROR";
        resp.error = Vxi11::ErrorCode::PARAMETER_ERROR;
    }

    return &resp;
}
Device_Error* device_enable_srq_1_svc(Device_EnableSrqParms* argp, struct svc_req* rqstp)
{
    static Device_Error resp;

    // Repeats the SCPI instruction. Not supported.
    if (argp != nullptr && rqstp != nullptr){
        qCDebug(vxiSer)<<"(device_enable_srq)Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp->enable;
        resp.error = Vxi11::ErrorCode::NO_ERROR;
    }else{
        qCWarning(vxiSer)<<"(device_enable_srq)PARAMETER_ERROR";
        resp.error = Vxi11::ErrorCode::PARAMETER_ERROR;
    }

    return &resp;
}
Device_DocmdResp* device_docmd_1_svc(Device_DocmdParms* argp, struct svc_req* rqstp)
{
    static Device_DocmdResp resp;

    // Repeats the SCPI instruction. Not supported.
    if (argp != nullptr && rqstp != nullptr){
        qCDebug(vxiSer)<<"(device_docmd)Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp->cmd;
        resp.error = Vxi11::ErrorCode::NO_ERROR;
    }else{
        qCWarning(vxiSer)<<"(device_docmd)PARAMETER_ERROR";
        resp.error = Vxi11::ErrorCode::PARAMETER_ERROR;
    }

    resp.data_out.data_out_val = nullptr;
    resp.data_out.data_out_len = 0;
    return &resp;
}
Device_Error* destroy_link_1_svc(Device_Link* argp, struct svc_req* rqstp)
{
    static Device_Error resp;

    if (argp != nullptr && rqstp != nullptr){
        qCDebug(vxiSer)<<"(destroy_link)Ser_Id "<<rqstp->rq_xprt<<",Link "<<*argp;

        if (*argp > 0 && vxi11_server::instance().m_activeLinks.contains(*argp)){
            if (vxi11_server::instance().m_lockLinkId == *argp){
                vxi11_server::instance().m_lockLinkId = -1;
            }

            vxi11_server::instance().m_activeLinks.removeOne(*argp);
            vxi11_server::instance().m_newLinkId -= 1;
            resp.error = Vxi11::ErrorCode::NO_ERROR;
        }else{
            resp.error = Vxi11::ErrorCode::INVALID_LINK_IDENTIFIER;
        }
    }else{
        qCWarning(vxiSer)<<"(destroy_link)PARAMETER_ERROR";
        resp.error = Vxi11::ErrorCode::PARAMETER_ERROR;
    }

    return &resp;
}
Device_Error* create_intr_chan_1_svc(Device_RemoteFunc* argp, struct svc_req* rqstp)
{
    static Device_Error resp;

    // Repeats the SCPI instruction. Not supported.
    if (argp != nullptr && rqstp != nullptr){
        qCDebug(vxiSer)<<"(create_intr_chan)Ser_Id "<<rqstp->rq_xprt<<",Link "<<argp->hostAddr;
        resp.error = Vxi11::ErrorCode::NO_ERROR;
    }else{
        qCWarning(vxiSer)<<"(create_intr_chan)PARAMETER_ERROR";
        resp.error = Vxi11::ErrorCode::PARAMETER_ERROR;
    }

    return &resp;
}
Device_Error* destroy_intr_chan_1_svc(void* argp, struct svc_req* rqstp)
{
    static Device_Error resp;

    // Repeats the SCPI instruction. Not supported.
    if (argp != nullptr && rqstp != nullptr){
        qCDebug(vxiSer)<<"(destroy_intr_chan)Ser_Id "<<rqstp->rq_xprt;
        resp.error = Vxi11::ErrorCode::NO_ERROR;
    }else{
        qCWarning(vxiSer)<<"(destroy_intr_chan)PARAMETER_ERROR";
        resp.error = Vxi11::ErrorCode::PARAMETER_ERROR;
    }

    return &resp;
}
#ifdef __cplusplus
}
#endif

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
            qCDebug(vxiSer)<<"(vxi11_server)SOCKET Request: "<<m_readbuffer;

            if (ConfigManager::isInteRemote(InterSt::LAN_VXI)){
                if (m_readbuffer.startsWith("*") || m_readbuffer.startsWith(":")) {
                    m_respbuffer = ScpiManager::instance().processCommand(m_readbuffer);
                }else{
                    m_respbuffer = "SCPI cmd format Error!";
                }
            }else{
                m_respbuffer = "Device on Other inter_Remote mode!";
            }

            if (!m_respbuffer.isEmpty()){
                qCDebug(vxiSer)<<"(vxi11_server)SOCKET Response: "<<m_respbuffer;
                m_sockUdp->writeDatagram(m_respbuffer, sender, senderPort);
            }
        }}, Qt::DirectConnection);

    m_sockTcp = new QTcpServer(this);
    connect(m_sockTcp,&QTcpServer::newConnection,this,[this](){
        QTcpSocket* client = m_sockTcp->nextPendingConnection();
        if (m_clients.size() < 9) { // restrict connect number = 9
            connect(client, &QTcpSocket::errorOccurred,this, [client](QAbstractSocket::SocketError error){
                qCWarning(vxiSer)<<"(vxi11_server)"<< client->objectName()<<", "<<error<<client->errorString();
                }, Qt::DirectConnection);
            connect(client, &QTcpSocket::disconnected,this,[this,client](){
                qCDebug(vxiSer)<<"(vxi11_server)"<<client->objectName()<<" disconnected";
                m_clients.removeOne(client);
                client->deleteLater();
                }, Qt::DirectConnection);
            connect(client, &QTcpSocket::readyRead,this, [this, client](){
                m_readbuffer = client->readAll();
                qCDebug(vxiSer)<<"(vxi11_server)SOCKET Request: "<<m_readbuffer;

                if (ConfigManager::isInteRemote(InterSt::LAN_VXI)){
                    if (m_readbuffer.startsWith("*") || m_readbuffer.startsWith(":")) {
                        m_respbuffer = ScpiManager::instance().processCommand(m_readbuffer);
                    }else{
                        m_respbuffer = "SCPI cmd format Error!";
                    }
                }else{
                    m_respbuffer = "Device on Other inter_Remote mode!";
                }

                if (!m_respbuffer.isEmpty()){
                    qCDebug(vxiSer)<<"(vxi11_server)SOCKET Response: "<<m_respbuffer;
                    client->write(m_respbuffer);
                }}, Qt::DirectConnection);

            client->setObjectName(QString("%1:%2").arg(client->peerAddress().toString()).arg(client->peerPort()));
            qCDebug(vxiSer)<<"(vxi11_server)New Connect-Client: "<<client->objectName();
            m_clients.append(client);
            return;
        }

        client->disconnectFromHost();
        client->deleteLater();
        }, Qt::DirectConnection);

    if (m_sockTcp->listen(QHostAddress::Any, Vxi11::VXI_PORT)){
        if (m_sockUdp->bind(QHostAddress::Any, Vxi11::VXI_PORT, QUdpSocket::ShareAddress)) {
            // VXI-11 LAN Instrument
            pmap_unset(DEVICE_CORE,DEVICE_CORE_VERSION);
            pmap_unset(DEVICE_ASYNC,DEVICE_ASYNC_VERSION);

            m_transpUdp = svcudp_create(RPC_ANYSOCK);
            if (m_transpUdp != NULL) {
                svc_register(m_transpUdp , DEVICE_CORE, DEVICE_CORE_VERSION, device_core_1, IPPROTO_UDP);
                svc_register(m_transpUdp , DEVICE_ASYNC, DEVICE_ASYNC_VERSION, device_async_1, IPPROTO_UDP);
                qCDebug(vxiSer)<<"(vxi11_server)VXI-11 UDP "<<m_transpUdp<<",Port "<<m_transpUdp->xp_port;

                m_transpTcp = svctcp_create(RPC_ANYSOCK, 0, 0);
                if (m_transpTcp != NULL) {
                    svc_register(m_transpTcp, DEVICE_CORE, DEVICE_CORE_VERSION, device_core_1, IPPROTO_TCP);
                    svc_register(m_transpTcp, DEVICE_ASYNC, DEVICE_ASYNC_VERSION, device_async_1, IPPROTO_TCP);
                    qCDebug(vxiSer)<<"(vxi11_server)VXI-11 TCP "<<m_transpTcp<<",Port "<<m_transpTcp->xp_port;

                    m_rpcThread = new RpcThread();
                    m_rpcThread->setObjectName("vxiSubThread");
                    m_rpcThread->start();
                    return;
                }

                qCCritical(vxiSer)<<"(vxi11_server)cannot create TCP_Server!!!";
                return;
            }

            qCCritical(vxiSer)<<"(vxi11_server)cannot create UDP_Server!!!";
            return;
        }

        qCritical(vxiSer)<<"(vxi11_server)Fail bind_Socket UDP!!!";
        return;
    }

    qCritical(vxiSer)<<"(vxi11_server)Fail listen_Socket TCP!!!";
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
        m_rpcThread->quit();
        m_rpcThread->wait(3000);
        delete m_rpcThread;
        m_rpcThread = nullptr;
    }
}




