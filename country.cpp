#include "country.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>

#include "FileException.h"
Country::Country(const QString& code, const QString& name)
    : countryCode(code), countryName(name) {}

QString Country::getCountryCode() const { return countryCode; }
QString Country::getCountryName() const { return countryName; }
QString Country::getFullInfo() const { return countryName + " (" + countryCode + ")"; }
bool Country::isValid() const { return !countryCode.isEmpty() && !countryName.isEmpty(); }

// 📂 Поиск страны по первым 2–3 цифрам штрих-кода
QString Country::findCountryByBarcode(const QString& barcode)
{
    if (barcode.isEmpty()) return QString();

    QString digits = barcode;
    digits.remove(QRegularExpression("[^0-9]"));
    if (digits.length() < 2) return QString();

    QString prefix3 = digits.left(3);
    QString prefix2 = digits.left(2);

    QString filePath = "C:/Users/rauko/Desktop/Barcode_Countries.txt";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw FileException("Не удалось открыть файл стран: " + filePath.toStdString());
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;

        int colonPos = line.indexOf(':');
        if (colonPos == -1) continue;

        QString codes = line.left(colonPos).trimmed();
        QString countryName = line.mid(colonPos + 1).trimmed();

        if (codes.contains('-')) {
            QStringList parts = codes.split('-');
            if (parts.size() == 2) {
                int start = parts[0].toInt();
                int end   = parts[1].toInt();
                int pref3 = prefix3.toInt();
                int pref2 = prefix2.toInt();

                if ((pref3 >= start && pref3 <= end) || (pref2 >= start && pref2 <= end)) {
                    return countryName;
                }
            }
        } else {
            if (codes == prefix3 || codes == prefix2) {
                return countryName;
            }
        }
    }

    return QString("Неизвестная страна");
}
