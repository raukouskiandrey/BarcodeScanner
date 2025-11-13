#ifndef BARCODESCANNER_H
#define BARCODESCANNER_H

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <chrono>
#include <ctime>
#include <zbar.h>

class BarcodeReader {
public:
    struct BarcodeResult {
        std::string type;
        std::string digits;
        std::string fullResult;
        std::string country;
        std::string manufacturerCode;
        std::string productCode;
    };

    BarcodeReader();
    BarcodeResult decode(const cv::Mat& image);
    BarcodeResult decode(const std::string& filename);

    static std::string getCountryName(const std::string& prefix);
    static void loadCountryCodes();

    static void saveToFile(const BarcodeResult& result);
    static void saveToFile(const std::string& barcodeData);

private:
    static std::map<std::string, std::string> countryCodesMap;
    static bool countryCodesLoaded;

    // Приватные методы для сохранения в разные файлы
    static void saveToMainFile(const BarcodeResult& result);
    static void saveToCountryFile(const BarcodeResult& result);
    static void saveToManufacturerFile(const BarcodeResult& result);
    static void saveToProductFile(const BarcodeResult& result);
    static void saveToStatisticsFile(const BarcodeResult& result);

    std::vector<int> extractBits(const std::vector<int>& profile);
    BarcodeResult decodeBits(const std::vector<int>& bits);
    BarcodeResult tryDecodeEAN13(const std::string& bitstream);
    BarcodeResult tryDecodeEAN8(const std::string& bitstream);
    static bool validateChecksum(const std::string& ean);

    // 🔥 Advanced Barcode Scanner методы
    BarcodeResult advancedDecode(const cv::Mat& image);
    std::vector<std::vector<cv::Point>> detectWithOpenCV(const cv::Mat& frame);
    std::vector<cv::Rect> detectCurvedBarcodesOptimized(const cv::Mat& frame);
    std::vector<cv::Rect> extractRegionsFromContours(const cv::Mat& binary, const cv::Size& image_size);
    bool isValidBarcodeRegionExtended(const cv::Rect& rect, const cv::Size& image_size, const std::vector<cv::Point>& contour);
    cv::Rect expandBarcodeRegion(const cv::Rect& original, const cv::Size& image_size);
    std::vector<cv::Rect> removeDuplicateRegions(const std::vector<cv::Rect>& regions);
    bool hasBarcodeTextureAdvanced(const cv::Mat& region);
    std::string smartDecodeWithUnwarp(const cv::Mat& frame, const cv::Rect& rect);
    cv::Mat enhanceContrast(const cv::Mat& input);
    cv::Mat enhanceSharpness(const cv::Mat& input, double strength);
    std::string decodeWithZBar(const cv::Mat& roi);
    std::string filterBarcodeResult(const std::string& result);
    BarcodeResult parseZBarResult(const std::string& zbarResult);
    BarcodeResult createDetailedResult(const BarcodeResult& basicResult);

    cv::barcode::BarcodeDetector opencv_detector;
    zbar::ImageScanner zbar_scanner;
};

#endif
