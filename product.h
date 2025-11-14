#ifndef PRODUCT_H
#define PRODUCT_H

#include <QString>

class Product
{
private:
    QString productCode;
    QString productName;
    QString barcode;

public:
    Product(const QString& code = "", const QString& name = "", const QString& barcode = "");

    QString getProductCode() const;
    QString getProductName() const;
    QString getBarcode() const;
    QString getFullInfo() const;

    bool isValid() const;
};

#endif // PRODUCT_H
