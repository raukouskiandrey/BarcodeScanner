#include "Product.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "FileException.h"
Product::Product(const QString& code, const QString& name, const QString& barcode)
    : productCode(code), productName(name), barcode(barcode) {}

QString Product::getProductCode() const { return productCode; }
QString Product::getProductName() const { return productName; }
QString Product::getBarcode() const { return barcode; }
QString Product::getFullInfo() const { return productName + " [Код: " + productCode + ", ШК: " + barcode + "]"; }
bool Product::isValid() const { return !productName.isEmpty(); }

// 📂 Поиск товара по штрих-коду напрямую в файле
QString Product::findProductByBarcode(const QString& barcode)
{
    QString filePath = "C:/Users/rauko/Desktop/Barcode_Products.txt";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw FileException("Не удалось открыть файл товаров: " + filePath.toStdString());
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;

        QStringList parts = line.split('|');
        if (parts.size() == 3) {
            QString fileBarcode  = parts[0].trimmed();
            QString manufacturer = parts[1].trimmed();
            QString productName  = parts[2].trimmed();

            if (fileBarcode == barcode) {
                return productName + " (Производитель: " + manufacturer + ", ШК: " + fileBarcode + ")";
            }
        }
    }

    return QString("Неизвестный товар (" + barcode + ")");
}
