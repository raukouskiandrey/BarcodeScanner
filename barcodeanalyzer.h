#ifndef BARCODEANALYZER_H
#define BARCODEANALYZER_H

#include "barcodescanner.h"

class CountryResolver;
class ManufacturerResolver;
class ProductResolver;

class BarcodeAnalyzer {
public:
    BarcodeAnalyzer();
    void setResolvers(CountryResolver* country, ManufacturerResolver* manufacturer, ProductResolver* product);
    BarcodeReader::BarcodeResult analyze(const BarcodeReader::BarcodeResult& basicResult);

private:
    CountryResolver* countryResolver;
    ManufacturerResolver* manufacturerResolver;
    ProductResolver* productResolver;
};

#endif // BARCODEANALYZER_H
