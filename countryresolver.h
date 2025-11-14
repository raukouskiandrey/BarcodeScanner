#ifndef COUNTRYRESOLVER_H
#define COUNTRYRESOLVER_H

#include "country.h"
#include <QMap>
#include <QString>

class CountryResolver {
private:
    QMap<QString, Country*> countries;

public:
    CountryResolver() = default;
    void setCountries(const QMap<QString, Country*>& countriesMap);
    Country* findCountryByCode(const QString& code);
};

#endif // COUNTRYRESOLVER_H
