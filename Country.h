#ifndef COUNTRY_H
#define COUNTRY_H

#include <QString>

class Country
{
private:
    QString countryCode;
    QString countryName;

public:
    Country(const QString& code = "", const QString& name = "");

    QString getCountryCode() const;
    QString getCountryName() const;
    QString getFullInfo() const;

    bool isValid() const;

    // 📂 Новый метод: поиск страны по первым 2–3 цифрам штрих-кода
    static QString findCountryByBarcode(const QString& barcode);
};

#endif // COUNTRY_H
