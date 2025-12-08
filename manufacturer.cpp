#include "manufacturer.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "FileException.h"

Manufacturer::Manufacturer(const QString& code, const QString& name, const QString& country)
    : manufacturerCode(code), manufacturerName(name), countryCode(country) {}

QString Manufacturer::getManufacturerCode() const { return manufacturerCode; }
QString Manufacturer::getManufacturerName() const { return manufacturerName; }
QString Manufacturer::getCountryCode() const { return countryCode; }
QString Manufacturer::getFullInfo() const { return manufacturerName + " (" + manufacturerCode + ")"; }
bool Manufacturer::isValid() const { return !manufacturerCode.isEmpty() && !manufacturerName.isEmpty(); }

// 📂 Поиск производителя по коду штрих-кода напрямую в файле
QString Manufacturer::findManufacturerByCode(const QString& code)
{
    QString filePath = "C:/Users/rauko/Desktop/Barcode_Manufacturers.txt";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw FileException("Не удалось открыть файл производителей: " + filePath.toStdString());
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;

        QStringList parts = line.split(':');
        if (parts.size() == 3) {
            QString manufacturerCode = parts[0].trimmed();
            QString manufacturerName = parts[1].trimmed();
            QString countryCode      = parts[2].trimmed();

            if (manufacturerCode == code) {
                return manufacturerName + " (" + manufacturerCode + "), страна: " + countryCode;
            }
        }
    }

    return QString("Неизвестный производитель (" + code + ")");
}
