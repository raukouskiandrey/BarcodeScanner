#ifndef RESULTSAVER_H
#define RESULTSAVER_H

#include <QString>
#include <QDateTime>
#include <QMap>
#include <QTextStream>
#include "barcodescanner.h"
#include "filemanager.h"

class ResultSaver
{
public:
    static void saveToMainFile(const BarcodeReader::BarcodeResult& result);
    static void saveToCountryFile(const BarcodeReader::BarcodeResult& result);
    static void saveToManufacturerFile(const BarcodeReader::BarcodeResult& result);
    static void saveToProductFile(const BarcodeReader::BarcodeResult& result);
    static void saveToStatisticsFile(const BarcodeReader::BarcodeResult& result);

    static void setOutputDirectory(const QString& directory);
    static QString getOutputDirectory();

private:
    static QString outputDirectory;
    static QString getTimestamp();
    static QString formatResultLine(const BarcodeReader::BarcodeResult& result);
    static void updateStatistics(const BarcodeReader::BarcodeResult& result,
                                 QMap<QString, int>& countryStats,
                                 QMap<QString, int>& typeStats);
};

#endif // RESULTSAVER_H
