#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextEdit>
#include <QProgressBar>
#include <QTimer>
#include <opencv2/opencv.hpp>
#include "barcodescanner.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadImage();
    void scanBarcode();
    void clearResults();
    void saveBarcode();
    void toggleCamera();
    void updateCameraFrame();
    void processBarcodeResult(const BarcodeReader::BarcodeResult& result);

private:
    void setupUI();
    void displayImage(const cv::Mat& image);

    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QHBoxLayout *buttonLayout;

    QPushButton *loadButton;
    QPushButton *scanButton;
    QPushButton *clearButton;
    QPushButton *saveButton;
    QPushButton *cameraButton;

    QLabel *imageLabel;
    QTextEdit *resultText;
    QProgressBar *progressBar;

    cv::Mat currentImage;
    BarcodeReader barcodeReader;
    bool imageLoaded;
    QString lastBarcodeResult;

    QTimer *cameraTimer;
    cv::VideoCapture *videoCapture;
    bool cameraActive;
};

#endif // MAINWINDOW_H
