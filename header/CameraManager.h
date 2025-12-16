#pragma once
#include <QObject>
#include <QTimer>
#include <opencv2/opencv.hpp>

class CameraManager : public QObject
{
    Q_OBJECT
public:
    explicit CameraManager(QObject* parent = nullptr);
    ~CameraManager() override;
    bool startCamera(int cameraIndex = 0);
    void stopCamera();
    bool isCameraActive() const;
    cv::Mat getCurrentFrame() const;
    void setMirrorMode(bool enabled);
signals:
    void frameReady(const cv::Mat& frame);
    void cameraStarted();
    void cameraStopped();
    void cameraError(const QString& error);
private:
    void updateFrame();
    bool tryOpenCameraWithBackend(int cameraIndex, int backend);
    cv::VideoCapture* videoCapture = nullptr;
    QTimer* frameTimer = nullptr;
    bool cameraActive = false;
    bool mirrorMode = true;
    cv::Mat currentFrame;
};
