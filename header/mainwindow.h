#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QProgressBar>
#include <QFileDialog>
#include <QMessageBox>

#include <opencv2/opencv.hpp>
#include <memory>
#include <vector>

#include "Cameramanager.h"
#include "Imagemanager.h"
#include "AbstractDecoder.h"
#include "BarcodeReader.h"
#include "BarcodeReader2D.h"
#include "BarcodeResult.h"
#include "WebServer.h"
#include "ImageBuffer.h"
#include "BarcodeException.h"
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void loadImage();
    void scanBarcode();
    void clearResults();
    void saveBarcode();
    void toggleCamera();

    // CameraManager
    void onCameraFrameReady(const cv::Mat& frame);
    void onCameraStarted();
    void onCameraStopped();
    void onCameraError(const QString& error);

    // ImageManager
    void onImageLoaded(const QString& filePath, const QSize& size);
    void onImageCleared();
    void onImageError(const QString& error);

private:
    // --- Менеджеры --- (объявляем ПЕРВЫМИ)
    CameraManager* cameraManager;                       // 1
    ImageManager* imageManager;                         // 2
    ImageBuffer<cv::Mat> cameraBuffer{10};              // 3 - in-class инициализация

    // --- UI ---
    QWidget* centralWidget;                             // 4
    QVBoxLayout* mainLayout;                            // 5
    QHBoxLayout* buttonLayout;                          // 6
    QPushButton* loadButton;                            // 7
    QPushButton* scanButton;                            // 8
    QPushButton* clearButton;                           // 9
    QPushButton* saveButton;                            // 10
    QPushButton* cameraButton;                          // 11
    QPushButton* phoneButton;                           // 12
    QLabel* imageLabel;                                 // 13
    QTextEdit* resultText;                              // 14
    QProgressBar* progressBar;                          // 15

    // --- Декодеры и результаты ---
    std::vector<std::unique_ptr<AbstractDecoder>> decoders;  // 16
    BarcodeResult lastResult;                            // 17
    QString lastBarcodeResult;                           // 18
    AbstractDecoder* lastDecoder = nullptr;              // 19

    // --- Методы ---
    void setupUI();
    void setupConnections();
    void displayImage(const cv::Mat& image);
    void updateScanButtonState();
    void processBarcodeResult(const BarcodeResult& result);
    void openPhoneDialog();
    BarcodeResult decodeImageWithDecoders(const cv::Mat& imageToScan);

};

#endif // MAINWINDOW_H
