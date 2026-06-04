#include "MainController.h"
#include "TrayIcon.h"
#include "HotkeyManager.h"
#include "OcrEngine.h"
#include "TranslateEngine.h"
#include "ClipboardListener.h"
#include "ScreenshotWidget.h"
#include "ResultWidget.h"
#include "StickyNoteWidget.h"
#include "SettingsDialog.h"
#include "HistoryDialog.h"
#include "Utils.h"
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QJsonObject>
#include <QDateTime>

MainController::MainController(QObject *parent) : QObject(parent)
{
    m_trayIcon = new TrayIcon(this);
    m_hotkeyMgr = new HotkeyManager(this);
    m_ocr = new OcrEngine(this);
    m_translator = new TranslateEngine(this);
    m_clipListener = new ClipboardListener(this);
    m_historyDlg = nullptr;
    m_currentResult = nullptr;
    m_ignoreClipboardOnce = false;

    connect(m_hotkeyMgr, &HotkeyManager::activated, this, &MainController::onHotkeyActivated);
    connect(m_ocr, &OcrEngine::recognitionFinished, this, &MainController::onOcrFinished);
    connect(m_translator, &TranslateEngine::translationFinished, this, &MainController::onTranslationFinished);
    connect(m_ocr, &OcrEngine::errorOccurred, this, &MainController::onOcrError);
    connect(m_ocr, &OcrEngine::fallbackOcr, this, [this](const QString &provider) {
        if (m_currentResult) {
            m_currentResult->setOriginalText(
                QString("百度OCR失败，正在使用 %1 重试…").arg(provider));
        }
    });
    connect(m_translator, &TranslateEngine::errorOccurred, this, &MainController::onTranslateError);
    connect(m_clipListener, &ClipboardListener::textChanged, this, &MainController::onClipboardTextChanged);
    connect(m_clipListener, &ClipboardListener::imageChanged, this, &MainController::onClipboardImageChanged);
}

void MainController::start()
{
    loadConfig();
    setupTrayIcon();
    setupHotkey();
    setupClipboardListener();
    m_trayIcon->show();

    if (m_showNotification) {
        m_trayIcon->showNotify("SnapTranslate", "程序已启动，按 Alt+Q 截图翻译");
    }
}

void MainController::loadConfig()
{
    QJsonObject config = Utils::loadJsonConfig("config.json");
    if (config.isEmpty()) {
        QMessageBox::critical(nullptr, "配置错误",
            "无法读取 config.json，请确保该文件存在且格式正确。");
        qApp->quit();
        return;
    }

    QJsonObject ocrObj = config["ocr"].toObject();
    QJsonObject transObj = config["translate"].toObject();
    QJsonObject generalObj = config["general"].toObject();

    QString ocrKey = ocrObj["api_key"].toString();
    QString ocrSecret = ocrObj["secret_key"].toString();
    QString transAppId = transObj["app_id"].toString();
    QString transSecret = transObj["secret_key"].toString();

    if (ocrKey.isEmpty() || transAppId.isEmpty()) {
        QMessageBox::critical(nullptr, "配置错误",
            "请在 config.json 中填写有效的 API 凭证");
        qApp->quit();
        return;
    }

    m_ocr->setCredentials(ocrKey, ocrSecret);
    m_ocr->setOcrMode(ocrObj["engine"].toString("auto"));

    QJsonObject easyOcr = ocrObj["easyocr"].toObject();
    QString easyKey = easyOcr["access_key"].toString();
    QString easyUser = easyOcr["github_user"].toString();
    if (!easyKey.isEmpty()) {
        m_ocr->setEasyCredentials(easyKey, easyUser);
    }

    QJsonObject tianyiCfg = ocrObj["tianyiocr"].toObject();
    m_ocr->setTianyiEnabled(tianyiCfg["enabled"].toBool(false));

    m_translator->setCredentials(transAppId, transSecret);

    m_targetLanguage = generalObj["target_language"].toString("zh");
    m_autoCopy = generalObj["auto_copy"].toBool(true);
    m_showNotification = generalObj["show_notification"].toBool(true);
    m_clipboardMonitor = generalObj["clipboard_monitor"].toBool(true);
}

void MainController::setupTrayIcon()
{
    connect(m_trayIcon, &TrayIcon::triggerScreenshot,
            this, &MainController::onHotkeyActivated);
    connect(m_trayIcon, &TrayIcon::showSettings,
            this, &MainController::onShowSettings);
    connect(m_trayIcon, &TrayIcon::showHistory,
            this, &MainController::onShowHistory);
    connect(m_trayIcon, &TrayIcon::quitApp,
            this, &MainController::onQuit);
}

void MainController::setupHotkey()
{
    QJsonObject config = Utils::loadJsonConfig("config.json");
    QString hotkeyStr = config["general"].toObject()["hotkey"].toString("Alt+Q");
    bool ok = m_hotkeyMgr->registerHotkey(QKeySequence(hotkeyStr));
    if (!ok) {
        m_trayIcon->showNotify("热键注册失败",
            QString("无法注册热键 %1，可能与其他程序冲突").arg(hotkeyStr),
            QSystemTrayIcon::Warning);
    }
}

void MainController::setupClipboardListener()
{
    if (m_clipboardMonitor) {
        m_clipListener->start(300);
    }
}

void MainController::onHotkeyActivated()
{
    ScreenshotWidget *screenshot = new ScreenshotWidget(nullptr);
    connect(screenshot, &ScreenshotWidget::screenshotTaken,
            this, &MainController::onScreenshotTaken);
    connect(screenshot, &ScreenshotWidget::canceled,
            screenshot, &QObject::deleteLater);
}

void MainController::onScreenshotTaken(const QPixmap &pixmap)
{
    showResultWidget("识别中...");
    m_ocr->recognize(pixmap);
}

void MainController::onOcrFinished(const QString &text)
{
    if (text.isEmpty()) {
        if (m_currentResult) {
            m_currentResult->setOriginalText("（未识别到文字）");
        }
        if (m_showNotification) {
            m_trayIcon->showNotify("OCR 结果", "未识别到文字");
        }
        return;
    }
    m_lastRecognizedText = text;
    if (m_currentResult) {
        m_currentResult->setOriginalText(text);
    }
    m_translator->translate(text, m_targetLanguage);
}

void MainController::onTranslationFinished(const QString &original, const QString &translated)
{
    if (m_currentResult) {
        m_currentResult->setTranslatedText(translated);
    }

    if (m_autoCopy) {
        m_ignoreClipboardOnce = true;
        QGuiApplication::clipboard()->setText(translated);
        if (m_showNotification) {
            m_trayIcon->showNotify("翻译完成", "译文已复制到剪贴板");
        }
    }

    // Save to history
    if (!m_historyDlg) {
        m_historyDlg = new HistoryDialog(nullptr);
    }
    HistoryEntry entry;
    entry.originalText = original;
    entry.translatedText = translated;
    entry.sourceLang = "auto";
    entry.targetLang = m_targetLanguage;
    entry.timestamp = QDateTime::currentDateTime();
    m_historyDlg->addEntry(entry);
}

void MainController::showResultWidget(const QString &originalHint)
{
    if (m_currentResult) {
        m_currentResult->close();
        m_currentResult->deleteLater();
    }

    m_currentResult = new ResultWidget;
    if (!originalHint.isEmpty()) {
        m_currentResult->setOriginalText(originalHint);
    }
    m_currentResult->showNearCursor();
    connect(m_currentResult, &ResultWidget::closed, this, [this]() {
        m_currentResult = nullptr;
    });
    connect(m_currentResult, &ResultWidget::pinRequested, this, &MainController::onPinRequested);
}

void MainController::onPinRequested(const QString &text)
{
    StickyNoteWidget *note = new StickyNoteWidget(text);
    note->move(QCursor::pos() - QPoint(100, 20));
    note->show();
}

void MainController::onOcrError(const QString &msg)
{
    if (m_currentResult) {
        m_currentResult->setOriginalText(QString("识别失败: %1").arg(msg));
    }
    if (m_showNotification) {
        m_trayIcon->showNotify("OCR 错误", msg, QSystemTrayIcon::Critical);
    }
}

void MainController::onTranslateError(const QString &msg)
{
    if (m_currentResult) {
        m_currentResult->setTranslatedText(QString("翻译失败: %1").arg(msg));
    }
    if (m_showNotification) {
        m_trayIcon->showNotify("翻译错误", msg, QSystemTrayIcon::Critical);
    }
}

void MainController::onClipboardTextChanged(const QString &text)
{
    if (m_ignoreClipboardOnce) {
        m_ignoreClipboardOnce = false;
        return;
    }
    m_translator->translate(text, m_targetLanguage);
}

void MainController::onClipboardImageChanged(const QPixmap &pix)
{
    m_ocr->recognize(pix);
}

void MainController::onShowSettings()
{
    SettingsDialog dlg;

    QJsonObject config = Utils::loadJsonConfig("config.json");
    QJsonObject general = config["general"].toObject();

    QJsonObject ocrCfg = config["ocr"].toObject();
    dlg.setOcrEngine(ocrCfg["engine"].toString("auto"));
    dlg.setHotkey(general["hotkey"].toString("Alt+Q"));
    dlg.setTargetLanguage(general["target_language"].toString("zh"));
    dlg.setAutoCopy(general["auto_copy"].toBool(true));
    dlg.setShowNotification(general["show_notification"].toBool(true));
    dlg.setClipboardMonitor(general["clipboard_monitor"].toBool(true));

    if (dlg.exec() == QDialog::Accepted) {
        QJsonObject newGeneral;
        newGeneral["hotkey"] = dlg.hotkey();
        newGeneral["target_language"] = dlg.targetLanguage();
        newGeneral["auto_copy"] = dlg.autoCopy();
        newGeneral["show_notification"] = dlg.showNotification();
        newGeneral["clipboard_monitor"] = dlg.clipboardMonitor();

        config["general"] = newGeneral;

        ocrCfg["engine"] = dlg.ocrEngine();
        config["ocr"] = ocrCfg;

        SettingsDialog::saveSettings(config);

        m_targetLanguage = dlg.targetLanguage();
        m_autoCopy = dlg.autoCopy();
        m_showNotification = dlg.showNotification();
        m_clipboardMonitor = dlg.clipboardMonitor();
        m_ocr->setOcrMode(dlg.ocrEngine());

        setupHotkey();
        setupClipboardListener();

        m_trayIcon->showNotify("设置已保存", "配置已更新");
    }
}

void MainController::onShowHistory()
{
    if (!m_historyDlg) {
        m_historyDlg = new HistoryDialog(nullptr);
    }
    m_historyDlg->show();
    m_historyDlg->raise();
    m_historyDlg->activateWindow();
}

void MainController::onQuit()
{
    qApp->quit();
}
