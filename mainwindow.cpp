#include "mainwindow.h"
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), imageLoaded(false), cameraActive(false)
{
    setupUI();

    cameraTimer = new QTimer(this);
    videoCapture = nullptr;

    connect(cameraTimer, &QTimer::timeout, this, &MainWindow::updateCameraFrame);
}

MainWindow::~MainWindow()
{
    if (videoCapture) {
        videoCapture->release();
        delete videoCapture;
    }
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);
    buttonLayout = new QHBoxLayout();

    // Создание кнопок
    loadButton = new QPushButton("📁 Загрузить изображение", this);
    scanButton = new QPushButton("🔍 Сканировать", this);
    clearButton = new QPushButton("🗑️ Очистить", this);
    saveButton = new QPushButton("💾 Сохранить", this);
    cameraButton = new QPushButton("📷 Включить камеру", this);

    saveButton->setEnabled(false);

    // Добавление кнопок в горизонтальный layout
    buttonLayout->addWidget(loadButton);
    buttonLayout->addWidget(scanButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cameraButton);

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
    mainLayout->addWidget(imageLabel);
    mainLayout->addWidget(resultText);
    mainLayout->addWidget(progressBar);

    // Подключение сигналов к слотам
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadImage);
    connect(scanButton, &QPushButton::clicked, this, &MainWindow::scanBarcode);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::clearResults);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveBarcode);
    connect(cameraButton, &QPushButton::clicked, this, &MainWindow::toggleCamera);

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
    if (!imageLoaded && !cameraActive) {
        QMessageBox::warning(this, "Ошибка", "Сначала загрузите изображение или включите камеру");
        return;
    }

    progressBar->setVisible(true);
    progressBar->setRange(0, 0); // Индикатор прогресса без определенного конца

    // Если камера активна, используем текущий кадр
    cv::Mat imageToScan = cameraActive ? currentImage.clone() : currentImage;

    resultText->append("🔍 Начинаю сканирование...");

    // Запускаем сканирование
    BarcodeReader::BarcodeResult result = barcodeReader.decode(imageToScan);

    processBarcodeResult(result);

    progressBar->setVisible(false);
}

void MainWindow::clearResults()
{
    resultText->clear();
    imageLabel->clear();
    imageLabel->setText("Изображение не загружено");
    imageLoaded = false;
    saveButton->setEnabled(false);

    // Останавливаем камеру при очистке
    if (cameraActive) {
        toggleCamera();
    }
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
        resultText->append("💡 Камера автоматически сканирует штрих-коды");

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

        // Автоматически сохраняем при обнаружении с камеры
        if (cameraActive) {
            QTimer::singleShot(1000, this, [this]() {
                saveBarcode();
                resultText->append("💾 Автоматически сохранено в файл");
            });
        }
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

            // 🔥 АВТОМАТИЧЕСКОЕ СКАНИРОВАНИЕ ПРИ АКТИВНОЙ КАМЕРЕ
            // Сканируем каждый кадр, но ограничиваем частоту чтобы не нагружать процессор
            static int frameCounter = 0;
            frameCounter++;

            // Сканируем каждый 10-й кадр (примерно 3 раза в секунду)
            if (frameCounter % 10 == 0) {
                BarcodeReader::BarcodeResult result = barcodeReader.decode(frame);
                if (result.type != "Неизвестно" && result.type != "Ошибка" && !result.digits.empty()) {
                    // Проверяем, не тот же ли самый штрих-код уже был распознан
                    QString newBarcode = QString::fromStdString(result.type) + " " + QString::fromStdString(result.digits);
                    if (newBarcode != lastBarcodeResult) {
                        processBarcodeResult(result);

                        // Добавляем небольшую задержку перед следующим сканированием
                        frameCounter = 0; // Сбрасываем счетчик
                    }
                }
            }
        }
    }
}
