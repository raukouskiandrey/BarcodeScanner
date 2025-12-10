#include "productresolver.h"

void ProductResolver::setProducts(const QMap<QString, Product*>& productsMap,
                                  const QMap<QString, Product*>& productsByCodeMap) {
    products = productsMap;
    productsByCode = productsByCodeMap;
}

Product* ProductResolver::findProductByBarcode(const QString& barcode) {
    return products.contains(barcode) ? products[barcode] : nullptr;
}

Product* ProductResolver::findProductByCode(const QString& code) {
    return productsByCode.contains(code) ? productsByCode[code] : nullptr;
}
