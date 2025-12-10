#include "resultsaver.h"

QString ResultSaver::outputDirectory = "C:/Users/rauko/Desktop";

void ResultSaver::saveToMainFile(const BarcodeReader::BarcodeResult& result)
{
    QString filePath = outputDirectory + "/Barcode_All.txt";

    QString content;
    content += "=== " + getTimestamp() + " ===\n";
    content += "Тип: " + QString::fromStdString(result.type) + "\n";
    content += "Цифры: " + QString::fromStdString(result.digits) + "\n";
    content += "Страна: " + QString::fromStdString(result.country) + "\n";
    content += "Код производителя: " + QString::fromStdString(result.manufacturerCode) + "\n";
    content += "Код товара: " + QString::fromStdString(result.productCode) + "\n";
    content += "----------------------------------------\n";

    FileManager::appendToFile(filePath, content);
}

void ResultSaver::saveToCountryFile(const BarcodeReader::BarcodeResult& result)
{
    if (result.country.empty() || result.country == "Неизвестно") {
        return;
    }

    QString filePath = outputDirectory + "/Barcode_Countries.txt";
    QString content = "[" + getTimestamp() + "] " +
                      QString::fromStdString(result.type) + " " +
                      QString::fromStdString(result.digits) +
                      " -> " + QString::fromStdString(result.country) + "\n";

    FileManager::appendToFile(filePath, content);
}

void ResultSaver::saveToManufacturerFile(const BarcodeReader::BarcodeResult& result)
{
    if (result.manufacturerCode.empty() ||
        result.manufacturerCode == "Н/Д" ||
        result.manufacturerCode == "Нет") {
        return;
    }

    QString filePath = outputDirectory + "/Barcode_Manufacturers.txt";
    QString content = "[" + getTimestamp() + "] " +
                      QString::fromStdString(result.type) + " " +
                      QString::fromStdString(result.digits) +
                      " -> Код производителя: " +
                      QString::fromStdString(result.manufacturerCode) +
                      " (Страна: " + QString::fromStdString(result.country) + ")\n";

    FileManager::appendToFile(filePath, content);
}

void ResultSaver::saveToProductFile(const BarcodeReader::BarcodeResult& result)
{
    if (result.productCode.empty() || result.productCode == "Н/Д") {
        return;
    }

    QString filePath = outputDirectory + "/Barcode_Products.txt";
    QString content = "[" + getTimestamp() + "] " +
                      QString::fromStdString(result.type) + " " +
                      QString::fromStdString(result.digits) +
                      " -> Код товара: " +
                      QString::fromStdString(result.productCode) +
                      " (Производитель: " +
                      QString::fromStdString(result.manufacturerCode) + ")\n";

    FileManager::appendToFile(filePath, content);
}

void ResultSaver::saveToStatisticsFile(const BarcodeReader::BarcodeResult& result)
{
    QString filePath = outputDirectory + "/Barcode_Statistics.txt";

    // Чтение существующей статистики
    QMap<QString, int> countryStats;
    QMap<QString, int> typeStats;

    if (FileManager::fileExists(filePath)) {
        QString content = FileManager::readFile(filePath);
        QTextStream stream(&content);
        QString line;

        while (stream.readLineInto(&line)) {
            if (line.startsWith("Страна:")) {
                int colonPos = line.indexOf(':');
                if (colonPos != -1) {
                    QString countryPart = line.mid(colonPos + 2);
                    int dashPos = countryPart.indexOf(" -");
                    if (dashPos != -1) {
                        QString country = countryPart.left(dashPos);
                        int count = countryPart.section(' ', -1).toInt();
                        countryStats[country] = count;
                    }
                }
            } else if (line.startsWith("Тип:")) {
                int colonPos = line.indexOf(':');
                if (colonPos != -1) {
                    QString typePart = line.mid(colonPos + 2);
                    int dashPos = typePart.indexOf(" -");
                    if (dashPos != -1) {
                        QString type = typePart.left(dashPos);
                        int count = typePart.section(' ', -1).toInt();
                        typeStats[type] = count;
                    }
                }
            }
        }
    }

    // Обновление статистики
    updateStatistics(result, countryStats, typeStats);

    // Сохранение обновленной статистики
    QString content;
    content += "=== СТАТИСТИКА СКАНИРОВАНИЙ ===\n";
    content += "Обновлено: " + getTimestamp() + "\n\n";

    content += "ПО СТРАНАМ:\n";
    for (auto it = countryStats.begin(); it != countryStats.end(); ++it) {
        content += "Страна: " + it.key() + " - " + QString::number(it.value()) + " шт.\n";
    }

    content += "\nПО ТИПАМ:\n";
    for (auto it = typeStats.begin(); it != typeStats.end(); ++it) {
        content += "Тип: " + it.key() + " - " + QString::number(it.value()) + " шт.\n";
    }

    FileManager::writeFile(filePath, content);
}

void ResultSaver::setOutputDirectory(const QString& directory)
{
    outputDirectory = directory;
    FileManager::createDirectory(directory);
}

QString ResultSaver::getOutputDirectory()
{
    return outputDirectory;
}

QString ResultSaver::getTimestamp()
{
    return QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
}

QString ResultSaver::formatResultLine(const BarcodeReader::BarcodeResult& result)
{
    return "[" + getTimestamp() + "] " +
           QString::fromStdString(result.type) + " " +
           QString::fromStdString(result.digits);
}

void ResultSaver::updateStatistics(const BarcodeReader::BarcodeResult& result,
                                   QMap<QString, int>& countryStats,
                                   QMap<QString, int>& typeStats)
{
    QString country = QString::fromStdString(result.country);
    QString type = QString::fromStdString(result.type);

    countryStats[country]++;
    typeStats[type]++;
}
