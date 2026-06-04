#include "Utils.h"
#include <QBuffer>
#include <QUrl>
#include <QFile>
#include <QJsonDocument>

namespace Utils {

QString pixmapToBase64Url(const QPixmap &pixmap)
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    pixmap.save(&buffer, "PNG");
    QString base64 = byteArray.toBase64();
    return QUrl::toPercentEncoding(base64);
}

QString md5(const QByteArray &data)
{
    return QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();
}

QJsonObject loadJsonConfig(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return QJsonObject();
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    return doc.object();
}

bool saveJsonConfig(const QString &path, const QJsonObject &obj)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    QJsonDocument doc(obj);
    return file.write(doc.toJson()) != -1;
}

} // namespace Utils
