#include "imagemanager.h"
#include <QDebug>
#include <QFileInfo>

ImageManager::ImageManager(QObject* parent)
    : QObject(parent), imageLoadedFlag(false)
{
}

bool ImageManager::loadImage(const QString& filePath)
{
    if (!QFileInfo::exists(filePath)) {
        emit imageError("Файл не существует: " + filePath);
        return false;
    }

    cv::Mat image = cv::imread(filePath.toStdString());
    if (image.empty()) {
        emit imageError("Не удалось загрузить изображение: " + filePath);
        return false;
    }

    currentImage = image;
    lastFilePath = filePath;
    imageLoadedFlag = true;

    emit imageLoaded(filePath, QSize(image.cols, image.rows));
    return true;
}

bool ImageManager::saveImage(const QString& filePath, const cv::Mat& image)
{
    if (image.empty()) {
        emit imageError("Пустое изображение для сохранения");
        return false;
    }

    bool success = cv::imwrite(filePath.toStdString(), image);
    if (!success) {
        emit imageError("Не удалось сохранить изображение: " + filePath);
    }

    return success;
}

cv::Mat ImageManager::getCurrentImage() const
{
    return currentImage.clone();
}

bool ImageManager::hasImage() const
{
    return imageLoadedFlag && !currentImage.empty();
}

void ImageManager::clearImage()
{
    currentImage.release();
    lastFilePath.clear();
    imageLoadedFlag = false;
    emit imageCleared();
}

cv::Mat ImageManager::resizeImage(const cv::Mat& image, const QSize& size, bool keepAspectRatio)
{
    if (image.empty()) return cv::Mat();

    cv::Mat resized;

    if (keepAspectRatio) {
        double scale = qMin(static_cast<double>(size.width()) / image.cols,
            static_cast<double>(size.height()) / image.rows);
        cv::resize(image, resized, cv::Size(), scale, scale, cv::INTER_AREA);
    }
    else {
        cv::resize(image, resized, cv::Size(size.width(), size.height()), 0, 0, cv::INTER_AREA);
    }

    return resized;
}

cv::Mat ImageManager::convertToDisplayFormat(const cv::Mat& image)
{
    if (image.empty()) return cv::Mat();

    cv::Mat displayImage;

    if (image.channels() == 3) {
        cv::cvtColor(image, displayImage, cv::COLOR_BGR2RGB);
    }
    else if (image.channels() == 1) {
        cv::cvtColor(image, displayImage, cv::COLOR_GRAY2RGB);
    }
    else {
        displayImage = image.clone();
    }

    return displayImage;
}

cv::Mat ImageManager::enhanceImage(const cv::Mat& image, double contrast, double brightness)
{
    if (image.empty()) return cv::Mat();

    cv::Mat enhanced;
    image.convertTo(enhanced, -1, contrast, brightness);
    return enhanced;
}

cv::Mat ImageManager::cropImage(const cv::Mat& image, const QRect& region)
{
    if (image.empty() || region.isEmpty()) return cv::Mat();

    // Проверяем границы
    int x = qMax(0, region.x());
    int y = qMax(0, region.y());
    int width = qMin(image.cols - x, region.width());
    int height = qMin(image.rows - y, region.height());

    if (width <= 0 || height <= 0) return cv::Mat();

    return image(cv::Rect(x, y, width, height)).clone();
}

QString ImageManager::getLastFilePath() const
{
    return lastFilePath;
}

QSize ImageManager::getImageSize() const
{
    if (currentImage.empty()) return QSize();
    return QSize(currentImage.cols, currentImage.rows);
}