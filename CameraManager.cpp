#include "cameramanager.h"
#include <QDebug>
#include "CameraException.h"

CameraManager::CameraManager(QObject* parent)
    : QObject(parent), videoCapture(nullptr), cameraActive(false), mirrorMode(true)
{
    frameTimer = new QTimer(this);
    connect(frameTimer, &QTimer::timeout, this, &CameraManager::updateFrame);
}

CameraManager::~CameraManager()
{
    stopCamera();
}

bool CameraManager::startCamera(int cameraIndex) {
    if (cameraActive) return true;

    if (tryOpenCameraWithBackend(cameraIndex, cv::CAP_DSHOW) ||
        tryOpenCameraWithBackend(cameraIndex, cv::CAP_MSMF) ||
        tryOpenCameraWithBackend(cameraIndex, cv::CAP_ANY)) {
        cameraActive = true;
        frameTimer->start(33);
        emit cameraStarted();
        return true;
    }

    throw CameraException("Не удалось подключиться к камере");
}

bool CameraManager::tryOpenCameraWithBackend(int cameraIndex, int backend)
{
    if (videoCapture) {
        delete videoCapture;
    }

    videoCapture = new cv::VideoCapture(cameraIndex, backend);

    if (videoCapture->isOpened()) {
        // Проверяем, что камера действительно передает изображение
        cv::Mat testFrame;
        *videoCapture >> testFrame;
        if (!testFrame.empty()) {
            return true;
        }
    }

    delete videoCapture;
    videoCapture = nullptr;
    return false;
}

void CameraManager::stopCamera()
{
    if (frameTimer->isActive()) {
        frameTimer->stop();
    }

    if (videoCapture) {
        if (videoCapture->isOpened()) {
            videoCapture->release();
        }
        delete videoCapture;
        videoCapture = nullptr;
    }

    if (cameraActive) {
        cameraActive = false;
        emit cameraStopped();
    }
}

bool CameraManager::isCameraActive() const
{
    return cameraActive;
}

cv::Mat CameraManager::getCurrentFrame()
{
    return currentFrame.clone();
}

void CameraManager::setMirrorMode(bool enabled)
{
    mirrorMode = enabled;
}

void CameraManager::updateFrame()
{
    if (videoCapture && videoCapture->isOpened()) {
        cv::Mat frame;
        *videoCapture >> frame;

        if (!frame.empty()) {
            if (mirrorMode) {
                cv::flip(frame, frame, 1); // Горизонтальное отражение
            }

            currentFrame = frame.clone();
            emit frameReady(frame);
        }
    }
}
