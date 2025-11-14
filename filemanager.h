#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDebug>

class FileManager
{
public:
    static bool fileExists(const QString& path);
    static QString readFile(const QString& path);
    static bool writeFile(const QString& path, const QString& content);
    static bool appendToFile(const QString& path, const QString& content);
    static bool createDirectory(const QString& path);
};

#endif // FILEMANAGER_H
