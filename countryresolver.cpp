#include "countryresolver.h"

void CountryResolver::setCountries(const QMap<QString, Country*>& countriesMap) {
    countries = countriesMap;
}

Country* CountryResolver::findCountryByCode(const QString& code) {
    if (countries.contains(code)) {
        return countries[code];
    }

    for (int len = code.length() - 1; len >= 2; len--) {
        QString partialCode = code.left(len);
        if (countries.contains(partialCode)) {
            return countries[partialCode];
        }
    }

    return nullptr;
}
