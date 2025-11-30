#include "CurvedBarcodeDetector.h"
#include <iostream>

std::vector<cv::Rect> CurvedBarcodeDetector::detectCurvedBarcodesOptimized(const cv::Mat& frame) {
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
        std::vector<cv::Rect> contour_regions = extractRegionsFromContours(binary, small_frame.size());
        curved_regions.insert(curved_regions.end(), contour_regions.begin(), contour_regions.end());
    }

    curved_regions = removeDuplicateRegions(curved_regions);

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

std::vector<cv::Rect> CurvedBarcodeDetector::extractRegionsFromContours(const cv::Mat& binary, const cv::Size& image_size) {
    std::vector<cv::Rect> regions;

    cv::Mat morph;
    cv::Mat kernel_horizontal = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 1));
    cv::Mat kernel_vertical = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 3));

    cv::morphologyEx(binary, morph, cv::MORPH_CLOSE, kernel_horizontal);
    cv::morphologyEx(morph, morph, cv::MORPH_OPEN, kernel_vertical);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(morph, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for (const auto& contour : contours) {
        if (contour.empty()) continue;

        cv::Rect bbox = cv::boundingRect(contour);

        if (isValidBarcodeRegionExtended(bbox, image_size, contour)) {
            if (hasBarcodeTextureAdvanced(binary(bbox))) {
                cv::Rect expanded_bbox = expandBarcodeRegion(bbox, image_size);
                regions.push_back(expanded_bbox);
            }
        }
    }

    return regions;
}

bool CurvedBarcodeDetector::isValidBarcodeRegionExtended(const cv::Rect& rect, const cv::Size& image_size,
    const std::vector<cv::Point>& contour) {
    if (rect.width < 30 || rect.height < 10) return false;
    if (rect.width > image_size.width * 0.7 || rect.height > image_size.height * 0.7) return false;

    double area = rect.width * rect.height;
    if (area < 500) return false;

    double aspect_ratio = (double)rect.width / rect.height;
    bool valid_aspect = (aspect_ratio > 1.0 && aspect_ratio < 15.0);

    double contour_area = cv::contourArea(contour);
    double extent = contour_area / area;
    bool valid_contour = (extent > 0.3);

    return valid_aspect && valid_contour;
}

cv::Rect CurvedBarcodeDetector::expandBarcodeRegion(const cv::Rect& original, const cv::Size& image_size) {
    int expand_x = original.width * 0.2;
    int expand_y = original.height * 0.3;

    cv::Rect expanded = original;
    expanded.x = std::max(0, original.x - expand_x / 2);
    expanded.y = std::max(0, original.y - expand_y / 2);
    expanded.width = std::min(image_size.width - expanded.x, original.width + expand_x);
    expanded.height = std::min(image_size.height - expanded.y, original.height + expand_y);

    return expanded;
}

std::vector<cv::Rect> CurvedBarcodeDetector::removeDuplicateRegions(const std::vector<cv::Rect>& regions) {
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

bool CurvedBarcodeDetector::hasBarcodeTextureAdvanced(const cv::Mat& region) {
    if (region.empty() || region.rows < 5 || region.cols < 5) return false;

    cv::Mat gray = region;
    if (region.channels() == 3) {
        cv::cvtColor(region, gray, cv::COLOR_BGR2GRAY);
    }

    cv::Mat grad_x, grad_y;
    cv::Sobel(gray, grad_x, CV_32F, 1, 0, 3);
    cv::Sobel(gray, grad_y, CV_32F, 0, 1, 3);

    cv::Scalar mean_x, stddev_x, mean_y, stddev_y;
    cv::meanStdDev(grad_x, mean_x, stddev_x);
    cv::meanStdDev(grad_y, mean_y, stddev_y);

    double horizontal_stripe = stddev_x[0] / (std::abs(mean_x[0]) + 1e-5);
    double vertical_stripe = stddev_y[0] / (std::abs(mean_y[0]) + 1e-5);

    bool is_barcode_like = (horizontal_stripe > 1.8) && (horizontal_stripe > vertical_stripe * 1.2);

    return is_barcode_like;
}
