#include "barcoderegionfinder.h"
#include <opencv2/opencv.hpp>
#include <algorithm>

BarcodeRegionFinder::BarcodeRegionFinder()
{
}

std::vector<cv::Rect> BarcodeRegionFinder::findBarcodeRegions(const cv::Mat& image)
{
    std::vector<cv::Rect> regions;

    if (image.empty()) {
        return regions;
    }

    try {
        cv::Mat gray;
        if (image.channels() == 3) {
            cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        } else {
            gray = image.clone();
        }

        // Улучшение контраста
        cv::Mat equalized;
        cv::equalizeHist(gray, equalized);

        // Детектор границ
        cv::Mat edges;
        cv::Canny(equalized, edges, 50, 150, 3);

        // Морфологические операции для соединения линий
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
        cv::morphologyEx(edges, edges, cv::MORPH_CLOSE, kernel);

        // Поиск контуров
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(edges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        // Фильтрация контуров по форме и размеру
        for (const auto& contour : contours) {
            if (contour.size() < 4) continue;

            cv::Rect bbox = cv::boundingRect(contour);

            // ПРОВЕРКА ВАЛИДНОСТИ ROI
            if (!isValidROI(bbox, image.size())) {
                continue;
            }

            double area = cv::contourArea(contour);
            double bboxArea = bbox.width * bbox.height;

            // Фильтр по размеру (игнорируем слишком маленькие области)
            if (area < 1000 || bboxArea < 1000) continue;

            // Фильтр по соотношению сторон (штрих-коды обычно прямоугольные)
            double aspectRatio = static_cast<double>(bbox.width) / bbox.height;
            if (aspectRatio < 0.1 || aspectRatio > 10.0) continue;

            // Добавляем небольшой отступ вокруг области
            cv::Rect paddedRect = addPadding(bbox, image.size(), 10);

            if (isValidROI(paddedRect, image.size())) {
                regions.push_back(paddedRect);
            } else {
                // Если paddedRect невалиден, используем оригинальный bbox
                regions.push_back(bbox);
            }
        }

        // Если не найдено регионов, используем все изображение
        if (regions.empty()) {
            regions.push_back(cv::Rect(0, 0, image.cols, image.rows));
        }

        // Удаляем дубликаты
        regions = removeDuplicateRegions(regions);

    } catch (const cv::Exception& e) {
        // В случае ошибки возвращаем все изображение как одну область
        regions.push_back(cv::Rect(0, 0, image.cols, image.rows));
    }

    return regions;
}

bool BarcodeRegionFinder::isValidROI(const cv::Rect& roi, const cv::Size& imageSize)
{
    return (roi.x >= 0 &&
            roi.y >= 0 &&
            roi.width > 0 &&
            roi.height > 0 &&
            roi.x + roi.width <= imageSize.width &&
            roi.y + roi.height <= imageSize.height);
}

cv::Rect BarcodeRegionFinder::addPadding(const cv::Rect& original, const cv::Size& imageSize, int padding)
{
    cv::Rect padded;

    padded.x = std::max(0, original.x - padding);
    padded.y = std::max(0, original.y - padding);

    int right = std::min(imageSize.width, original.x + original.width + padding);
    int bottom = std::min(imageSize.height, original.y + original.height + padding);

    padded.width = right - padded.x;
    padded.height = bottom - padded.y;

    // Гарантируем минимальный размер
    if (padded.width <= 0) padded.width = 1;
    if (padded.height <= 0) padded.height = 1;

    return padded;
}

std::vector<cv::Rect> BarcodeRegionFinder::removeDuplicateRegions(const std::vector<cv::Rect>& regions)
{
    std::vector<cv::Rect> unique_regions;

    for (const auto& rect : regions) {
        bool is_duplicate = false;

        for (const auto& existing : unique_regions) {
            cv::Rect intersection = rect & existing;
            double overlap = (double)intersection.area() / std::min(rect.area(), existing.area());

            if (overlap > 0.6) {
                is_duplicate = true;
                break;
            }
        }

        if (!is_duplicate) {
            unique_regions.push_back(rect);
        }
    }

    return unique_regions;
}
