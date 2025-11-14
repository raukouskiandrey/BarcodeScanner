#ifndef FILEEXPORTSERVICE_H
#define FILEEXPORTSERVICE_H

#include <string>

// Используем forward declaration
struct BarcodeResult;

class FileExportService
{
public:
    FileExportService();

    static void saveToMainFile(const BarcodeResult& result);
    static void saveToCountryFile(const BarcodeResult& result);
    static void saveToManufacturerFile(const BarcodeResult& result);
    static void saveToProductFile(const BarcodeResult& result);
    static void saveToStatisticsFile(const BarcodeResult& result);
    static void saveToSimpleFile(const std::string& barcodeData);

private:
    static void appendToFile(const std::string& filePath, const std::string& content);
};

#endif // FILEEXPORTSERVICE_H
