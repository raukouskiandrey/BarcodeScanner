#include "manufacturerresolver.h"

void ManufacturerResolver::setManufacturers(const QMap<QString, Manufacturer*>& manufacturersMap) {
    manufacturers = manufacturersMap;
}

Manufacturer* ManufacturerResolver::findManufacturerByCode(const QString& code) {
    return manufacturers.contains(code) ? manufacturers[code] : nullptr;
}
