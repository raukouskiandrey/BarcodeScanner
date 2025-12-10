#ifndef PRODUCTDATALOADER_H
#define PRODUCTDATALOADER_H

#include "product.h"
#include <QMap>
#include <QString>
#include <string>

class ProductDataLoader
{
public:
    ProductDataLoader();
    ~ProductDataLoader();

    void loadFromFile(const std::string& filePath = "C:/Users/rauko/Desktop/Barcode_Products.txt");
    Product* getProductByBarcode(const QString& barcode);
    Product* getProductByCode(const QString& code);
    int getProductsCount() const { return products.size(); }

private:
    void parseProductLine(const std::string& line);

    QMap<QString, Product*> products;
    QMap<QString, Product*> productsByCode;
};

#endif // PRODUCTDATALOADER_H
