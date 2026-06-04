#include "TranslateEngine.h"
#include "Utils.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRandomGenerator>

TranslateEngine::TranslateEngine(QObject *parent) : QObject(parent)
{
    m_nam = new QNetworkAccessManager(this);
    connect(m_nam, &QNetworkAccessManager::finished, this, &TranslateEngine::onReplyFinished);
}

void TranslateEngine::setCredentials(const QString &appId, const QString &secretKey)
{
    m_appId = appId;
    m_secretKey = secretKey;
}

void TranslateEngine::translate(const QString &text, const QString &targetLang)
{
    if (text.isEmpty()) {
        emit errorOccurred("待翻译文本为空");
        return;
    }

    int salt = QRandomGenerator::global()->bounded(0, 2147483647);
    QString sign = generateSign(m_appId, text, QString::number(salt), m_secretKey);

    QUrl url("https://fanyi-api.baidu.com/api/trans/vip/translate");
    QUrlQuery query;
    query.addQueryItem("q", text);
    query.addQueryItem("from", "auto");
    query.addQueryItem("to", targetLang);
    query.addQueryItem("appid", m_appId);
    query.addQueryItem("salt", QString::number(salt));
    query.addQueryItem("sign", sign);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QByteArray postData = query.toString(QUrl::FullyEncoded).toUtf8();
    m_nam->post(request, postData);
}

QString TranslateEngine::generateSign(const QString &appId, const QString &query,
                                       const QString &salt, const QString &secretKey)
{
    QString str = appId + query + salt + secretKey;
    return Utils::md5(str.toUtf8());
}

void TranslateEngine::onReplyFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();

    if (obj.contains("error_code")) {
        int code = obj["error_code"].toInt();
        QString msg = obj["error_msg"].toString();
        emit errorOccurred(QString("翻译API错误 %1: %2").arg(code).arg(msg));
        reply->deleteLater();
        return;
    }

    QString srcText, dstText;
    if (obj.contains("trans_result")) {
        QJsonArray results = obj["trans_result"].toArray();
        if (!results.isEmpty()) {
            QJsonObject first = results[0].toObject();
            srcText = first["src"].toString();
            dstText = first["dst"].toString();
        }
    }
    emit translationFinished(srcText, dstText);
    reply->deleteLater();
}
