#pragma once
#include "channel/scpi_handle.h"
#include "auxiliary/battery_model.h"
#include "widgets/interaction/channelpage.h"
#include <QWebSocketServer>
#include <QLoggingCategory>
#include <QTcpServer>

class WebServerManager : public QObject
{
    Q_OBJECT

public:
    explicit WebServerManager(QObject *parent = nullptr);
    ~WebServerManager();

    channelpage* m_ChPage;
    void set_network(bool isstatic);
    std::shared_ptr<BatteryModelManager> m_BatteryManager;

private:
    void sendHttpResponse(QTcpSocket *client, const QByteArray &content,
             const QString &contentType = "text/html",int statusCode = 200);
    static const QHash<QString, QString> m_mimeTypes;
    static const QHash<QString, QString> m_staticFiles;
    void setupHTTPRoutes();

    using ApiHandler = std::function<void(QTcpSocket*)>;
    QHash<QString, ApiHandler> m_apiRoutes;
    QHash<QString, QByteArray> m_fileCache;
    QList<QTcpSocket*> m_clients;
    QByteArray m_readbuffer;

private:
    bool addModelFromNetwork(const QString &modelName, const QJsonArray &modelData);
    QJsonObject getModelsInfo() const;
    void setupWSRoutes();

    using WebHandler = std::function<void(QWebSocket*, const QJsonObject&)>;
    QHash<QString, WebHandler> m_webRoutes;
    QList<QWebSocket*> m_sockets;

    QThread* m_serverThread{nullptr};
    QTcpServer *m_httpServer{nullptr};
    QWebSocketServer *m_wsServer{nullptr};
};
