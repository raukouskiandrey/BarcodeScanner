#include "mainwindow.h"
#include <QApplication>
#include <QDir>
#include <QDateTime>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QtConcurrent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), imageLoaded(false), cameraActive(false),
    serverActive(false), serverPort(8080), isScanning(false)
{
    setupUI();

    cameraTimer = new QTimer(this);
    videoCapture = nullptr;

    connect(cameraTimer, &QTimer::timeout, this, &MainWindow::updateCameraFrame);

    // Web server initialization
    tcpServer = nullptr;
    uploadDir = QDir::currentPath() + "/uploads";
    QDir().mkpath(uploadDir);
}

MainWindow::~MainWindow()
{
    if (videoCapture) {
        videoCapture->release();
        delete videoCapture;
    }

    if (tcpServer) {
        tcpServer->close();
        delete tcpServer;
    }

    // Ждем завершения сканирования
    if (scanFuture.isRunning()) {
        scanFuture.waitForFinished();
    }
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);
    buttonLayout = new QHBoxLayout();
    serverButtonLayout = new QHBoxLayout();

    // Создание кнопок
    loadButton = new QPushButton("📁 Загрузить изображение", this);
    scanButton = new QPushButton("🔍 Сканировать", this);
    clearButton = new QPushButton("🗑️ Очистить", this);
    saveButton = new QPushButton("💾 Сохранить", this);
    cameraButton = new QPushButton("📷 Включить камеру", this);
    webServerButton = new QPushButton("🌐 Запустить веб-сервер", this);

    saveButton->setEnabled(false);

    // Добавление кнопок в горизонтальный layout
    buttonLayout->addWidget(loadButton);
    buttonLayout->addWidget(scanButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cameraButton);

    // Web server section
    serverStatusLabel = new QLabel("🔴 Веб-сервер не запущен", this);
    serverButtonLayout->addWidget(webServerButton);
    serverButtonLayout->addWidget(serverStatusLabel);

    // Создание остальных элементов интерфейса
    imageLabel = new QLabel("Изображение не загружено", this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setMinimumSize(400, 300);
    imageLabel->setStyleSheet("border: 1px solid gray;");

    resultText = new QTextEdit(this);
    resultText->setReadOnly(true);
    resultText->setPlaceholderText("Результаты сканирования появятся здесь...");

    progressBar = new QProgressBar(this);
    progressBar->setVisible(false);

    // Добавление элементов в основной layout
    mainLayout->addLayout(buttonLayout);
    mainLayout->addLayout(serverButtonLayout);
    mainLayout->addWidget(imageLabel);
    mainLayout->addWidget(resultText);
    mainLayout->addWidget(progressBar);

    // Подключение сигналов к слотам
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadImage);
    connect(scanButton, &QPushButton::clicked, this, &MainWindow::scanBarcode);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::clearResults);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveBarcode);
    connect(cameraButton, &QPushButton::clicked, this, &MainWindow::toggleCamera);
    connect(webServerButton, &QPushButton::clicked, this, &MainWindow::toggleWebServer);

    // лог
    mainLayout->addWidget(new QLabel("Event log:", this));
    m_logText = new QTextEdit(this);
    m_logText->setReadOnly(true);
    m_logText->setMaximumHeight(200);
    mainLayout->addWidget(m_logText);

    // место для картинки
    m_imageLabel = new QLabel("📷 Uploaded image will appear here", this);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setStyleSheet("border: 1px solid gray; background: #fafafa;");
    m_imageLabel->setMinimumSize(400, 300);
    mainLayout->addWidget(m_imageLabel);

}

void MainWindow::loadImage()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    "Выберите изображение со штрих-кодом",
                                                    "",
                                                    "Images (*.png *.jpg *.jpeg *.bmp *.tiff)");

    if (!filename.isEmpty()) {
        currentImage = cv::imread(filename.toStdString());
        if (!currentImage.empty()) {
            displayImage(currentImage);
            imageLoaded = true;
            resultText->append("✅ Изображение загружено: " + filename);
            resultText->append("📏 Размер: " + QString::number(currentImage.cols) + "x" + QString::number(currentImage.rows));
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось загрузить изображение");
        }
    }
}

void MainWindow::scanBarcode()
{
    if (isScanning) {
        QMessageBox::information(this, "Информация", "Сканирование уже выполняется...");
        return;
    }

    if (!imageLoaded && !cameraActive) {
        QMessageBox::warning(this, "Ошибка", "Сначала загрузите изображение или включите камеру");
        return;
    }

    // Блокируем кнопки во время сканирования
    scanButton->setEnabled(false);
    loadButton->setEnabled(false);
    cameraButton->setEnabled(false);
    isScanning = true;

    progressBar->setVisible(true);
    progressBar->setRange(0, 0);

    resultText->append("🔍 Начинаю сканирование...");

    // Запускаем сканирование в отдельном потоке
    scanFuture = QtConcurrent::run([this]() {
        try {
            // Если камера активна, используем текущий кадр
            cv::Mat imageToScan;
            if (cameraActive) {
                // УПРОЩЕННАЯ ВЕРСИЯ БЕЗ МЬЮТЕКСА
                imageToScan = currentImage.clone();
            } else {
                imageToScan = currentImage.clone();
            }

            // Запускаем сканирование
            BarcodeReader::BarcodeResult result = barcodeReader.decode(imageToScan);

            // Используем QMetaObject для thread-safe вызова
            QMetaObject::invokeMethod(this, "processBarcodeResult",
                                      Qt::QueuedConnection,
                                      Q_ARG(BarcodeReader::BarcodeResult, result));

        } catch (const std::exception& e) {
            qDebug() << "Scanning error:" << e.what();
            BarcodeReader::BarcodeResult errorResult;
            errorResult.type = "Ошибка";
            errorResult.digits = "Ошибка при сканировании: " + std::string(e.what());

            QMetaObject::invokeMethod(this, "processBarcodeResult",
                                      Qt::QueuedConnection,
                                      Q_ARG(BarcodeReader::BarcodeResult, errorResult));
        }

        // Сигнализируем о завершении
        QMetaObject::invokeMethod(this, "onScanFinished", Qt::QueuedConnection);
    });
}
void MainWindow::onScanFinished()
{
    // Разблокируем кнопки
    scanButton->setEnabled(true);
    loadButton->setEnabled(true);
    cameraButton->setEnabled(true);
    isScanning = false;
    progressBar->setVisible(false);
}

void MainWindow::clearResults()
{
    // Останавливаем сканирование если оно выполняется
    if (scanFuture.isRunning()) {
        scanFuture.cancel();
        scanFuture.waitForFinished();
    }

    resultText->clear();
    imageLabel->clear();
    imageLabel->setText("Изображение не загружено");
    imageLoaded = false;
    saveButton->setEnabled(false);
    isScanning = false;

    // Останавливаем камеру при очистке
    if (cameraActive) {
        toggleCamera();
    }

    // Разблокируем кнопки
    scanButton->setEnabled(true);
    loadButton->setEnabled(true);
    cameraButton->setEnabled(true);
    progressBar->setVisible(false);
}

void MainWindow::saveBarcode()
{
    if (!lastBarcodeResult.isEmpty()) {
        BarcodeReader::saveToFile(lastBarcodeResult.toStdString());
        resultText->append("✅ Результат сохранен в файл!");
    }
}

void MainWindow::toggleCamera()
{
    if (!cameraActive) {
        // Включение камеры
        videoCapture = new cv::VideoCapture(0);

        if (!videoCapture->isOpened()) {
            QMessageBox::warning(this, "Ошибка", "Не удалось открыть камеру");
            delete videoCapture;
            videoCapture = nullptr;
            return;
        }

        cameraActive = true;
        cameraButton->setText("📷 Выключить камеру");
        cameraTimer->start(33); // ~30 FPS
        resultText->append("📷 Камера включена. Наведите на штрих-код...");

    } else {
        // Выключение камеры
        cameraTimer->stop();
        if (videoCapture) {
            videoCapture->release();
            delete videoCapture;
            videoCapture = nullptr;
        }
        cameraActive = false;
        cameraButton->setText("📷 Включить камеру");
        resultText->append("📷 Камера выключена");
    }
}

void MainWindow::displayImage(const cv::Mat& image)
{
    if (image.empty()) return;

    cv::Mat displayImage;
    if (image.channels() == 3) {
        cv::cvtColor(image, displayImage, cv::COLOR_BGR2RGB);
    } else {
        cv::cvtColor(image, displayImage, cv::COLOR_GRAY2RGB);
    }

    QImage qimage(displayImage.data,
                  displayImage.cols,
                  displayImage.rows,
                  displayImage.step,
                  QImage::Format_RGB888);

    QPixmap pixmap = QPixmap::fromImage(qimage);
    pixmap = pixmap.scaled(imageLabel->width(),
                           imageLabel->height(),
                           Qt::KeepAspectRatio,
                           Qt::SmoothTransformation);

    imageLabel->setPixmap(pixmap);
}

void MainWindow::processBarcodeResult(const BarcodeReader::BarcodeResult& result)
{
    resultText->append("\n🎯 === РЕЗУЛЬТАТ СКАНИРОВАНИЯ ===");
    resultText->append("📊 Тип: " + QString::fromStdString(result.type));
    resultText->append("🔢 Полный код: " + QString::fromStdString(result.digits));

    if (!result.country.empty() && result.country != "Неизвестно") {
        resultText->append("🌍 Страна: " + QString::fromStdString(result.country));
    }

    if (!result.manufacturerCode.empty() && result.manufacturerCode != "Н/Д" && result.manufacturerCode != "Нет") {
        resultText->append("🏭 Код производителя: " + QString::fromStdString(result.manufacturerCode));
    }

    if (!result.productCode.empty() && result.productCode != "Н/Д") {
        resultText->append("📦 Код товара: " + QString::fromStdString(result.productCode));
    }

    lastBarcodeResult = QString::fromStdString(result.type) + " " +
                        QString::fromStdString(result.digits);

    if (result.type != "Неизвестно" && result.type != "Ошибка" && !result.digits.empty()) {
        resultText->append("✅ Штрих-код успешно распознан!");
        saveButton->setEnabled(true);
    } else {
        resultText->append("❌ Не удалось распознать штрих-код");
        saveButton->setEnabled(false);
    }
}

void MainWindow::updateCameraFrame()
{
    if (videoCapture && videoCapture->isOpened()) {
        cv::Mat frame;
        *videoCapture >> frame;

        if (!frame.empty()) {
            // 🔄 УБИРАЕМ ЗЕРКАЛЬНОЕ ОТОБРАЖЕНИЕ - переворачиваем по горизонтали
            cv::flip(frame, frame, 1); // 1 - горизонтальное отражение

            currentImage = frame.clone();
            displayImage(frame);
        }
    }
}

// ... остальные методы веб-сервера остаются без изменений ...

// Web server implementation
void MainWindow::toggleWebServer()
{
    if (!serverActive) {
        // Start web server
        tcpServer = new QTcpServer(this);

        if (!tcpServer->listen(QHostAddress::Any, serverPort)) {
            QMessageBox::warning(this, "Ошибка", "Не удалось запустить веб-сервер: " + tcpServer->errorString());
            delete tcpServer;
            tcpServer = nullptr;
            return;
        }

        connect(tcpServer, &QTcpServer::newConnection, this, &MainWindow::onNewConnection);

        serverActive = true;
        webServerButton->setText("🌐 Остановить веб-сервер");
        serverStatusLabel->setText("🟢 Веб-сервер запущен на порту " + QString::number(serverPort));

        // Show connection URL
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

        resultText->append("🌐 Веб-сервер запущен: http://" + ipAddress + ":" + QString::number(serverPort));
        resultText->append("📱 Теперь можно загружать изображения через браузер");

    } else {
        // Stop web server
        for (QTcpSocket *client : clients) {
            client->close();
            client->deleteLater();
        }
        clients.clear();

        if (tcpServer) {
            tcpServer->close();
            tcpServer->deleteLater();
            tcpServer = nullptr;
        }

        serverActive = false;
        webServerButton->setText("🌐 Запустить веб-сервер");
        serverStatusLabel->setText("🔴 Веб-сервер не запущен");
        resultText->append("🌐 Веб-сервер остановлен");
    }
}

void MainWindow::onNewConnection()
{
    QTcpSocket *client = tcpServer->nextPendingConnection();
    if (!client) return;

    clients.append(client);
    connect(client, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(client, &QTcpSocket::disconnected, this, &MainWindow::onClientDisconnected);

    resultText->append("🌐 Новое подключение: " + client->peerAddress().toString());
}

void MainWindow::onReadyRead()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    QByteArray requestData = client->readAll();
    processHttpRequest(client, requestData);
}

void MainWindow::onClientDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    clients.removeAll(client);
    client->deleteLater();
    resultText->append("🌐 Клиент отключен");
}
void MainWindow::processHttpRequest(QTcpSocket *client, const QByteArray &request)
{
    QString requestStr = QString::fromUtf8(request);
    resultText->append("🌐 Запрос: " + requestStr.left(100) + "..."); // Логируем начало запроса

    if (requestStr.startsWith("GET / ")) {
        // Show HTML form
        QString html = generateHtmlForm();
        sendHttpResponse(client, html.toUtf8());
        resultText->append("🌐 Отправлена HTML форма для загрузки");
    }
    else if (requestStr.startsWith("POST /upload")) {
        // Process file upload
        resultText->append("🌐 Получен запрос на загрузку изображения");

        // Extract boundary from content-type
        QString boundary;
        int boundaryIndex = requestStr.indexOf("boundary=");
        if (boundaryIndex != -1) {
            boundary = requestStr.mid(boundaryIndex + 9);
            boundary = boundary.left(boundary.indexOf("\r\n"));
            boundary = "--" + boundary;
            resultText->append("🌐 Boundary: " + boundary);
        } else {
            resultText->append("❌ Boundary не найден в запросе");
            QString errorMessage = "❌ Boundary not found";
            sendHttpResponse(client, errorMessage.toUtf8(), "text/plain", 400);
            return;
        }

        if (!boundary.isEmpty()) {
            saveUploadedImage(request, boundary);
            QString successMessage = "✅ Изображение успешно загружено и обработано!";
            sendHttpResponse(client, successMessage.toUtf8(), "text/plain");
        } else {
            QString errorMessage = "❌ Ошибка при загрузке изображения";
            sendHttpResponse(client, errorMessage.toUtf8(), "text/plain", 500);
        }
    }
    else if (requestStr.startsWith("GET")) {
        // For any other GET request, show the form
        QString html = generateHtmlForm();
        sendHttpResponse(client, html.toUtf8());
        resultText->append("🌐 Отправлена HTML форма (альтернативный GET)");
    }
    else {
        QString notFoundMessage = "404 - Страница не найдена";
        sendHttpResponse(client, notFoundMessage.toUtf8(), "text/plain", 404);
        resultText->append("❌ Неизвестный запрос");
    }
}
void MainWindow::sendHttpResponse(QTcpSocket *client, const QByteArray &content,
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

QString MainWindow::generateHtmlForm()
{
    return QString(
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "    <title>Barcode Scanner - Upload</title>"
        "    <meta charset='utf-8'>"
        "    <style>"
        "        body { font-family: Arial; margin: 40px; background: #f5f5f5; }"
        "        .container { max-width: 500px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }"
        "        h2 { color: #333; text-align: center; }"
        "        .form-group { margin-bottom: 20px; }"
        "        input[type='file'] { padding: 15px; border: 2px dashed #ccc; width: 100%%; border-radius: 5px; }"
        "        input[type='submit'] { background: #007cba; color: white; padding: 15px 30px; border: none; border-radius: 5px; cursor: pointer; width: 100%%; font-size: 16px; }"
        "        input[type='submit']:hover { background: #005a87; }"
        "        .status { margin-top: 20px; padding: 15px; border-radius: 5px; text-align: center; }"
        "        .success { background: #d4edda; color: #155724; }"
        "        .info { background: #d1ecf1; color: #0c5460; }"
        "    </style>"
        "</head>"
        "<body>"
        "    <div class='container'>"
        "        <h2>📷 Загрузка изображения со штрих-кодом</h2>"
        "        <div class='status info'>"
        "            Веб-сервер запущен! Загрузите изображение для сканирования штрих-кода."
        "        </div>"
        "        <form action='/upload' method='post' enctype='multipart/form-data'>"
        "            <div class='form-group'>"
        "                <input type='file' name='barcode_image' accept='image/*' required>"
        "            </div>"
        "            <div class='form-group'>"
        "                <input type='submit' value='📤 Отправить для сканирования'>"
        "            </div>"
        "        </form>"
        "        <div id='status'></div>"
        "    </div>"
        "    <script>"
        "        document.querySelector('form').addEventListener('submit', function(e) {"
        "            document.getElementById('status').innerHTML = '<div class=\"status success\">Отправка файла на сервер...</div>';"
        "        });"
        "    </script>"
        "</body>"
        "</html>"
        );
}

void MainWindow::saveUploadedImage(const QByteArray &data, const QString &boundary)
{
    resultText->append("📱 Получено изображение с телефона");
    resultText->append("🔧 Анализируем структуру данных...");

    // Получаем полный boundary (с -- в начале)
    QByteArray fullBoundary = boundary.toUtf8();
    resultText->append("📍 Boundary: " + QString(fullBoundary));

    // 1. Ищем начало файловой части
    QByteArray filePartStart = fullBoundary + "\r\n";
    filePartStart += "Content-Disposition: form-data;";

    int filePartPos = data.indexOf(filePartStart);
    if (filePartPos == -1) {
        resultText->append("❌ Не найдена файловая часть");
        return;
    }
    resultText->append("✅ Файловая часть найдена на позиции: " + QString::number(filePartPos));

    // 2. Ищем имя файла
    QByteArray filenameMarker = "filename=\"";
    int filenameStart = data.indexOf(filenameMarker, filePartPos);
    if (filenameStart == -1) {
        resultText->append("❌ Не найдено имя файла");
        return;
    }
    filenameStart += filenameMarker.length();

    int filenameEnd = data.indexOf("\"", filenameStart);
    if (filenameEnd == -1) {
        resultText->append("❌ Не найден конец имени файла");
        return;
    }

    QString filename = QString::fromUtf8(data.mid(filenameStart, filenameEnd - filenameStart));
    resultText->append("📁 Имя файла: " + filename);

    // 3. Ищем конец заголовков и начало бинарных данных
    QByteArray headersEnd = "\r\n\r\n";
    int dataStart = data.indexOf(headersEnd, filenameEnd);
    if (dataStart == -1) {
        resultText->append("❌ Не найден конец заголовков");
        return;
    }
    dataStart += headersEnd.length();
    resultText->append("📍 Начало бинарных данных: " + QString::number(dataStart));

    // 4. Ищем конец данных (следующий boundary)
    QByteArray endBoundary = fullBoundary + "--";
    int dataEnd = data.indexOf(endBoundary, dataStart);
    if (dataEnd == -1) {
        // Если не нашли закрывающий boundary, ищем обычный boundary
        dataEnd = data.indexOf(fullBoundary, dataStart);
        if (dataEnd == -1) {
            resultText->append("❌ Не найден конец данных");
            // Используем конец данных как fallback
            dataEnd = data.length();
            resultText->append("🔄 Использую конец данных: " + QString::number(dataEnd));
        }
    }

    // Вычитаем 2 байта для \r\n перед boundary
    if (dataEnd > 2) {
        dataEnd -= 2;
    }

    resultText->append("📍 Конец бинарных данных: " + QString::number(dataEnd));

    // 5. Извлекаем данные изображения
    int imageDataSize = dataEnd - dataStart;
    if (imageDataSize <= 0) {
        resultText->append("❌ Размер данных изображения некорректен: " + QString::number(imageDataSize));
        return;
    }

    QByteArray imageData = data.mid(dataStart, imageDataSize);
    resultText->append("📊 Размер данных изображения: " + QString::number(imageData.size()) + " байт");

    // 6. Проверяем валидность данных
    if (imageData.size() < 100) {
        resultText->append("❌ Данные изображения слишком малы");
        return;
    }

    // Проверяем JPEG маркеры
    bool hasJpegMarkers = imageData.startsWith("\xff\xd8") && imageData.endsWith("\xff\xd9");
    resultText->append(hasJpegMarkers ? "✅ Обнаружены JPEG маркеры" : "⚠️ JPEG маркеры не обнаружены");

    // Покажем первые 20 байт в hex для отладки
    QByteArray firstBytes = imageData.left(20).toHex();
    resultText->append("🔍 Первые байты: " + QString(firstBytes));

    // 7. Сохраняем данные изображения
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString savedFilename = uploadDir + "/uploaded_" + timestamp + ".jpg";

    QFile file(savedFilename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(imageData);
        file.close();
        resultText->append("✅ Изображение сохранено: " + savedFilename);

        // 8. Проверяем валидность сохраненного файла
        QImage testImage;
        if (testImage.load(savedFilename)) {
            resultText->append("✅ Изображение валидно: " +
                               QString::number(testImage.width()) + "x" +
                               QString::number(testImage.height()));
        } else {
            // Пробуем другие форматы
            resultText->append("❌ JPEG не загружается, пробую другие форматы...");

            if (testImage.load(savedFilename, "PNG")) {
                resultText->append("✅ Обнаружен PNG формат");
            } else if (testImage.load(savedFilename, "JPEG")) {
                resultText->append("✅ Обнаружен JPEG формат (с указанием codec)");
            } else {
                resultText->append("❌ Не удалось определить формат изображения");

                // Попробуем сохранить как PNG
                QString pngFilename = uploadDir + "/uploaded_" + timestamp + ".png";
                QFile pngFile(pngFilename);
                if (pngFile.open(QIODevice::WriteOnly)) {
                    pngFile.write(imageData);
                    pngFile.close();

                    if (testImage.load(pngFilename)) {
                        resultText->append("✅ Успешно сохранено как PNG");
                        savedFilename = pngFilename;
                    }
                }
                return;
            }
        }

        // 9. Загружаем через OpenCV и отображаем
        currentImage = cv::imread(savedFilename.toStdString());
        if (currentImage.empty()) {
            resultText->append("❌ OpenCV не смог загрузить изображение, пробую через QImage...");

            // Альтернативный способ через QImage
            QImage qImage(savedFilename);
            if (!qImage.isNull()) {
                qImage = qImage.convertToFormat(QImage::Format_RGB888);
                currentImage = cv::Mat(qImage.height(), qImage.width(), CV_8UC3,
                                       (void*)qImage.constBits(), qImage.bytesPerLine()).clone();
                resultText->append("✅ Изображение загружено через QImage");
            }
        }

        if (!currentImage.empty()) {
            displayImage(currentImage);
            imageLoaded = true;
            resultText->append("🎯 Изображение отображено на экране");
            resultText->append("📏 Размер OpenCV: " +
                               QString::number(currentImage.cols) + "x" +
                               QString::number(currentImage.rows));

            // 10. Автоматически сканируем
            QTimer::singleShot(1000, this, [this]() {
                resultText->append("🔍 Запускаю сканирование...");
                scanBarcode();
            });
        } else {
            resultText->append("❌ Не удалось загрузить изображение для отображения");
        }

    } else {
        resultText->append("❌ Ошибка сохранения файла: " + file.errorString());
    }
}
// Добавьте эту функцию для конвертации QImage в cv::Mat
cv::Mat MainWindow::QImageToMat(const QImage& qImage)
{
    try {
        if (qImage.isNull()) return cv::Mat();

        QImage converted = qImage.convertToFormat(QImage::Format_RGB888);
        return cv::Mat(converted.height(), converted.width(), CV_8UC3,
                       (void*)converted.constBits(), converted.bytesPerLine()).clone();
    } catch (...) {
        return cv::Mat();
    }
}

void MainWindow::onImageReceived(const QPixmap &pixmap)
{
    if (!pixmap.isNull()) {
        m_imageLabel->setPixmap(pixmap.scaled(m_imageLabel->size(),
                                              Qt::KeepAspectRatio,
                                              Qt::SmoothTransformation));
        this->onLogMessage("✅ Image displayed on screen");
    } else {
        m_imageLabel->setText("❌ Failed to decode image");
        this->onLogMessage("❌ Error: received invalid image data");
    }
}

void MainWindow::onLogMessage(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_logText->append("[" + timestamp + "] " + message);
}



