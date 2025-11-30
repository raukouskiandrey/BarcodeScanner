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

#include "cameramanager.h"
#include "imagemanager.h"
#include "BarcodeReader.h"
#include "BarcodeReader2D.h"
#include "BarcodeResult.h"
#include "WebServer.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // --- кнопки ---
    void loadImage();
    void scanBarcode();
    void clearResults();
    void saveBarcode();
    void toggleCamera();

    // --- CameraManager ---
    void onCameraFrameReady(const cv::Mat& frame);
    void onCameraStarted();
    void onCameraStopped();
    void onCameraError(const QString& error);

    // --- ImageManager ---
    void onImageLoaded(const QString& filePath, const QSize& size);
    void onImageCleared();
    void onImageError(const QString& error);

private:
    // --- UI ---
    QWidget* centralWidget;
    QVBoxLayout* mainLayout;
    QHBoxLayout* buttonLayout;

    QPushButton* loadButton;
    QPushButton* scanButton;
    QPushButton* clearButton;
    QPushButton* saveButton;
    QPushButton* cameraButton;

    QLabel* imageLabel;
    QTextEdit* resultText;
    QProgressBar* progressBar;

    // --- Менеджеры ---
    CameraManager* cameraManager;
    ImageManager* imageManager;
    BarcodeReader barcodeReader;   // экземпляр BarcodeReader
    BarcodeReader2D barcodeReader2D;

    // --- Последний результат ---
    BarcodeResult lastResult;
    QString lastBarcodeResult;

    QPushButton* phoneButton;


    // --- Методы ---
    void setupUI();
    void setupConnections();
    void displayImage(const cv::Mat& image);
    void updateScanButtonState();
    void processBarcodeResult(const BarcodeResult& result);
    void openPhoneDialog();
};

#endif // MAINWINDOW_H
