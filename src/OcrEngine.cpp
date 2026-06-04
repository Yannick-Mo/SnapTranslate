#include "OcrEngine.h"
#include "Utils.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QBuffer>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QUrl>

OcrEngine::OcrEngine(QObject *parent)
    : QObject(parent), m_fetchingToken(false), m_usingFallback(false), m_ocrMode("auto"), m_tianyiEnabled(false)
{
    m_baiduNam = new QNetworkAccessManager(this);
    m_easyNam = new QNetworkAccessManager(this);
    m_tianyiNam = new QNetworkAccessManager(this);
}

void OcrEngine::setCredentials(const QString &apiKey, const QString &secretKey)
{
    m_apiKey = apiKey;
    m_secretKey = secretKey;
    fetchAccessToken();
}

void OcrEngine::setEasyCredentials(const QString &accessKey, const QString &githubUser)
{
    m_easyAccessKey = accessKey;
    m_easyGithubUser = githubUser;
}

void OcrEngine::setTianyiEnabled(bool enabled)
{
    m_tianyiEnabled = enabled;
}

void OcrEngine::setOcrMode(const QString &mode)
{
    m_ocrMode = mode;
}

void OcrEngine::fetchAccessToken()
{
    if (m_fetchingToken) return;
    m_fetchingToken = true;

    QUrl url("https://aip.baidubce.com/oauth/2.0/token");
    QUrlQuery query;
    query.addQueryItem("grant_type", "client_credentials");
    query.addQueryItem("client_id", m_apiKey);
    query.addQueryItem("client_secret", m_secretKey);
    url.setQuery(query);

    QNetworkRequest request(url);
    QNetworkReply *reply = m_baiduNam->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            m_fetchingToken = false;
            emit errorOccurred("获取百度AccessToken失败: " + reply->errorString());
            reply->deleteLater();
            return;
        }
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        handleTokenResponse(doc.object());
        reply->deleteLater();
    });
}

void OcrEngine::recognize(const QPixmap &pixmap)
{
    m_usingFallback = false;
    m_currentPixmap = pixmap;

    if (m_ocrMode == "easyocr") {
        if (m_easyAccessKey.isEmpty()) {
            emit errorOccurred("EasyOCR 未配置，请在 config.json 中填写 easyocr 凭证");
            return;
        }
        recognizeWithEasy(pixmap);
        return;
    }
    if (m_ocrMode == "tianyi") {
        if (!m_tianyiEnabled) {
            emit errorOccurred("天翼OCR 未启用");
            return;
        }
        recognizeWithTianyi(pixmap);
        return;
    }

    if (m_accessToken.isEmpty()) {
        m_pendingPixmap = pixmap;
        fetchAccessToken();
        return;
    }

    QString imageBase64 = Utils::pixmapToBase64Url(pixmap);
    QUrl url("https://aip.baidubce.com/rest/2.0/ocr/v1/accurate_basic");
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("access_token", m_accessToken);
    url.setQuery(urlQuery);

    QByteArray postData;
    postData.append("image=" + imageBase64.toUtf8());
    postData.append("&language_type=CHN_ENG");
    postData.append("&detect_direction=true");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = m_baiduNam->post(request, postData);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onBaiduReplyFinished(reply);
    });
}

void OcrEngine::onBaiduReplyFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        if (m_ocrMode == "auto" && !m_easyAccessKey.isEmpty()) {
            emit fallbackOcr("EasyOCR");
            recognizeWithEasy(m_currentPixmap);
        } else {
            emit errorOccurred("百度OCR网络错误: " + reply->errorString());
        }
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();

    if (obj.contains("access_token")) {
        handleTokenResponse(obj);
    } else {
        handleOcrResponse(obj);
    }
    reply->deleteLater();
}

void OcrEngine::handleTokenResponse(const QJsonObject &obj)
{
    m_fetchingToken = false;
    m_accessToken = obj["access_token"].toString();

    if (!m_pendingPixmap.isNull()) {
        recognize(m_pendingPixmap);
        m_pendingPixmap = QPixmap();
    }
}

void OcrEngine::handleOcrResponse(const QJsonObject &obj)
{
    if (obj.contains("error_code")) {
        int code = obj["error_code"].toInt();
        QString msg = obj["error_msg"].toString();
        if (m_ocrMode == "auto" && !m_easyAccessKey.isEmpty()) {
            emit fallbackOcr("EasyOCR");
            recognizeWithEasy(m_currentPixmap);
        } else {
            emit errorOccurred(QString("百度OCR错误 %1: %2").arg(code).arg(msg));
        }
        return;
    }

    QStringList textLines;
    if (obj.contains("words_result")) {
        QJsonArray words = obj["words_result"].toArray();
        for (const QJsonValue &val : words) {
            QString word = val.toObject()["words"].toString();
            if (!word.isEmpty())
                textLines.append(word);
        }
    }
    emit recognitionFinished(textLines.join("\n"));
}

void OcrEngine::recognizeWithEasy(const QPixmap &pixmap)
{
    m_usingFallback = true;

    QByteArray bytes;
    QBuffer buffer(&bytes);
    pixmap.save(&buffer, "PNG");

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, "image/png");
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader,
        QString("form-data; name=\"file\"; filename=\"screenshot.png\""));
    imagePart.setBody(bytes);
    multiPart->append(imagePart);

    QUrl url("https://console.easyocr.org/api/ocr");
    QNetworkRequest request(url);
    request.setRawHeader("X-Access-Key", m_easyAccessKey.toUtf8());

    QNetworkReply *reply = m_easyNam->post(request, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleEasyReply(reply);
    });
}

void OcrEngine::handleEasyReply(QNetworkReply *reply)
{
    if (!m_usingFallback) {
        reply->deleteLater();
        return;
    }
    m_usingFallback = false;

    if (reply->error() != QNetworkReply::NoError) {
        if (m_ocrMode == "auto" && m_tianyiEnabled) {
            emit fallbackOcr("天翼OCR");
            recognizeWithTianyi(m_currentPixmap);
        } else {
            emit errorOccurred("EasyOCR网络错误: " + reply->errorString());
        }
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();

    if (obj.contains("words")) {
        QJsonArray words = obj["words"].toArray();
        QStringList textLines;
        for (const QJsonValue &val : words) {
            QString text = val.toObject()["text"].toString();
            if (!text.isEmpty())
                textLines.append(text);
        }
        if (!textLines.isEmpty()) {
            emit recognitionFinished(textLines.join("\n"));
        } else {
            if (m_ocrMode == "auto" && m_tianyiEnabled) {
                emit fallbackOcr("天翼OCR");
                recognizeWithTianyi(m_currentPixmap);
            } else {
                emit errorOccurred("EasyOCR: 未识别到文字");
            }
        }
    } else {
        QString msg = obj["message"].toString("未知错误");
        if (m_ocrMode == "auto" && m_tianyiEnabled) {
            emit fallbackOcr("天翼OCR");
            recognizeWithTianyi(m_currentPixmap);
        } else {
            emit errorOccurred("EasyOCR失败: " + msg);
        }
    }
    reply->deleteLater();
}

void OcrEngine::recognizeWithTianyi(const QPixmap &pixmap)
{
    m_usingFallback = true;

    QByteArray bytes;
    QBuffer buffer(&bytes);
    pixmap.save(&buffer, "PNG");

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, "image/png");
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader,
        QString("form-data; name=\"file\"; filename=\"screenshot.png\""));
    imagePart.setBody(bytes);
    multiPart->append(imagePart);

    QUrl url("https://api.tianyiocr.com/v1/ocr");
    QNetworkRequest request(url);

    QNetworkReply *reply = m_tianyiNam->post(request, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleTianyiReply(reply);
    });
}

void OcrEngine::handleTianyiReply(QNetworkReply *reply)
{
    if (!m_usingFallback) {
        reply->deleteLater();
        return;
    }
    m_usingFallback = false;

    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred("天翼OCR网络错误: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();

    if (obj.contains("text")) {
        QString text = obj["text"].toString().trimmed();
        if (!text.isEmpty()) {
            emit recognitionFinished(text);
        } else {
            emit errorOccurred("天翼OCR: 未识别到文字");
        }
    } else {
        emit errorOccurred("天翼OCR: 返回格式异常");
    }
    reply->deleteLater();
}
