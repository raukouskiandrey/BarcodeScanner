#ifndef MANUFACTURERRESOLVER_H
#define MANUFACTURERRESOLVER_H

#include "manufacturer.h"
#include <QMap>
#include <QString>

class ManufacturerResolver {
private:
    QMap<QString, Manufacturer*> manufacturers;

public:
    ManufacturerResolver() = default;
    void setManufacturers(const QMap<QString, Manufacturer*>& manufacturersMap);
    Manufacturer* findManufacturerByCode(const QString& code);
};

#endif // MANUFACTURERRESOLVER_H
