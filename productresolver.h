#ifndef PRODUCTRESOLVER_H
#define PRODUCTRESOLVER_H

#include "product.h"
#include <QMap>
#include <QString>

class ProductResolver {
private:
    QMap<QString, Product*> products;
    QMap<QString, Product*> productsByCode;

public:
    ProductResolver() = default;
    void setProducts(const QMap<QString, Product*>& productsMap,
                     const QMap<QString, Product*>& productsByCodeMap);
    Product* findProductByBarcode(const QString& barcode);
    Product* findProductByCode(const QString& code);
};

#endif // PRODUCTRESOLVER_H
