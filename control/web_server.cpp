#include "widgets/interaction/channelpage.h"
#include "auxiliary/battery_model.h"
#include "auxiliary/global_static.h"
#include "auxiliary/signal_bus.h"
#include "channel/scpi_handle.h"
#include "vxi_protocol.h"
#include "web_server.h"
#include <QWebSocketServer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QWebSocket>
#include <QTcpSocket>
#include <QTcpServer>
#include <QFileInfo>
#include <QtMath>
#include <QFile>

const QHash<QString, QString> WebServerManager::m_staticFiles =
{
    {"/",                     ":/web/html/index.html"},
    {"/index.html",           ":/web/html/index.html"},
    {"/channels.html",        ":/web/html/channels.html"},
    {"/import.html",          ":/web/html/import.html"},
    {"/js/index.js",          ":/web/js/index.js"},
    {"/js/channels.js",       ":/web/js/channels.js"},
    {"/js/import.js",         ":/web/js/import.js"},
    {"/js/header.js",         ":/web/js/header.js"},
    {"/css/style.css",        ":/web/css/style.css"},
    {"/css/index.css",        ":/web/css/index.css"},
    {"/css/channels.css",     ":/web/css/channels.css"},
    {"/css/import.css",       ":/web/css/import.css"},
    {"/css/global.css",       ":/web/css/global.css"},
    {"/icon/leacesylogo.png", ":/web/icon/leacesylogo.png"},
    {"/icon/leacesyicon.png", ":/web/icon/leacesyicon.png"}
};
const QHash<QString, QString> WebServerManager::m_mimeTypes =
{
    {"html",                  "text/html"},
    {"css",                   "text/css"},
    {"js",                    "application/javascript"},
    {"json",                  "application/json"},
    {"png",                   "image/png"},
    {"jpg",                   "image/jpeg"},
    {"svg",                   "image/svg+xml"},
    {"ico",                   "image/x-icon"},
    {"txt",                   "text/plain"},
    {"xml",                   "application/xml"},
    {"pdf",                   "application/pdf"},
};

void WebServerManager::sendHttpResponse(QTcpSocket *client, const QByteArray &content,const QString &contentType, int statusCode)
{
    QString statusText;
    switch (statusCode) {
        case 200: statusText = "OK"; break;
        case 204: statusText = "No Content"; break;
        case 400: statusText = "Bad Request"; break;
        case 404: statusText = "Not Found"; break;
        case 405: statusText = "Method Not Allowed"; break;
        case 500: statusText = "Internal Server Error"; break;
        default: statusText = "OK"; break;
    }

    QString header = QString(
        "HTTP/1.1 %1 %2\r\n"
        "Content-Type: %3; charset=utf-8\r\n"
        "Content-Length: %4\r\n"
        "Connection: close\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "\r\n"
    ).arg(QString::number(statusCode), statusText, contentType, QString::number(content.size()));

    client->write(header.toUtf8());
    client->write(content);
    //client->close(); // http1.0 long connection
}
void WebServerManager::setupHTTPRoutes()
{
    m_apiRoutes["/api/device/info"] = [this](QTcpSocket* client)
    {
        QJsonObject response;
        response["model"] = Device::ModelNumber;
        response["serial"] = Device::Identifying;
        response["software"] = Device::SoftVer;
        response["hardware"] = Device::HardVer;

        QByteArray jsonData = QJsonDocument(response).toJson();
        sendHttpResponse(client, jsonData, "application/json");
    };
    m_apiRoutes["/api/scpi_commands"] = [this](QTcpSocket* client)
    {
        QJsonObject response; QJsonArray commands;
        for (int i = 0; ScpiManager::m_scpiCommands[i].pattern != nullptr; ++i) {
            if (strlen(ScpiManager::m_scpiCommands[i].pattern) > 0) {
                commands.append(QString::fromLatin1(ScpiManager::m_scpiCommands[i].pattern));
            }
        }
        response["commands"] = commands;

        QByteArray jsonData = QJsonDocument(response).toJson();
        sendHttpResponse(client, jsonData, "application/json");
    };
    m_apiRoutes["/api/channels"] = [this](QTcpSocket* client)
    {
        if (m_ChPage){
            QJsonObject response;
            response["channels"] = m_ChPage->getAllChannelsData();

            QByteArray jsonData = QJsonDocument(response).toJson();
            sendHttpResponse(client, jsonData, "application/json");
        }
    };
    m_apiRoutes["/api/models"] = [this](QTcpSocket* client)
    {
        QJsonObject response = getModelsInfo();
        QByteArray jsonData = QJsonDocument(response).toJson();
        sendHttpResponse(client, jsonData, "application/json");
    };
}

WebServerManager::WebServerManager(QObject *parent) : QObject(parent)
{
    m_httpServer = new QTcpServer(this);
    connect(m_httpServer, &QTcpServer::newConnection,this,[this](){
        QTcpSocket* client = m_httpServer->nextPendingConnection();
        if (m_clients.size() < 9) { // restrict connect number = 9
            client->setObjectName(QString("%1:%2").arg(client->peerAddress().toString()).arg(client->peerPort()));
            qCDebug(webSer)<<"[WebServerManager]:New HTTP client: "<< client->objectName();

            connect(client, &QTcpSocket::errorOccurred,this, [client](QAbstractSocket::SocketError error){
                qCWarning(webSer)<<"[WebServerManager]:("<< client->objectName()<<")"<<error<<client->errorString();
                }, Qt::DirectConnection);
            connect(client, &QTcpSocket::disconnected,this, [this, client](){
                qCDebug(webSer)<<"[WebServerManager]: "<<client->objectName()<<" disconnected";
                m_clients.removeOne(client);
                client->deleteLater();
                }, Qt::DirectConnection);
            connect(client, &QTcpSocket::readyRead,this, [this, client](){
                m_readbuffer = client->readAll();
                if (m_readbuffer.contains("\r\n\r\n")) {
                    QString requestLine = QString::fromUtf8(m_readbuffer).split("\r\n").first();
                    QStringList parts = requestLine.split(" ");

                    if (parts.size() >= 3) {
                        QString path = QUrl::fromPercentEncoding(parts[1].toUtf8());
                        qCDebug(webSer)<<"[handleHttpRequest]:HTTP request: "<< parts[0] << path;

                        if (m_apiRoutes.contains(path)) {
                            m_apiRoutes[path](client);
                            return;
                        }

                        if (m_staticFiles.contains(path)) {
                            QString resourcePath = m_staticFiles[path];
                            if (!m_fileCache.contains(path)) {
                                QFile file(resourcePath);
                                if (!file.open(QIODevice::ReadOnly)) {
                                    qCWarning(webSer)<<"[handleHttpRequest]: open web file: "<<resourcePath<<"failed!";
                                    sendHttpResponse(client, "500 Internal Server Error", "text/plain", 500);
                                    return;
                                }

                                m_fileCache[path] = file.readAll();
                            }

                            QString contentType = m_mimeTypes[QFileInfo(resourcePath).suffix()];
                            sendHttpResponse(client, m_fileCache[path], contentType);
                            return;
                        }

                        qCWarning(webSer)<<"[handleHttpRequest]: request route method not exist!";
                        sendHttpResponse(client, "404 Not Found", "text/plain", 404);
                        return;
                    }
                }

                sendHttpResponse(client, "Please access using a web browser.", "text/plain");
                qCWarning(webSer)<<"[handleHttpRequest]: request format failed!";
                }, Qt::DirectConnection);

            m_clients.append(client);
            return;
        }

        client->disconnectFromHost();
        client->deleteLater();
        },Qt::DirectConnection);

    if (m_httpServer->listen(QHostAddress::Any, Vxi11::HTTP_PORT)) {
        setupHTTPRoutes();

        m_wsServer = new QWebSocketServer("Leacesy",QWebSocketServer::NonSecureMode, this);
        connect(m_wsServer, &QWebSocketServer::newConnection,this, [this](){
            QWebSocket* socket = m_wsServer->nextPendingConnection();
            if (m_sockets.size() < 9) { // restrict connect number = 9
                socket->setObjectName(QString("%1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort()));
                qCDebug(webSer)<<"[WebServerManager]:New WebSocket client: "<< socket->objectName();

                connect(socket,QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),this, [socket](QAbstractSocket::SocketError error){
                    qCWarning(webSer)<<"[WebServerManager]:("<<socket->objectName()<<")"<< error <<socket->errorString();
                    }, Qt::DirectConnection);
                connect(socket, &QWebSocket::disconnected,this, [this, socket](){
                    qCDebug(webSer)<<"[WebServerManager]: "<<socket->objectName()<<" disconnected";
                    m_sockets.removeOne(socket);
                    socket->deleteLater();
                    }, Qt::DirectConnection);
                connect(socket, &QWebSocket::textMessageReceived,this, [this, socket](const QString &message){
                    qCDebug(webSer)<<"[onWsTextMessageReceived]:WEB request: "<<message;
                    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
                    if (!doc.isNull() && doc.isObject()) {
                        QJsonObject obj = doc.object();
                        QString type = obj["type"].toString();
                        if (m_webRoutes.contains(type)) {
                            switch (ConfigManager::control_Mode.load()) {
                                case InterSt::LOCAL:emit ChannelBus::instance().switchRemote(InterSt::LAN_WEB);// fall through
                                case InterSt::LAN_WEB:m_webRoutes[type](socket,obj);break;
                                default:QJsonObject response;
                                        response["type"] = "scpi_response";
                                        response["result"] = "Other interfaces of the instrument are currently in operation";
                                        qCDebug(webSer)<<"[setupRoutes]:Currently in an alternative remote mode";
                                        socket->sendTextMessage(QJsonDocument(response).toJson());
                            }

                            return;
                        }

                        qCWarning(webSer)<<"[onWsTextMessageReceived]: request route method not exist!";
                    }}, Qt::DirectConnection);

                m_sockets.append(socket);
                return;
            }

            socket->disconnect();
            socket->deleteLater();
            },Qt::DirectConnection);

        if (m_wsServer->listen(QHostAddress::Any, Vxi11::WEB_PORT)) {
            setupWSRoutes();
            return;
        }

        qCCritical(webSer)<<"[startServer]: m_wsServer listen failed!";
        return;
    }

    qCCritical(webSer)<<"[startServer]: m_httpServer listen failed!";
    return;
}
WebServerManager::~WebServerManager()
{
    for (QTcpSocket* client : qAsConst(m_clients)) {
        if (client) {
            client->disconnect(this);
            client->deleteLater();
        }
    }
    m_clients.clear();

    for (QWebSocket* socket : qAsConst(m_sockets)) {
        if (socket) {
            socket->disconnect(this);
            socket->deleteLater();
        }
    }
    m_sockets.clear();
}

void WebServerManager::set_network(bool isstatic)
{
    QFile file("/etc/network/interfaces");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        // WriteOnly -> Clear the original content
        QString configContent;
        if(isstatic){
            configContent =
                "# interface file auto-generated by buildroot\n"
                "auto lo\n"
                "iface lo inet loopback\n\n"
                "auto eth0\n"
                "iface eth0 inet static\n"
                "    address " + ConfigManager::IP + "\n"
                "    netmask " + ConfigManager::SM + "\n"
                "    gateway " + ConfigManager::Gate;
        }else{
            configContent =
                "# interface file auto-generated by buildroot\n"
                "auto lo\n"
                "iface lo inet loopback\n\n"
                "auto eth0\n"
                "iface eth0 inet dhcp";
        }

        QTextStream out(&file);
        out << configContent; file.close();
        QString restartCmd = "/etc/init.d/S40network restart";
        if (system(restartCmd.toStdString().c_str()) == 0) {
            emit ChannelBus::instance().interAcUpdate();
            return;
        }

        qCWarning(webSer)<< "[set_network]:/etc/init.d/S40network restart failed!";
        return;
    }

    qCWarning(webSer)<< "[set_network]:Cannot open interfaces file for writing";
    return;
}

bool WebServerManager::addModelFromNetwork(const QString &modelName, const QJsonArray &modelData)
{
    if (!BatteryModelManager::instance().m_models.contains(modelName)) {
         QVector<BatteryDataPoint> pts;

        for (const auto &item : modelData) {
            if (!item.isNull() && item.isObject()) {
                QJsonObject pointObj = item.toObject();

                if (pointObj.contains("soc") && pointObj.contains("ocv") && pointObj.contains("imp")) {
                    BatteryDataPoint point;
                    point.soc = pointObj["soc"].toDouble();
                    point.ocv = pointObj["ocv"].toDouble();
                    point.imp = pointObj["imp"].toDouble();
                    pts.append(point);
                    continue;
                }

                qCWarning(webSer)<< "[addModelFromNetwork]:Parameter lack error!";
                return false;
            }

            qCWarning(webSer)<< "[addModelFromNetwork]:Parameter model format error!";
            return false;
        }

        // model->name = modelName;   saveModel to set model->name
        return BatteryModelManager::instance().saveModel(pts, modelName);
    }

    qCWarning(webSer)<<"[addModelFromNetwork]:model incorrect!";
    return false;
}
QJsonObject  WebServerManager::getModelsInfo() const
{
    QJsonObject result;
    QJsonArray modelsArray;

    for (auto it = BatteryModelManager::instance().m_models.begin(); it != BatteryModelManager::instance().m_models.end(); ++it) {
        QJsonObject modelInfo;
        modelInfo["name"] = it.key();

        QJsonArray dataArray;
        const auto &model = it.value();

        for (const auto &point : model) {
            QJsonObject pointObj;
            pointObj["soc"] = std::round(point.soc * 10.0) / 10.0;
            pointObj["ocv"] = std::round(point.ocv * 100.0) / 100.0;
            pointObj["esr"] = std::round(point.imp * 1000.0) / 1000.0;
            dataArray.append(pointObj);
        }

        modelInfo["data"] = dataArray;
        modelsArray.append(modelInfo);
    }

    result["models"] = modelsArray;
    result["status"] = "success";
    return result;
}
void WebServerManager::setupWSRoutes()
{
    m_webRoutes["scpi_command"] = [](QWebSocket* socket, const QJsonObject& obj)
    {
        QByteArray cmd = (obj["command"].toString()+"\n").toUtf8();
        qCDebug(webSer)<<"[setupRoutes]:SCPI command received:" << cmd;
        QByteArray res = ScpiManager::instance().processCommand(cmd);

        if (!res.isEmpty()){
            QJsonObject response;
            response["type"] = "scpi_response";
            response["result"] = QString::fromUtf8(res);
            socket->sendTextMessage(QJsonDocument(response).toJson());
        }
    };
    m_webRoutes["channels_update"] = [this](QWebSocket* socket, const QJsonObject&)
    {
        if (m_ChPage){
            QJsonObject channelData;
            channelData["type"] = "channels_response";
            channelData["channels"] = m_ChPage->getAllChannelsData();
            socket->sendTextMessage(QJsonDocument(channelData).toJson());
        }
    };
    m_webRoutes["model_upload"] = [this](QWebSocket* socket, const QJsonObject& obj)
    {
        QJsonObject response;
        QJsonObject content = obj["content"].toObject();
        QString modelName = content["name"].toString();
        QJsonArray modelData = content["data"].toArray();

        if (addModelFromNetwork(modelName,modelData)) {
            response = getModelsInfo();
            response["type"] = "model_sync";
            emit ChannelBus::instance().interModelUpdate();
            socket->sendTextMessage(QJsonDocument(response).toJson());
        } else {
            response["type"] = "error";
            response["message"] = QString("Failed to save model \"%1\"").arg(modelName);
            socket->sendTextMessage(QJsonDocument(response).toJson());
        }
    };
    m_webRoutes["model_delete"] = [this](QWebSocket* socket, const QJsonObject& obj)
    {
        QJsonObject response;
        QString modelName = obj["name"].toString();

        if (m_ChPage && !m_ChPage->existBatteryOutPut() && BatteryModelManager::instance().removeModel(modelName)) {
            response = getModelsInfo();
            response["type"] = "model_sync";
            emit ChannelBus::instance().interModelUpdate();
            socket->sendTextMessage(QJsonDocument(response).toJson());
        }else {
            response["type"] = "error";
            response["message"] = QString("Now Work in Bettary mode OR Failed to delete model \"%1\"").arg(modelName);
            socket->sendTextMessage(QJsonDocument(response).toJson());
        }
    };
}
