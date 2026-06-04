#ifndef TRANSLATEENGINE_H
#define TRANSLATEENGINE_H

#include <QObject>
#include <QNetworkAccessManager>

class QNetworkReply;

class TranslateEngine : public QObject
{
    Q_OBJECT
public:
    explicit TranslateEngine(QObject *parent = nullptr);
    void setCredentials(const QString &appId, const QString &secretKey);
    void translate(const QString &text, const QString &targetLang = "zh");

signals:
    void translationFinished(const QString &original, const QString &translated);
    void errorOccurred(const QString &errorMsg);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QString generateSign(const QString &appId, const QString &query,
                         const QString &salt, const QString &secretKey);
    QNetworkAccessManager *m_nam;
    QString m_appId;
    QString m_secretKey;
};

#endif // TRANSLATEENGINE_H
