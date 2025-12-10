#ifndef BARCODESERVER_H
#define BARCODESERVER_H
#include <QHash>
#include <QObject>


class QTcpServer;
class QTcpSocket;

class BarcodeServer : public QObject
{
    Q_OBJECT

public:
    explicit BarcodeServer(QObject *parent = nullptr);
    ~BarcodeServer();

    bool startServer(quint16 port = 8080);
    void stopServer();
    bool isRunning() const;
    QString getServerUrl() const;



private:
    QHash<QTcpSocket*, QByteArray> m_buffers;


signals:
    void imageReceived(const QPixmap &pixmap);


signals:
    void serverStarted(bool success);
    void serverStopped();
    void imageReceived(const QString &filePath);
    void logMessage(const QString &message);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();

private:
    void processHttpRequest(QTcpSocket *client, const QByteArray &request);
    void sendHttpResponse(QTcpSocket *client, const QByteArray &content,
                          const QString &contentType = "text/html", int statusCode = 200);
    QString generateHtmlForm();

    QTcpServer *m_tcpServer;
    QList<QTcpSocket*> m_clients;
    quint16 m_port;
    bool m_isRunning;
    QString m_uploadDir;
};

#endif // BARCODESERVER_H
