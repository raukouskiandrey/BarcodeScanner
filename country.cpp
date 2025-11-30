#include "country.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>


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

    // Берём только цифры
    QString digits = barcode;
    digits.remove(QRegularExpression("[^0-9]"));

    if (digits.length() < 2) return QString();

    QString prefix3 = digits.left(3); // первые 3 цифры
    QString prefix2 = digits.left(2); // первые 2 цифры

    QString filePath = "C:/Users/rauko/Desktop/Barcode_Countries.txt";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл:" << filePath;
        return QString();
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;

        int colonPos = line.indexOf(':');
        if (colonPos == -1) continue;

        QString codes = line.left(colonPos).trimmed();
        QString countryName = line.mid(colonPos + 1).trimmed();

        // Диапазон (например "490-499")
        if (codes.contains('-')) {
            QStringList parts = codes.split('-');
            if (parts.size() == 2) {
                bool ok1, ok2;
                int start = parts[0].toInt(&ok1);
                int end = parts[1].toInt(&ok2);

                if (ok1 && ok2) {
                    int pref3 = prefix3.toInt();
                    int pref2 = prefix2.toInt();

                    if ((pref3 >= start && pref3 <= end) || (pref2 >= start && pref2 <= end)) {
                        file.close();
                        return countryName;
                    }
                }
            }
        }
        // Одиночный код (например "528")
        else {
            if (codes == prefix3 || codes == prefix2) {
                file.close();
                return countryName;
            }
        }
    }

    file.close();
    return QString("Неизвестная страна");
}
