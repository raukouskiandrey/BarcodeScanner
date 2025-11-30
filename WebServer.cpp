#include "WebServer.h"
#include <QHostAddress>
#include <QNetworkInterface>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>


WebServer::WebServer(QObject* parent)
    : QObject(parent),
    tcpServer(new QTcpServer(this)),
    clientSocket(nullptr),
    running(false)
{
    connect(tcpServer, &QTcpServer::newConnection,
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
    clientSocket = tcpServer->nextPendingConnection();
    connect(clientSocket, &QTcpSocket::readyRead,
            this, &WebServer::onReadyRead);
}

void WebServer::onReadyRead()
{
    if (!clientSocket) return;

    QByteArray requestData = clientSocket->readAll();

    // Определяем метод
    const bool isGet  = requestData.startsWith("GET ");
    const bool isPost = requestData.startsWith("POST ");

    QByteArray response;

    if (isGet) {
        // Отдаём HTML‑страницу с формой загрузки
        QByteArray body = buildUploadPage();
        response = "HTTP/1.1 200 OK\r\n"
                   "Content-Type: text/html; charset=UTF-8\r\n"
                   "Connection: close\r\n"
                   "Content-Length: " + QByteArray::number(body.size()) + "\r\n\r\n" +
                   body;
    }
    else if (isPost) {
        // Проверяем, что это multipart/form-data
        if (!requestData.contains("Content-Type: multipart/form-data")) {
            QByteArray body = badRequestPage("Ожидается multipart/form-data");
            response = "HTTP/1.1 400 Bad Request\r\n"
                       "Content-Type: text/html; charset=UTF-8\r\n"
                       "Connection: close\r\n"
                       "Content-Length: " + QByteArray::number(body.size()) + "\r\n\r\n" +
                       body;
        } else {
            QByteArray fileContent;
            if (extractMultipartBody(requestData, fileContent)) {
                // --- Сохраняем файл ---
                QString uploadDir = QDir::currentPath() + "/uploads";
                if (!QDir().mkpath(uploadDir)) {
                    emit serverError("❌ Не удалось создать папку: " + uploadDir);
                } else {
                    // Уникальное имя файла по времени
                    QString savedPath = uploadDir + "/uploaded_" +
                                        QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") +
                                        ".jpg"; // можно заменить на .png

                    QFile out(savedPath);
                    if (!out.open(QIODevice::WriteOnly)) {
                        emit serverError("❌ Не удалось открыть файл для записи: " + savedPath);
                    } else {
                        out.write(fileContent);
                        out.close();

                        // Уведомляем MainWindow
                        emit fileSaved(savedPath);
                    }
                }

                // --- Ответ клиенту ---
                QByteArray body = okPage();
                response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/html; charset=UTF-8\r\n"
                           "Connection: close\r\n"
                           "Content-Length: " + QByteArray::number(body.size()) + "\r\n\r\n" +
                           body;
            } else {
                QByteArray body = badRequestPage("Не удалось извлечь файл из запроса");
                response = "HTTP/1.1 400 Bad Request\r\n"
                           "Content-Type: text/html; charset=UTF-8\r\n"
                           "Connection: close\r\n"
                           "Content-Length: " + QByteArray::number(body.size()) + "\r\n\r\n" +
                           body;
            }
        }
    }
    else {
        // Метод не поддерживается
        QByteArray body = badRequestPage("Метод не поддерживается");
        response = "HTTP/1.1 405 Method Not Allowed\r\n"
                   "Content-Type: text/html; charset=UTF-8\r\n"
                   "Connection: close\r\n"
                   "Content-Length: " + QByteArray::number(body.size()) + "\r\n\r\n" +
                   body;
    }

    clientSocket->write(response);
    clientSocket->disconnectFromHost();
}

// Build HTML upload page with title and form
QByteArray WebServer::buildUploadPage() const
{
    const QByteArray html =
        "<!DOCTYPE html>"
        "<html lang='ru'><head><meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<title>📷 Barcode Photo Upload</title>"
        "<style>"
        "body{font-family:system-ui,-apple-system,Segoe UI,Roboto,Arial,sans-serif;"
        "margin:2rem;max-width:720px}"
        "h1{font-size:1.6rem;margin-bottom:1rem}"
        "form{display:flex;gap:.75rem;align-items:center;flex-wrap:wrap}"
        "input[type=file]{flex:1}"
        "button{padding:.6rem 1rem;font-weight:600;border:1px solid #ccc;border-radius:.5rem;}"
        "</style></head><body>"
        "<h1>📷 Barcode Photo Upload</h1>"
        "<form method='POST' enctype='multipart/form-data'>"
        "<input type='file' name='upload' accept='image/*' capture='environment'>"
        "<button type='submit'>📤 Send for processing</button>"
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
    int eol = boundary.indexOf("\r\n");
    if (eol != -1) boundary = boundary.left(eol);
    boundary = boundary.trimmed();

    QByteArray boundaryPrefix = "--" + boundary.toUtf8();
    QByteArray partEnd        = "\r\n--" + boundary.toUtf8();
    QByteArray finalEnd       = "\r\n--" + boundary.toUtf8() + "--";

    // 2. Найти Content-Disposition с filename=
    int dispoPos = request.indexOf("Content-Disposition:");
    while (dispoPos != -1) {
        int lineEnd = request.indexOf("\r\n", dispoPos);
        if (lineEnd == -1) break;

        QByteArray dispoLine = request.mid(dispoPos, lineEnd - dispoPos);
        if (dispoLine.contains("form-data") && dispoLine.contains("filename=")) {
            break;
        }
        dispoPos = request.indexOf("Content-Disposition:", lineEnd);
    }
    if (dispoPos == -1) return false;

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

