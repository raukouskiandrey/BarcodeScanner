#include "dataloader.h"
#include "country.h"
#include "manufacturer.h"
#include "product.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <iostream>

QMap<QString, Country*> DataLoader::loadCountries(const QString& filename) {
    QMap<QString, Country*> countries;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "Cannot open countries file: " << filename.toStdString() << std::endl;
        return countries;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;

        // Предполагаем формат: код;название_страны
        QStringList parts = line.split(";");
        if (parts.size() >= 2) {
            QString code = parts[0].trimmed();
            QString name = parts[1].trimmed();

            if (!code.isEmpty() && !name.isEmpty()) {
                countries[code] = new Country(code, name);
                std::cout << "Loaded country: " << code.toStdString() << " - " << name.toStdString() << std::endl;
            }
        }
    }

    file.close();
    return countries;
}

QMap<QString, Manufacturer*> DataLoader::loadManufacturers(const QString& filename) {
    QMap<QString, Manufacturer*> manufacturers;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "Cannot open manufacturers file: " << filename.toStdString() << std::endl;
        return manufacturers;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;

        // Формат может быть разным, попробуем разные варианты
        QStringList parts = line.split(";");
        if (parts.size() >= 2) {
            QString manufacturerCode = parts[0].trimmed();
            QString manufacturerName = parts[1].trimmed();
            QString countryCode = "";

            // Если есть третий параметр - это код страны
            if (parts.size() >= 3) {
                countryCode = parts[2].trimmed();
            }

            if (!manufacturerCode.isEmpty() && !manufacturerName.isEmpty()) {
                manufacturers[manufacturerCode] = new Manufacturer(manufacturerCode, manufacturerName, countryCode);
                std::cout << "Loaded manufacturer: " << manufacturerCode.toStdString()
                          << " - " << manufacturerName.toStdString()
                          << " (country: " << countryCode.toStdString() << ")" << std::endl;
            }
        }
    }

    file.close();
    return manufacturers;
}

QMap<QString, Product*> DataLoader::loadProducts(const QString& filename) {
    QMap<QString, Product*> products;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "Cannot open products file: " << filename.toStdString() << std::endl;
        return products;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;

        // Формат: штрих-код;код_продукта;название_продукта
        QStringList parts = line.split(";");
        if (parts.size() >= 3) {
            QString barcode = parts[0].trimmed();
            QString productCode = parts[1].trimmed();
            QString productName = parts[2].trimmed();

            if (!barcode.isEmpty() && !productCode.isEmpty() && !productName.isEmpty()) {
                products[barcode] = new Product(barcode, productCode, productName);
                std::cout << "Loaded product: " << barcode.toStdString()
                          << " - " << productCode.toStdString()
                          << " - " << productName.toStdString() << std::endl;
            }
        }
    }

    file.close();
    return products;
}
