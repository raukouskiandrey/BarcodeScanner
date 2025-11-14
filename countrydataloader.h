#ifndef COUNTRYDATALOADER_H
#define COUNTRYDATALOADER_H

#include "country.h"
#include <QMap>
#include <QString>
#include <string>

class CountryDataLoader
{
public:
    CountryDataLoader();
    ~CountryDataLoader();

    void loadFromFile(const std::string& filePath = "C:/Users/rauko/Desktop/Barcode_Countries.txt");
    Country* getCountryByCode(const QString& code);
    int getCountriesCount() const { return countries.size(); }

private:
    void parseCountryLine(const std::string& line);

    QMap<QString, Country*> countries;
};

#endif // COUNTRYDATALOADER_H
