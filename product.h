#ifndef PRODUCT_H
#define PRODUCT_H

#include <QString>

class Product
{
private:
    QString productCode;   // Код товара (последние цифры)
    QString productName;   // Название товара
    QString barcode;       // Полный штрих-код

public:
    Product(const QString& code = "", const QString& name = "", const QString& barcode = "");

    QString getProductCode() const;
    QString getProductName() const;
    QString getBarcode() const;
    QString getFullInfo() const;

    bool isValid() const;

    // 📂 Новый метод: поиск товара по штрих-коду напрямую в файле
    static QString findProductByBarcode(const QString& barcode);
};

#endif // PRODUCT_H
