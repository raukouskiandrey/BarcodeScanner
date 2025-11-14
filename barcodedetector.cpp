#include "barcodedetector.h"
#include <iostream>
#include <opencv2/opencv.hpp>

BarcodeDetector::BarcodeDetector()
{
}

std::vector<std::vector<cv::Point>> BarcodeDetector::detectWithOpenCV(const cv::Mat& frame)
{
    std::vector<std::vector<cv::Point>> polygons;
    std::vector<cv::Point> corners;
    std::vector<std::string> decoded_info, decoded_type;

    try {
        bool detected = opencv_detector.detectAndDecodeWithType(frame, decoded_info, decoded_type, corners);

        if (detected && !corners.empty() && corners.size() % 4 == 0) {
            for (int i = 0; i < (int)corners.size(); i += 4) {
                if (i + 3 < corners.size()) {
                    std::vector<cv::Point> polygon = { corners[i], corners[i + 1], corners[i + 2], corners[i + 3] };
                    polygons.push_back(polygon);
                }
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "OpenCV detection error: " << e.what() << std::endl;
    }

    return polygons;
}

std::vector<cv::Rect> BarcodeDetector::detectCurvedBarcodesOptimized(const cv::Mat& frame)
{
    std::vector<cv::Rect> curved_regions;

    cv::Mat small_frame;
    cv::resize(frame, small_frame, cv::Size(320, 240));

    cv::Mat gray;
    cv::cvtColor(small_frame, gray, cv::COLOR_BGR2GRAY);

    std::vector<cv::Mat> binary_images;

    cv::Mat binary1, binary2, binary3;
    cv::adaptiveThreshold(gray, binary1, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                          cv::THRESH_BINARY, 21, 5);
    cv::adaptiveThreshold(gray, binary2, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                          cv::THRESH_BINARY, 31, 10);
    cv::threshold(gray, binary3, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    binary_images.push_back(binary1);
    binary_images.push_back(binary2);
    binary_images.push_back(binary3);

    cv::Mat grad_x, grad_y;
    cv::Sobel(gray, grad_x, CV_16S, 1, 0, 3);
    cv::Sobel(gray, grad_y, CV_16S, 0, 1, 3);
    cv::convertScaleAbs(grad_x, grad_x);
    cv::convertScaleAbs(grad_y, grad_y);
    cv::Mat gradients;
    cv::addWeighted(grad_x, 0.5, grad_y, 0.5, 0, gradients);
    cv::threshold(gradients, gradients, 50, 255, cv::THRESH_BINARY);
    binary_images.push_back(gradients);

    for (const auto& binary : binary_images) {
        // Здесь будет вызов BarcodeRegionFinder
        // curved_regions.insert(curved_regions.end(), contour_regions.begin(), contour_regions.end());
    }

    // Удаление дубликатов через BarcodeRegionFinder
    // curved_regions = removeDuplicateRegions(curved_regions);

    double scale_x = (double)frame.cols / small_frame.cols;
    double scale_y = (double)frame.rows / small_frame.rows;

    for (auto& bbox : curved_regions) {
        bbox.x = (int)(bbox.x * scale_x);
        bbox.y = (int)(bbox.y * scale_y);
        bbox.width = (int)(bbox.width * scale_x);
        bbox.height = (int)(bbox.height * scale_y);
    }

    return curved_regions;
}
