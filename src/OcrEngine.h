#ifndef OCRENGINE_H
#define OCRENGINE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QPixmap>

class QNetworkReply;

class OcrEngine : public QObject
{
    Q_OBJECT
public:
    explicit OcrEngine(QObject *parent = nullptr);
    void setCredentials(const QString &apiKey, const QString &secretKey);
    void setEasyCredentials(const QString &accessKey, const QString &githubUser);
    void setTianyiEnabled(bool enabled);
    void setOcrMode(const QString &mode);
    void recognize(const QPixmap &pixmap);

signals:
    void recognitionFinished(const QString &text);
    void errorOccurred(const QString &errorMsg);
    void fallbackOcr(const QString &providerName);

private slots:
    void onBaiduReplyFinished(QNetworkReply *reply);

private:
    void fetchAccessToken();
    void handleTokenResponse(const QJsonObject &obj);
    void handleOcrResponse(const QJsonObject &obj);
    void recognizeWithEasy(const QPixmap &pixmap);
    void handleEasyReply(QNetworkReply *reply);
    void recognizeWithTianyi(const QPixmap &pixmap);
    void handleTianyiReply(QNetworkReply *reply);

    QNetworkAccessManager *m_baiduNam;
    QNetworkAccessManager *m_easyNam;
    QNetworkAccessManager *m_tianyiNam;

    QString m_apiKey;
    QString m_secretKey;
    QString m_accessToken;
    QPixmap m_pendingPixmap;
    bool m_fetchingToken;

    QString m_easyAccessKey;
    QString m_easyGithubUser;
    bool m_tianyiEnabled;
    QPixmap m_currentPixmap;
    QString m_ocrMode;
    bool m_usingFallback;
};

#endif // OCRENGINE_H
