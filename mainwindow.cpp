#include "mainwindow.h"
#include <QApplication>
#include <QClipboard>

#include "ImageLoadException.h"
#include "DecodeException.h"
#include "FileException.h"
#include "CameraException.h"
#include "ImageBuffer.h"
#include "FailureAnalysis.h"

MainWindow::~MainWindow()
{
    delete cameraManager;
    delete imageManager;
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
    cameraManager(new CameraManager(this)),
    imageManager(new ImageManager(this))
{
    // Добавляем все декодеры в список
    decoders.push_back(std::make_unique<BarcodeReader>());
    decoders.push_back(std::make_unique<BarcodeReader2D>());
    // В будущем можно добавить: decoders.push_back(std::make_unique<BarcodeReaderPDF417>());

    setupUI();
    setupConnections();
    updateScanButtonState();
}


void MainWindow::setupConnections()
{
    // CameraManager
    connect(cameraManager, &CameraManager::frameReady, this, &MainWindow::onCameraFrameReady);
    connect(cameraManager, &CameraManager::cameraStarted, this, &MainWindow::onCameraStarted);
    connect(cameraManager, &CameraManager::cameraStopped, this, &MainWindow::onCameraStopped);
    connect(cameraManager, &CameraManager::cameraError, this, &MainWindow::onCameraError);

    // ImageManager
    connect(imageManager, &ImageManager::imageLoaded, this, &MainWindow::onImageLoaded);
    connect(imageManager, &ImageManager::imageCleared, this, &MainWindow::onImageCleared);
    connect(imageManager, &ImageManager::imageError, this, &MainWindow::onImageError);

    // Buttons
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadImage);
    connect(scanButton, &QPushButton::clicked, this, &MainWindow::scanBarcode);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::clearResults);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveBarcode);
    connect(cameraButton, &QPushButton::clicked, this, &MainWindow::toggleCamera);
    connect(phoneButton, &QPushButton::clicked, this, &MainWindow::openPhoneDialog);
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);
    buttonLayout = new QHBoxLayout();

    loadButton   = new QPushButton("📁 Загрузить изображение", this);
    scanButton   = new QPushButton("🔍 Сканировать", this);
    clearButton  = new QPushButton("🗑️ Очистить", this);
    saveButton   = new QPushButton("💾 Сохранить", this);
    cameraButton = new QPushButton("📷 Включить камеру", this);
    phoneButton = new QPushButton("📱 Загрузить с телефона", this);

    scanButton->setEnabled(false);
    saveButton->setEnabled(false);

    buttonLayout->addWidget(loadButton);
    buttonLayout->addWidget(scanButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cameraButton);
    buttonLayout->addWidget(phoneButton);

    imageLabel = new QLabel("Изображение не загружено", this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setMinimumSize(400, 300);
    imageLabel->setStyleSheet("border: 1px solid gray;");

    resultText = new QTextEdit(this);
    resultText->setReadOnly(true);
    resultText->setPlaceholderText("Результаты сканирования появятся здесь...");

    progressBar = new QProgressBar(this);
    progressBar->setVisible(false);

    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(imageLabel);
    mainLayout->addWidget(resultText);
    mainLayout->addWidget(progressBar);

    setWindowTitle("Barcode Scanner v2.0 - Считывание с камеры");
    resize(800, 600);
}

void MainWindow::loadImage()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    "Выберите изображение со штрих-кодом",
                                                    "",
                                                    "Images (*.png *.jpg *.jpeg *.bmp *.tiff)");

    if (!filename.isEmpty()) {
        if (cameraManager->isCameraActive()) {
            cameraManager->stopCamera();
        }

        try {
            imageManager->loadImage(filename); // теперь выбрасывает исключение
        }
        catch (const ImageLoadException& e) {
            QMessageBox::critical(this, "Ошибка загрузки", e.what());
        }
    }
}

BarcodeResult MainWindow::decodeImageWithDecoders(const cv::Mat& imageToScan) {
    BarcodeResult result;
    for (auto& decoder : decoders) {
        try {
            result = decoder->decode(imageToScan);
            if (result.type != "Неизвестно" && !result.digits.empty()) {
                return result;
            }
        } catch (const DecodeException& e) {
            resultText->append(QString("⚠️ Ошибка декодера: ") + e.what());
        }
    }
    throw DecodeException("Ни один декодер не распознал штрих-код");
}

void MainWindow::scanBarcode() {
    progressBar->setVisible(true);
    progressBar->setRange(0, 0);

    try {
        if (!imageManager->hasImage() && !cameraManager->isCameraActive()) {
            throw ImageLoadException("Нет изображения или камеры для сканирования");
        }

        cv::Mat imageToScan = cameraManager->isCameraActive()
                                  ? cameraManager->getCurrentFrame()
                                  : imageManager->getCurrentImage();

        resultText->append("🔍 Начинаю сканирование...");

        BarcodeResult result = decodeImageWithDecoders(imageToScan);
        processBarcodeResult(result);
    }
    catch (const DecodeException& e) {
        resultText->append("❌ Не удалось распознать штрих-код");

        cv::Mat imageToAnalyze = cameraManager->isCameraActive()
                                     ? cameraManager->getCurrentFrame()
                                     : imageManager->getCurrentImage();

        auto* reader = dynamic_cast<BarcodeReader*>(decoders[0].get());
        if (reader) {
            FailureAnalysis analysis = analyzeDecodingFailure(*reader, imageToAnalyze, "");
            resultText->append("📋 Диагностика ошибки:");
            resultText->append("🔍 " + QString::fromStdString(analysis.primaryProblem.description));
            resultText->append("📌 Причина: " + QString::fromStdString(analysis.primaryProblem.cause));
            resultText->append("💡 Рекомендация: " + QString::fromStdString(analysis.primaryProblem.recommendation));
        }

        QMessageBox::information(this, "Не удалось распознать", e.what());
    }
    catch (const ImageLoadException& e) {
        QMessageBox::critical(this, "Ошибка загрузки", e.what());
    }
    catch (const FileException& e) {
        QMessageBox::critical(this, "Ошибка файла", e.what());
    }
    catch (const CameraException& e) {
        QMessageBox::critical(this, "Ошибка камеры", e.what());
    }
    catch (const BarcodeException& e) {
        QMessageBox::critical(this, "Общая ошибка", e.what());
    }

    progressBar->setVisible(false);
}

void MainWindow::clearResults()
{
    resultText->clear();
    imageLabel->clear();
    imageLabel->setText("Изображение не загружено");

    imageManager->clearImage();
    cameraManager->stopCamera();

    lastBarcodeResult.clear();
    lastResult = BarcodeResult();   // сброс структуры

    saveButton->setEnabled(false);
    updateScanButtonState();
}

void MainWindow::saveBarcode()
{
    if (!lastBarcodeResult.isEmpty()) {
        try {
            for (const auto& decoder : decoders) {
                if (decoder->getDecoderName() == lastResult.type ||
                    (lastResult.type.find("QR") != std::string::npos && decoder->getDecoderName() == "BarcodeReader2D")) {
                    decoder->saveToFile(lastResult);
                    break;
                }
            }
            resultText->append("✅ Результат сохранен в файл!");
        }
        catch (const FileException& e) {
            QMessageBox::critical(this, "Ошибка сохранения", e.what());
        }
    }
}




void MainWindow::toggleCamera()
{
    if (!cameraManager->isCameraActive()) {
        resultText->append("🔄 Попытка подключения к камере...");
        imageManager->clearImage();

        try {
            cameraManager->startCamera(); // ⚠️ может выбросить CameraException
        }
        catch (const CameraException& e) {
            QMessageBox::critical(this, "Ошибка камеры", e.what());
        }
    } else {
        cameraManager->stopCamera();
    }
}

static ImageBuffer<cv::Mat> cameraBuffer(10);

void MainWindow::onCameraFrameReady(const cv::Mat& frame)
{
    // Отображаем кадр в интерфейсе
    displayImage(frame);

    // Счётчик кадров
    static int frameCounter = 0;
    frameCounter++;

    // Добавляем только каждый 5-й кадр
    if (frameCounter % 5 == 0 && !frame.empty()) {
        try {
            cameraBuffer << frame;
        } catch (const std::runtime_error& e) {
            resultText->append(QString("⚠️ Ошибка буфера: ") + e.what());
            return;
        }
    }

    // Проверяем все кадры в контейнере
    try {
        for (const auto& img : cameraBuffer) {
            for (const auto& decoder : decoders) {
                BarcodeResult result = decoder->decode(img);
                if (result.type != "Неизвестно" && !result.digits.empty()) {
                    processBarcodeResult(result);
                    cameraBuffer.clear();
                    frameCounter = 0;
                    return;
                }
            }
        }
    }
    catch (const DecodeException& e) {
        resultText->append(QString("⚠️ Ошибка декодера: ") + e.what());
    }
    catch (const std::exception& e) {
        resultText->append(QString("⚠️ Общая ошибка: ") + e.what());
    }
}
void MainWindow::onCameraStarted()
{
    cameraButton->setText("📷 Выключить камеру");
    resultText->append("✅ Камера успешно подключена!");
    resultText->append("📷 Камера включена. Наведите на штрих-код...");
    updateScanButtonState();
}

void MainWindow::onCameraStopped()
{
    cameraButton->setText("📷 Включить камеру");
    resultText->append("📷 Камера выключена");
    updateScanButtonState();
}

void MainWindow::onCameraError(const QString& error)
{
    QMessageBox::warning(this, "Ошибка камеры", error);
    cameraButton->setText("📷 Включить камеру");
    updateScanButtonState();
}

// --- ImageManager slots ---
void MainWindow::onImageLoaded(const QString& filePath, const QSize& size)
{
    displayImage(imageManager->getCurrentImage());
    resultText->append("✅ Изображение загружено: " + filePath);
    resultText->append("📏 Размер: " + QString::number(size.width()) + "x" + QString::number(size.height()));
    updateScanButtonState();
}

void MainWindow::onImageCleared()
{
    imageLabel->clear();
    imageLabel->setText("Изображение не загружено");
    updateScanButtonState();
}

void MainWindow::onImageError(const QString& error)
{
    QMessageBox::warning(this, "Ошибка изображения", error);
}

// --- Общие методы ---
void MainWindow::displayImage(const cv::Mat& image)
{
    if (image.empty()) {
        imageLabel->setText("Изображение не доступно");
        return;
    }

    cv::Mat displayImage = imageManager->convertToDisplayFormat(image);

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

void MainWindow::updateScanButtonState()
{
    bool canScan = imageManager->hasImage() || cameraManager->isCameraActive();
    scanButton->setEnabled(canScan);

    if (!canScan) {
        scanButton->setToolTip("Сначала загрузите изображение или включите камеру");
    } else {
        scanButton->setToolTip("Начать сканирование штрих-кода");
    }

    if (cameraManager->isCameraActive()) {
        cameraButton->setText("📷 Выключить камеру");
    } else {
        cameraButton->setText("📷 Включить камеру");
    }
}

void MainWindow::processBarcodeResult(const BarcodeResult& result)
{
    resultText->append("\n🎯 === РЕЗУЛЬТАТ СКАНИРОВАНИЯ ===");
    resultText->append("📊 Тип: " + QString::fromStdString(result.type));
    resultText->append("🔢 Полный код: " + QString::fromStdString(result.digits));

    if (!result.country.empty() && result.country != "Неизвестно") {
        resultText->append("🌍 Страна: " + QString::fromStdString(result.country));
    }
    if (!result.manufacturerCode.empty() &&
        result.manufacturerCode != "Н/Д" &&
        result.manufacturerCode != "Нет") {
        resultText->append("🏭 Код производителя: " + QString::fromStdString(result.manufacturerCode));
    }
    // Для 1D штрих-кодов выводим код товара, для 2D — нет
    if (result.type != "QR/DataMatrix" && result.type != "QR-Code" &&
        !result.productCode.empty() && result.productCode != "Н/Д") {
        resultText->append("📦 Код товара: " + QString::fromStdString(result.productCode));
    }


    // сохраняем объект для последующего вызова saveToFile
    lastResult = result;
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

void MainWindow::openPhoneDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("📱 Загрузка с телефона");

    auto* layout = new QVBoxLayout(&dialog);
    auto* startBtn = new QPushButton("🚀 Запустить веб-сервер", &dialog);
    auto* stopBtn = new QPushButton("⛔ Выключить веб-сервер", &dialog);
    auto* copyBtn = new QPushButton("📋 Скопировать адрес", &dialog);
    auto* statusLabel = new QLabel("Сервер не запущен", &dialog);

    auto* server = new WebServer(&dialog);

    layout->addWidget(startBtn);
    layout->addWidget(stopBtn);
    layout->addWidget(statusLabel);
    layout->addWidget(copyBtn);

    copyBtn->setEnabled(false);
    stopBtn->setEnabled(false); // выключать можно только если сервер запущен

    connect(startBtn, &QPushButton::clicked, [&]() {
        if (server->startServer(8080)) {
            statusLabel->setText("✅ Сервер запущен: " + server->serverAddress());
            copyBtn->setEnabled(true);
            stopBtn->setEnabled(true);
        }
    });

    connect(stopBtn, &QPushButton::clicked, [&]() {
        server->stopServer();
        statusLabel->setText("⛔ Сервер остановлен");
        copyBtn->setEnabled(false);
        stopBtn->setEnabled(false);
    });

    connect(copyBtn, &QPushButton::clicked, [&]() {
        QApplication::clipboard()->setText(server->serverAddress());
        QMessageBox::information(&dialog, "Скопировано", "Адрес скопирован!");
    });

    // Заменяем большую лямбду на несколько маленьких:
    connect(server, &WebServer::fileSaved, this, [this](const QString& path) {
        // Часть 1: Загрузка и отображение
        resultText->append("📂 Файл сохранён: " + path);

        cv::Mat mat = cv::imread(path.toStdString());
        if (!mat.empty()) {
            displayImage(mat);
        } else {
            resultText->append("❌ Ошибка: OpenCV не смог загрузить изображение");
            return;
        }
    });

    // Вторая лямбда для декодирования
    connect(server, &WebServer::fileSaved, this, [this](const QString& path) {
        try {
            BarcodeResult result;
            for (const auto& decoder : decoders) {
                result = decoder->decode(path.toStdString());
                if (result.type != "Неизвестно" && result.type != "Ошибка" && !result.digits.empty()) {
                    processBarcodeResult(result);
                    return;
                }
            }

            QMessageBox::warning(this, "Ошибка при распознавании",
                                 "Не удалось распознать штрих-код на изображении");
        }
        catch (const BarcodeException& e) {
            QMessageBox::warning(this, "Ошибка при распознавании", e.what());
        }
    });
    dialog.exec();
}



