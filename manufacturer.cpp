#include "manufacturer.h"

Manufacturer::Manufacturer(const QString& code, const QString& name, const QString& country)
    : manufacturerCode(code), manufacturerName(name), countryCode(country)
{
}

QString Manufacturer::getManufacturerCode() const {
    return manufacturerCode;
}

QString Manufacturer::getManufacturerName() const {
    return manufacturerName;
}

QString Manufacturer::getCountryCode() const {
    return countryCode;
}

QString Manufacturer::getFullInfo() const {
    return manufacturerName + " (" + manufacturerCode + ")";
}

bool Manufacturer::isValid() const {
    return !manufacturerCode.isEmpty() && !manufacturerName.isEmpty();
}
