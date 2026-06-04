#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QPixmap>
#include <QJsonObject>
#include <QCryptographicHash>

namespace Utils {

QString pixmapToBase64Url(const QPixmap &pixmap);
QString md5(const QByteArray &data);
QJsonObject loadJsonConfig(const QString &path);
bool saveJsonConfig(const QString &path, const QJsonObject &obj);

} // namespace Utils

#endif // UTILS_H
