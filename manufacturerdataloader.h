#ifndef MANUFACTURERDATALOADER_H
#define MANUFACTURERDATALOADER_H

#include "manufacturer.h"
#include <QMap>
#include <QString>
#include <string>

class ManufacturerDataLoader
{
public:
    ManufacturerDataLoader();
    ~ManufacturerDataLoader();

    void loadFromFile(const std::string& filePath = "C:/Users/rauko/Desktop/Barcode_Manufacturers.txt");
    Manufacturer* getManufacturerByCode(const QString& code);
    int getManufacturersCount() const { return manufacturers.size(); }

private:
    void parseManufacturerLine(const std::string& line);

    QMap<QString, Manufacturer*> manufacturers;
};

#endif // MANUFACTURERDATALOADER_H
