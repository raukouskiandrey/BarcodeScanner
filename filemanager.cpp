#include "filemanager.h"

bool FileManager::fileExists(const QString& path)
{
    QFile file(path);
    return file.exists();
}

QString FileManager::readFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file for reading:" << path;
        return QString();
    }

    QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    in.setCodec("UTF-8");
#else
    in.setEncoding(QStringConverter::Utf8);
#endif
    QString content = in.readAll();
    file.close();

    return content;
}

bool FileManager::writeFile(const QString& path, const QString& content)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file for writing:" << path;
        return false;
    }

    QTextStream out(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#else
    out.setEncoding(QStringConverter::Utf8);
#endif
    out << content;
    file.close();

    return true;
}

bool FileManager::appendToFile(const QString& path, const QString& content)
{
    QFile file(path);
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Cannot open file for appending:" << path;
        return false;
    }

    QTextStream out(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#else
    out.setEncoding(QStringConverter::Utf8);
#endif
    out << content;
    file.close();

    return true;
}

bool FileManager::createDirectory(const QString& path)
{
    QDir dir;
    return dir.mkpath(path);
}
