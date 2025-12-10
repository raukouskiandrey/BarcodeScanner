#include "barcodeserver.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QImage>
#include <QPixmap>
#include <QRegularExpression>


BarcodeServer::BarcodeServer(QObject *parent)
    : QObject(parent), m_tcpServer(nullptr), m_port(8080), m_isRunning(false)
{
    m_uploadDir = QDir::currentPath() + "/uploads";
    QDir().mkpath(m_uploadDir);
}

BarcodeServer::~BarcodeServer()
{
    stopServer();
}

bool BarcodeServer::startServer(quint16 port)
{
    if (m_isRunning) {
        emit logMessage("Server already running");
        return true;
    }

    m_port = port;
    m_tcpServer = new QTcpServer(this);

    if (!m_tcpServer->listen(QHostAddress::Any, m_port)) {
        emit logMessage("Server start error: " + m_tcpServer->errorString());
        emit serverStarted(false);
        return false;
    }

    connect(m_tcpServer, &QTcpServer::newConnection, this, &BarcodeServer::onNewConnection);

    m_isRunning = true;
    emit logMessage("Server started on port " + QString::number(m_port));

    // Show IP for connection
    QString ipAddress;
    for (const QHostAddress &address : QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol &&
            address != QHostAddress::LocalHost) {
            ipAddress = address.toString();
            break;
        }
    }

    if (ipAddress.isEmpty()) {
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    }

    emit logMessage("URL for connection: http://" + ipAddress + ":" + QString::number(m_port));
    emit serverStarted(true);
    return true;
}

void BarcodeServer::stopServer()
{
    if (!m_isRunning) return;

    for (QTcpSocket *client : m_clients) {
        client->close();
        client->deleteLater();
    }
    m_clients.clear();

    if (m_tcpServer) {
        m_tcpServer->close();
        m_tcpServer->deleteLater();
        m_tcpServer = nullptr;
    }

    m_isRunning = false;
    emit logMessage("Server stopped");
    emit serverStopped();
}

bool BarcodeServer::isRunning() const
{
    return m_isRunning;
}

QString BarcodeServer::getServerUrl() const
{
    return "http://localhost:" + QString::number(m_port);
}

void BarcodeServer::onNewConnection()
{
    QTcpSocket *client = m_tcpServer->nextPendingConnection();
    if (!client) return;

    m_clients.append(client);
    connect(client, &QTcpSocket::readyRead, this, &BarcodeServer::onReadyRead);
    connect(client, &QTcpSocket::disconnected, this, &BarcodeServer::onClientDisconnected);

    emit logMessage("New client connected");
}

void BarcodeServer::onReadyRead()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    // дополняем буфер
    m_buffers[client].append(client->readAll());

    QByteArray &buffer = m_buffers[client];

    // ищем конец заголовков
    int headerEnd = buffer.indexOf("\r\n\r\n");
    if (headerEnd == -1) return; // заголовки ещё не дошли

    // вытаскиваем Content-Length
    QString headers = QString::fromUtf8(buffer.left(headerEnd));
    QRegularExpression re("Content-Length: (\\d+)");
    QRegularExpressionMatch m = re.match(headers);
    if (!m.hasMatch()) return;

    int contentLength = m.captured(1).toInt();
    int totalSize = headerEnd + 4 + contentLength;

    // ждём пока весь запрос придёт
    if (buffer.size() < totalSize) return;

    // теперь у нас полный запрос
    QByteArray fullRequest = buffer.left(totalSize);
    processHttpRequest(client, fullRequest);

    // очищаем использованное
    buffer.remove(0, totalSize);
}


void BarcodeServer::onClientDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    m_clients.removeAll(client);
    client->deleteLater();
    emit logMessage("Client disconnected");
    m_buffers.remove(client);
}

void BarcodeServer::processHttpRequest(QTcpSocket *client, const QByteArray &request)
{
    const QString requestStr = QString::fromUtf8(request);

    // --- обработка GET ---
    if (requestStr.startsWith("GET / ")) {
        const QString html = generateHtmlForm();
        sendHttpResponse(client, html.toUtf8());
        emit logMessage("Sent upload form");
        return;
    }

    // --- обработка POST /upload ---
    if (requestStr.contains("POST") && requestStr.contains("/upload")) {
        emit logMessage("Получен запрос на загрузку изображения");

        // 1) найти boundary из заголовка Content-Type
        int bpos = requestStr.indexOf("boundary=");
        if (bpos == -1) {
            sendHttpResponse(client, "No boundary found", "text/plain", 400);
            emit logMessage("❌ Boundary not found");
            return;
        }
        QString boundary = requestStr.mid(bpos + 9);
        int eol = boundary.indexOf("\r\n");
        if (eol != -1) boundary = boundary.left(eol);
        boundary = boundary.trimmed();

        const QByteArray boundaryPrefix = "--" + boundary.toUtf8();
        const QByteArray partEnd = "\r\n--" + boundary.toUtf8();      // следующий блок
        const QByteArray finalEnd = "\r\n--" + boundary.toUtf8() + "--"; // финальный маркер
        emit logMessage("Boundary: " + boundary);

        // 2) найти часть с файлом: ищем Content-Disposition с filename=
        int dispoPos = request.indexOf("Content-Disposition:");
        while (dispoPos != -1) {
            // конец заголовка Content-Disposition строки
            int lineEnd = request.indexOf("\r\n", dispoPos);
            if (lineEnd == -1) break;

            QByteArray dispoLine = request.mid(dispoPos, lineEnd - dispoPos);
            if (dispoLine.contains("form-data") && dispoLine.contains("filename=")) {
                // нашли нужную часть
                break;
            }
            // перейти к следующему вхождению
            dispoPos = request.indexOf("Content-Disposition:", lineEnd);
        }

        if (dispoPos == -1) {
            sendHttpResponse(client, "No file part found", "text/plain", 400);
            emit logMessage("❌ Не найдена файловая часть");
            return;
        }

        // 3) извлечь имя файла (не обязательно, но полезно для лога)
        int filenamePos = request.indexOf("filename=", dispoPos);
        QString filenameLog;
        if (filenamePos != -1) {
            int quoteStart = request.indexOf('"', filenamePos);
            int quoteEnd   = (quoteStart != -1) ? request.indexOf('"', quoteStart + 1) : -1;
            if (quoteStart != -1 && quoteEnd != -1) {
                filenameLog = QString::fromUtf8(request.mid(quoteStart + 1, quoteEnd - quoteStart - 1));
                emit logMessage("📁 Имя файла: " + filenameLog);
            }
        }

        // 4) конец заголовков этой части: \r\n\r\n
        int headerEnd = request.indexOf("\r\n\r\n", dispoPos);
        if (headerEnd == -1) {
            sendHttpResponse(client, "No header end for part", "text/plain", 400);
            emit logMessage("❌ Не найден конец заголовков части");
            return;
        }
        int fileStart = headerEnd + 4;
        emit logMessage(QString("📍 Начало бинарных данных: %1").arg(fileStart));

        // 5) конец файла — перед следующим маркером boundary
        // пробуем сначала финальный, затем обычный, затем просто префикс (fallback)
        int fileEnd = request.indexOf(finalEnd, fileStart);
        if (fileEnd == -1) fileEnd = request.indexOf(partEnd, fileStart);
        if (fileEnd == -1) fileEnd = request.indexOf(boundaryPrefix, fileStart);
        if (fileEnd == -1) {
            // если не нашли, считаем до конца буфера
            fileEnd = request.size();
            emit logMessage("⚠️ Маркер конца части не найден, используем конец запроса");
        }

        // если конец найден по маркеру, убрать возможный завершающий \r\n перед ним
        // чтобы не включать лишние символы в файл
        int trimmedEnd = fileEnd;
        // убрать CRLF непосредственно перед маркером, если они есть
        if (trimmedEnd >= 2 && request.mid(trimmedEnd - 2, 2) == "\r\n") trimmedEnd -= 2;

        QByteArray fileContent = request.mid(fileStart, trimmedEnd - fileStart);
        emit logMessage(QString("📊 Размер данных изображения: %1 байт").arg(fileContent.size()));

        // 6) сохранить файл на диск
        const QString ext = ".jpg"; // по умолчанию JPEG; можно определить из Content-Type при желании
        const QString savedPath = m_uploadDir + "/uploaded_" +
                                  QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ext;

        QFile out(savedPath);
        if (!out.open(QIODevice::WriteOnly)) {
            sendHttpResponse(client, "Failed to open file for writing", "text/plain", 500);
            emit logMessage("❌ Не удалось открыть файл для записи: " + savedPath);
            return;
        }
        out.write(fileContent);
        out.close();
        emit logMessage("✅ Изображение сохранено: " + savedPath);

        // 7) автоматически загрузить сохранённое изображение и передать дальше
        QImage img(savedPath);
        if (!img.isNull()) {
            QPixmap pixmap = QPixmap::fromImage(img);
            emit imageReceived(pixmap);
            sendHttpResponse(client, "✅ Image saved and processed!", "text/plain", 200);
            emit logMessage(QString("✅ Изображение валидно: %1x%2").arg(img.width()).arg(img.height()));
        } else {
            sendHttpResponse(client, "❌ Failed to load saved image", "text/plain", 500);
            emit logMessage("❌ Ошибка: не удалось загрузить сохранённое изображение (возможно, обрезанные данные)");
        }
        return;
    }

    // --- обработка остальных маршрутов ---
    const QString notFoundMessage = "404 - Page not found";
    sendHttpResponse(client, notFoundMessage.toUtf8(), "text/plain", 404);
}

void BarcodeServer::sendHttpResponse(QTcpSocket *client, const QByteArray &content,
                                     const QString &contentType, int statusCode)
{
    QString response = QString(
                           "HTTP/1.1 %1 OK\r\n"
                           "Content-Type: %2; charset=utf-8\r\n"
                           "Content-Length: %3\r\n"
                           "Connection: close\r\n"
                           "Access-Control-Allow-Origin: *\r\n"
                           "\r\n"
                           ).arg(statusCode).arg(contentType).arg(content.length());

    client->write(response.toUtf8());
    client->write(content);
    client->close();
}

QString BarcodeServer::generateHtmlForm()
{
    return QString(
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "    <title>Barcode Upload</title>"
        "    <meta charset='utf-8'>"
        "    <style>"
        "        body { font-family: Arial; margin: 40px; background: #f5f5f5; }"
        "        .container { max-width: 500px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }"
        "        h2 { color: #333; text-align: center; }"
        "        .form-group { margin-bottom: 20px; }"
        "        input[type='file'] { padding: 15px; border: 2px dashed #ccc; width: 100%; border-radius: 5px; }"
        "        input[type='submit'] { background: #007cba; color: white; padding: 15px 30px; border: none; border-radius: 5px; cursor: pointer; width: 100%; font-size: 16px; }"
        "        input[type='submit']:hover { background: #005a87; }"
        "        .status { margin-top: 20px; padding: 15px; border-radius: 5px; text-align: center; }"
        "        .success { background: #d4edda; color: #155724; }"
        "        .info { background: #d1ecf1; color: #0c5460; }"
        "    </style>"
        "</head>"
        "<body>"
        "    <div class='container'>"
        "        <h2>📷 Barcode Photo Upload</h2>"
        "        <div class='status info'>"
        "            Server is running! You can test the upload."
        "        </div>"
        "        <form action='/upload' method='post' enctype='multipart/form-data'>"
        "            <div class='form-group'>"
        "                <input type='file' name='barcode_image' accept='image/*' required>"
        "            </div>"
        "            <div class='form-group'>"
        "                <input type='submit' value='📤 Send for processing'>"
        "            </div>"
        "        </form>"
        "        <div id='status'></div>"
        "    </div>"
        "    <script>"
        "        document.querySelector('form').addEventListener('submit', function(e) {"
        "            document.getElementById('status').innerHTML = '<div class=\"status success\">Sending file to server...</div>';"
        "        });"
        "    </script>"
        "</body>"
        "</html>"
        );
}
