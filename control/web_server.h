#pragma once
#include <QHash>
#include <memory>
#include <QObject>

class channelpage;
class QTcpSocket;
class QTcpServer;
class QWebSocket;
class QWebSocketServer;

class WebServerManager : public QObject
{
    Q_OBJECT

private:
    static const QHash<QString, QString> m_staticFiles;
    static const QHash<QString, QString> m_mimeTypes;

    using ApiHandler = std::function<void(QTcpSocket*)>;
    QHash<QString, ApiHandler> m_apiRoutes;
    QHash<QString, QByteArray> m_fileCache;
    QByteArray m_readbuffer;

    QTcpServer *m_httpServer{nullptr};
    QList<QTcpSocket*> m_clients;

    void sendHttpResponse(QTcpSocket *client, const QByteArray &content,
             const QString &contentType = "text/html",int statusCode = 200);
    void setupHTTPRoutes();

public:
    explicit WebServerManager(QObject *parent = nullptr);
    ~WebServerManager();

    void set_network(bool isstatic);
    channelpage* m_ChPage;

private:
    QWebSocketServer *m_wsServer{nullptr};
    QList<QWebSocket*> m_sockets;

    using WebHandler = std::function<void(QWebSocket*, const QJsonObject&)>;
    QHash<QString, WebHandler> m_webRoutes;

    bool addModelFromNetwork(const QString &modelName, const QJsonArray &modelData);
    QJsonObject getModelsInfo() const;
    void setupWSRoutes();
};
