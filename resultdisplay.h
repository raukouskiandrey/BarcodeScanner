#ifndef RESULTDISPLAY_H
#define RESULTDISPLAY_H

#include <string>

// Используем forward declaration
struct BarcodeResult;

class ResultDisplay
{
public:
    ResultDisplay();

    static std::string formatBarcodeResult(const BarcodeResult& result);
    static void displayScanInfo(const std::string& info);
    static void showStatistics(const std::string& stats);
};

#endif // RESULTDISPLAY_H
