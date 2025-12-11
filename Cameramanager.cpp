#include "Cameramanager.h"
#include <QDebug>
#include "CameraException.h"

CameraManager::CameraManager(QObject* parent)
    : QObject(parent)
    , frameTimer(std::make_unique<QTimer>(this))
{
    connect(frameTimer.get(), &QTimer::timeout, this, &CameraManager::updateFrame);
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
    videoCapture = std::make_unique<cv::VideoCapture>(cameraIndex, backend);

    if (videoCapture->isOpened()) {
        cv::Mat testFrame;
        *videoCapture >> testFrame;
        if (!testFrame.empty()) {
            return true;
        }
    }

    videoCapture.reset();
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
        videoCapture.reset();
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

cv::Mat CameraManager::getCurrentFrame() const
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
                cv::flip(frame, frame, 1);
            }

            currentFrame = frame.clone();
            emit frameReady(frame);
        }
    }
}
