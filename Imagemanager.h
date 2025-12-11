#ifndef IMAGEMANAGER_H
#define IMAGEMANAGER_H

#include <QObject>
#include <QString>
#include <QSize>      // ????????
#include <QRect>      // ????????
#include <opencv2/opencv.hpp>

class ImageManager : public QObject
{
    Q_OBJECT

public:
    explicit ImageManager(QObject* parent = nullptr);

    bool loadImage(const QString& filePath);
    bool saveImage(const QString& filePath, const cv::Mat& image);
    cv::Mat getCurrentImage() const;
    bool hasImage() const;
    void clearImage();

    // Методы для обработки изображений
    cv::Mat resizeImage(const cv::Mat& image, const QSize& size, bool keepAspectRatio = true);
    cv::Mat convertToDisplayFormat(const cv::Mat& image);
    cv::Mat enhanceImage(const cv::Mat& image, double contrast = 1.0, double brightness = 0.0);
    cv::Mat cropImage(const cv::Mat& image, const QRect& region);

    QString getLastFilePath() const;
    QSize getImageSize() const;

signals:
    void imageLoaded(const QString& filePath, const QSize& size);
    void imageCleared();
    void imageError(const QString& error);

private:
    cv::Mat currentImage;
    QString lastFilePath;
    bool imageLoadedFlag;
};

#endif // IMAGEMANAGER_H
