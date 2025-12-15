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
    CameraManager* cameraManager;
    ImageManager* imageManager;
    ImageBuffer<cv::Mat> cameraBuffer{10};

    // --- UI ---
    QWidget* centralWidget;                             // 4
    QVBoxLayout* mainLayout;                            // 5
    QHBoxLayout* buttonLayout;                          // 6
    QPushButton* loadButton;                            // 7
    QPushButton* scanButton;
    QPushButton* clearButton;                           // 9
    QPushButton* saveButton;                            // 10
    QPushButton* cameraButton;
    QPushButton* phoneButton;                           // 12
    QLabel* imageLabel;
    QTextEdit* resultText;
    QProgressBar* progressBar;

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
