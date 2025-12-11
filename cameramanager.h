#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H

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
    cv::Mat getCurrentFrame();
    void setMirrorMode(bool enabled);

signals:
    void frameReady(const cv::Mat& frame);
    void cameraStarted();
    void cameraStopped();
    void cameraError(const QString& error);

private slots:
    void updateFrame();

private:
    bool tryOpenCameraWithBackend(int cameraIndex, int backend);
    cv::VideoCapture* videoCapture;
    QTimer* frameTimer;
    bool cameraActive;
    bool mirrorMode;
    cv::Mat currentFrame;
};

#endif // CAMERAMANAGER_H
