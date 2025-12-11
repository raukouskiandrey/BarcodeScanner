#ifndef MANUFACTURER_H
#define MANUFACTURER_H

#include <QString>

class Manufacturer
{
private:
    QString manufacturerCode;   // Код производителя
    QString manufacturerName;   // Название производителя
    QString countryCode;        // Код страны

public:
    Manufacturer(const QString& code = "", const QString& name = "", const QString& country = "");

    QString getManufacturerCode() const;
    QString getManufacturerName() const;
    QString getCountryCode() const;
    QString getFullInfo() const;

    bool isValid() const;

    // 📂 Новый метод: поиск производителя по коду штрих-кода напрямую в файле
    static QString findManufacturerByCode(const QString& code);
};

#endif // MANUFACTURER_H
