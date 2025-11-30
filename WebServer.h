#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>

class WebServer : public QObject
{
    Q_OBJECT

public:
    explicit WebServer(QObject* parent = nullptr);
    ~WebServer();

    bool startServer(quint16 port = 8080);
    void stopServer();
    QString serverAddress() const;
    bool isRunning() const;

signals:
    void serverStarted(const QString& address);
    void serverStopped();
    void serverError(const QString& error);
    void imageReceived(const QByteArray& data);

    void fileSaved(const QString& path);
private slots:
    void onNewConnection();
    void onReadyRead();

private:
    QByteArray buildUploadPage() const;
    QByteArray okPage() const;
    QByteArray badRequestPage(const QString& message) const;

    bool extractMultipartBody(const QByteArray& request, QByteArray& outBody) const;

private:
    QTcpServer* tcpServer;
    QTcpSocket* clientSocket;
    QString address;
    bool running;
};

#endif // WEBSERVER_H
