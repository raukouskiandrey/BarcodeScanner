#ifndef MANUFACTURER_H
#define MANUFACTURER_H

#include <QString>

class Manufacturer
{
private:
    QString manufacturerCode;
    QString manufacturerName;
    QString countryCode;

public:
    Manufacturer(const QString& code = "", const QString& name = "", const QString& country = "");

    QString getManufacturerCode() const;
    QString getManufacturerName() const;
    QString getCountryCode() const;
    QString getFullInfo() const;

    bool isValid() const;
};

#endif // MANUFACTURER_H
