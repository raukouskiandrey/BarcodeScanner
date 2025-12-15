#include "WebServer.h"
#include <QHostAddress>
#include <QNetworkInterface>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QDateTime>


WebServer::WebServer(QObject* parent)
    : QObject(parent),
    tcpServer(std::make_unique<QTcpServer>(this))
{
    connect(tcpServer.get(), &QTcpServer::newConnection,
            this, &WebServer::onNewConnection);
}

WebServer::~WebServer()
{
    stopServer();
}

bool WebServer::startServer(quint16 port)
{
    if (running) return true;

    if (!tcpServer->listen(QHostAddress::Any, port)) {
        emit serverError(tcpServer->errorString());
        return false;
    }

    for (const QHostAddress& addr : QNetworkInterface::allAddresses()) {
        if (addr.protocol() == QAbstractSocket::IPv4Protocol &&
            addr != QHostAddress::LocalHost) {
            address = QString("http://%1:%2").arg(addr.toString()).arg(port);
            break;
        }
    }
    if (address.isEmpty()) {
        address = QString("http://127.0.0.1:%1").arg(port);
    }

    running = true;
    emit serverStarted(address);
    return true;
}

void WebServer::stopServer()
{
    if (!running) return;
    tcpServer->close();
    running = false;
    emit serverStopped();
}

QString WebServer::serverAddress() const
{
    return address;
}

bool WebServer::isRunning() const
{
    return running;
}

void WebServer::onNewConnection()
{
    QTcpSocket* newSocket = tcpServer->nextPendingConnection();

    // Для каждого соединения создаем отдельную структуру состояния
    connect(newSocket, &QTcpSocket::readyRead,
            this, [this, newSocket]() { this->onReadyRead(newSocket); });
    connect(newSocket, &QTcpSocket::disconnected,
            newSocket, &QTcpSocket::deleteLater);
}

void WebServer::onReadyRead(QTcpSocket* socket)
{
    if (!socket) return;

    // Читаем все доступные данные
    QByteArray data = socket->readAll();

    // Добавляем в буфер для этого сокета (можно хранить в map)
    // Для простоты будем обрабатывать сразу полный запрос
    static QMap<QTcpSocket*, QByteArray> buffers;
    buffers[socket].append(data);

    const QByteArray& requestBuffer = buffers[socket];
    // Пытаемся найти конец заголовков
    int headerEnd = requestBuffer.indexOf("\r\n\r\n");
    if (headerEnd == -1) {
        // Заголовки еще не полностью получены
        return;
    }

    // Извлекаем Content-Length если есть
    qint64 expectedLength = -1;
    int contentLengthPos = requestBuffer.indexOf("Content-Length:");
    if (contentLengthPos != -1 && contentLengthPos < headerEnd) {
        int end = requestBuffer.indexOf("\n", contentLengthPos);
        if (end != -1) {
            QByteArray lenLine = requestBuffer.mid(contentLengthPos, end - contentLengthPos);
            QList<QByteArray> parts = lenLine.split(' ');
            if (parts.size() >= 2) {
                expectedLength = parts.last().toLongLong();
            }
        }
    }

    // Проверяем, получено ли все тело
    qint64 bodyStart = headerEnd + 4;

    if (qint64 totalBodySize = requestBuffer.size() - bodyStart;
        expectedLength != -1 && totalBodySize < expectedLength) {
        // Тело еще не полностью получено
        return;
    }
    // Теперь у нас полный запрос
    const bool isGet = requestBuffer.startsWith("GET ");
    const bool isPost = requestBuffer.startsWith("POST ");

    QByteArray response;

    if (isGet) {
        QByteArray body = buildUploadPage();
        response = "HTTP/1.1 200 OK\r\n"
                   "Content-Type: text/html; charset=UTF-8\r\n"
                   "Connection: close\r\n"
                   "Content-Length: " + QByteArray::number(body.size()) + "\r\n\r\n" +
                   body;
    }
    else if (isPost) {
        QByteArray fileContent;
        if (extractMultipartBody(requestBuffer, fileContent)) {
            QString uploadDir = QDir::currentPath() + "/uploads";
            QDir().mkpath(uploadDir);

            QString savedPath = uploadDir + "/uploaded_" +
                                QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") +
                                ".jpg";

            if (QFile out(savedPath); out.open(QIODevice::WriteOnly)) {
                out.write(fileContent);
                out.close();
                emit fileSaved(savedPath);
            }

            QByteArray body = okPage();
            response = "HTTP/1.1 200 OK\r\n"
                       "Content-Type: text/html; charset=UTF-8\r\n"
                       "Connection: close\r\n"
                       "Content-Length: " + QByteArray::number(body.size()) + "\r\n\r\n" +
                       body;
        } else {
            QByteArray body = badRequestPage("Не удалось извлечь файл");
            response = "HTTP/1.1 400 Bad Request\r\n"
                       "Content-Type: text/html; charset=UTF-8\r\n"
                       "Connection: close\r\n"
                       "Content-Length: " + QByteArray::number(body.size()) + "\r\n\r\n" +
                       body;
        }
    }

    if (!response.isEmpty()) {
        socket->write(response);
    }

    socket->disconnectFromHost();
    buffers.remove(socket);
}

QByteArray WebServer::buildUploadPage() const
{
    const QByteArray html =
        "<!DOCTYPE html>"
        "<html lang='ru'><head><meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<title>📷 Загрузка фото штрих-кода</title>"
        "<style>"
        "body{font-family:system-ui,-apple-system,Segoe UI,Roboto,Arial,sans-serif;"
        "margin:2rem;max-width:720px}"
        "h1{font-size:1.6rem;margin-bottom:1rem}"
        "form{display:flex;gap:.75rem;align-items:center;flex-wrap:wrap}"
        "input[type=file]{flex:1}"
        "button{padding:.6rem 1rem;font-weight:600;border:1px solid #ccc;border-radius:.5rem;}"
        "</style></head><body>"
        "<h1>📷 Загрузка фото штрих-кода</h1>"
        "<form method='POST' enctype='multipart/form-data'>"
        "<input type='file' name='upload' accept='image/*' capture='environment'>"
        "<button type='submit'>📤 Отправить на обработку</button>"
        "</form>"
        "</body></html>";
    return html;
}

// Build OK page shown after successful upload
QByteArray WebServer::okPage() const
{
    const QByteArray html =
        "<!DOCTYPE html><html lang='ru'><head><meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<title>Файл получен</title>"
        "<style>body{font-family:system-ui;margin:2rem}h1{color:#0a7}</style>"
        "</head><body><h1>Файл получен</h1></body></html>";
    return html;
}

// Build error page
QByteArray WebServer::badRequestPage(const QString& message) const
{
    const QByteArray html =
        "<!DOCTYPE html><html lang='ru'><head><meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<title>Ошибка</title>"
        "<style>body{font-family:system-ui;margin:2rem}h1{color:#a00}</style>"
        "</head><body><h1>Ошибка: " + message.toUtf8() + "</h1></body></html>";
    return html;
}

bool WebServer::extractMultipartBody(const QByteArray& request, QByteArray& outBody) const
{
    // 1. Найти boundary
    int bpos = request.indexOf("boundary=");
    if (bpos == -1) return false;

    QString boundary = QString::fromUtf8(request.mid(bpos + 9));
    if (int eol = boundary.indexOf("\r\n"); eol != -1)
        boundary = boundary.left(eol);

    QByteArray boundaryPrefix = "--" + boundary.toUtf8();
    QByteArray partEnd        = "\r\n--" + boundary.toUtf8();
    QByteArray finalEnd       = "\r\n--" + boundary.toUtf8() + "--";

    // 2. Найти Content-Disposition с filename=
    int dispoPos = request.indexOf("Content-Disposition:");
    bool foundFilename = false;
    while (dispoPos != -1 && !foundFilename) {
        int lineEnd = request.indexOf("\r\n", dispoPos);
        if (lineEnd == -1) break;

        QByteArray dispoLine = request.mid(dispoPos, lineEnd - dispoPos);
        if (dispoLine.contains("form-data") && dispoLine.contains("filename=")) {
            foundFilename = true;
        } else {
            dispoPos = request.indexOf("Content-Disposition:", lineEnd);
        }
    }
    if (!foundFilename) return false;

    // 3. Найти конец заголовков этой части
    int headerEnd = request.indexOf("\r\n\r\n", dispoPos);
    if (headerEnd == -1) return false;
    int fileStart = headerEnd + 4;

    // 4. Найти конец файла
    int fileEnd = request.indexOf(finalEnd, fileStart);
    if (fileEnd == -1) fileEnd = request.indexOf(partEnd, fileStart);
    if (fileEnd == -1) fileEnd = request.indexOf(boundaryPrefix, fileStart);
    if (fileEnd == -1) fileEnd = request.size();

    // 5. Обрезать лишние \r\n перед boundary
    int trimmedEnd = fileEnd;
    if (trimmedEnd >= 2 && request.mid(trimmedEnd - 2, 2) == "\r\n") trimmedEnd -= 2;

    // 6. Извлечь бинарные данные
    outBody = request.mid(fileStart, trimmedEnd - fileStart);
    return !outBody.isEmpty();
}
