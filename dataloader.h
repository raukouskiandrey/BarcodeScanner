#ifndef DATALOADER_H
#define DATALOADER_H

#include <QString>
#include <QMap>
#include <QTextStream>
#include <QDebug>
#include "country.h"
#include "manufacturer.h"
#include "product.h"
#include "filemanager.h"

class DataLoader
{
public:
    static QMap<QString, Country*> loadCountries(const QString& filename);
    static QMap<QString, Manufacturer*> loadManufacturers(const QString& filename);
    static QMap<QString, Product*> loadProducts(const QString& filename);

private:
    static void parseCountryRange(const QString& codes, const QString& countryName,
                                  QMap<QString, Country*>& countries);
    static QString trim(const QString& str);
};

#endif // DATALOADER_H
