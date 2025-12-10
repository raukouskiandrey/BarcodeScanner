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
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QFuture>
#include <QtConcurrent>
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

    // Web server slots
    void toggleWebServer();
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();

    // Async slots
    void onScanFinished();

private slots:
    void onImageReceived(const QPixmap &pixmap);
    void onLogMessage(const QString &message);

private:
    void setupUI();
    void displayImage(const cv::Mat& image);
    void processHttpRequest(QTcpSocket *client, const QByteArray &request);
    void sendHttpResponse(QTcpSocket *client, const QByteArray &content,
                          const QString &contentType = "text/html", int statusCode = 200);
    QString generateHtmlForm();
    void saveUploadedImage(const QByteArray &data, const QString &boundary);
    cv::Mat QImageToMat(const QImage& qImage);

    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QHBoxLayout *buttonLayout;
    QHBoxLayout *serverButtonLayout;

    QPushButton *loadButton;
    QPushButton *scanButton;
    QPushButton *clearButton;
    QPushButton *saveButton;
    QPushButton *cameraButton;
    QPushButton *webServerButton;

    QLabel *imageLabel;
    QLabel *serverStatusLabel;
    QTextEdit *resultText;
    QProgressBar *progressBar;

    cv::Mat currentImage;
    BarcodeReader barcodeReader;
    bool imageLoaded;
    QString lastBarcodeResult;

    QTimer *cameraTimer;
    cv::VideoCapture *videoCapture;
    bool cameraActive;

    // Web server members
    QTcpServer *tcpServer;
    QList<QTcpSocket*> clients;
    quint16 serverPort;
    bool serverActive;
    QString uploadDir;
    QLabel *m_imageLabel;
    QTextEdit *m_logText;
    // Async scanning
    bool isScanning;
    QFuture<void> scanFuture;
};

#endif // MAINWINDOW_H
