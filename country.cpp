#include "country.h"

Country::Country(const QString& code, const QString& name)
    : countryCode(code), countryName(name)
{
}

QString Country::getCountryCode() const {
    return countryCode;
}

QString Country::getCountryName() const {
    return countryName;
}

QString Country::getFullInfo() const {
    return countryName + " (" + countryCode + ")";
}

bool Country::isValid() const {
    return !countryCode.isEmpty() && !countryName.isEmpty();
}
