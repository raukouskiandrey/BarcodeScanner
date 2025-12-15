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
    ~WebServer() override;

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
    void onReadyRead(QTcpSocket* socket); // Измененная сигнатура

private:
    QByteArray buildUploadPage() const;
    QByteArray okPage() const;
    QByteArray badRequestPage(const QString& message) const;

    bool extractMultipartBody(const QByteArray& request, QByteArray& outBody) const;

    std::unique_ptr<QTcpServer> tcpServer;
    QTcpSocket* clientSocket = nullptr;
    QString address;
    bool running = false;
    QByteArray requestBuffer;
    qint64 expectedLength = -1;
};

#endif // WEBSERVER_H
