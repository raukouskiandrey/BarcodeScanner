#include "cameramanager.h"
#include <QDebug>

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

bool CameraManager::startCamera(int cameraIndex)
{
    if (cameraActive) {
        return true;
    }

    // Пробуем разные бэкенды в порядке приоритета
    if (tryOpenCameraWithBackend(cameraIndex, cv::CAP_DSHOW) ||
        tryOpenCameraWithBackend(cameraIndex, cv::CAP_MSMF) ||
        tryOpenCameraWithBackend(cameraIndex, cv::CAP_ANY)) {

        cameraActive = true;
        frameTimer->start(33); // ~30 FPS
        emit cameraStarted();
        return true;
    }

    emit cameraError("Не удалось подключиться к камере");
    return false;
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