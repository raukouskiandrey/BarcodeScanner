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
#include <QMap>
#include <QString>

#include "country.h"
#include "manufacturer.h"
#include "product.h"
#include "barcodedetector.h"
#include "barcodedecoder.h"
#include "barcodevalidator.h"
#include "barcodepreprocessor.h"
#include "barcoderegionfinder.h"
#include "countrydataloader.h"
#include "manufacturerdataloader.h"
#include "productdataloader.h"
#include "fileexportservice.h"
#include "resultdisplay.h"

// Определяем класс здесь чтобы был доступен везде
class BarcodeResult {
public:
    std::string type;
    std::string digits;
    std::string fullResult;
    std::string country;
    std::string manufacturerCode;
    std::string productCode;

    BarcodeResult()
        : type("Неизвестно"), digits(""), fullResult(""), country(""), manufacturerCode(""), productCode("")
    {
    }

    BarcodeResult(const std::string& t, const std::string& d, const std::string& f = "",
                  const std::string& c = "", const std::string& m = "", const std::string& p = "")
        : type(t), digits(d), fullResult(f), country(c), manufacturerCode(m), productCode(p)
    {
    }

    bool isValid() const {
        return !type.empty() && type != "Неизвестно" && type != "Ошибка" && !digits.empty();
    }

    std::string toString() const {
        return type + ": " + digits;
    }
};

class BarcodeReader {
public:
    // Используем тот же класс
    typedef ::BarcodeResult BarcodeResult;

    BarcodeReader();
    ~BarcodeReader();

    BarcodeResult decode(const cv::Mat& image);
    BarcodeResult decode(const std::string& filename);

    static void saveToFile(const BarcodeResult& result);
    static void saveToFile(const std::string& barcodeData);

private:
    CountryDataLoader countryLoader;
    ManufacturerDataLoader manufacturerLoader;
    ProductDataLoader productLoader;
    BarcodeDetector detector;
    BarcodeDecoder decoder;
    BarcodeValidator validator;
    BarcodePreprocessor preprocessor;
    BarcodeRegionFinder regionFinder;

    void loadData();
    void cleanupData();

    BarcodeResult createDetailedResult(const BarcodeResult& basicResult);
    BarcodeResult createDetailedResultWithClasses(const BarcodeResult& basicResult);
    BarcodeResult advancedDecode(const cv::Mat& image);
    BarcodeResult parseZBarResult(const std::string& zbarResult);
};

#endif // BARCODESCANNER_H
