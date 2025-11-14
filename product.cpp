#include "product.h"

Product::Product(const QString& code, const QString& name, const QString& barcode)
    : productCode(code), productName(name), barcode(barcode)
{
}

QString Product::getProductCode() const {
    return productCode;
}

QString Product::getProductName() const {
    return productName;
}

QString Product::getBarcode() const {
    return barcode;
}

QString Product::getFullInfo() const {
    return productName + " [Код: " + productCode + ", ШК: " + barcode + "]";
}

bool Product::isValid() const {
    return !productName.isEmpty();
}
