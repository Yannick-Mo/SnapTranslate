#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QPixmap>
#include <QSystemTrayIcon>

class HotkeyManager;
class OcrEngine;
class TranslateEngine;
class ClipboardListener;
class TrayIcon;
class HistoryDialog;
class ResultWidget;

class MainController : public QObject
{
    Q_OBJECT
public:
    explicit MainController(QObject *parent = nullptr);
    void start();

private slots:
    void onHotkeyActivated();
    void onScreenshotTaken(const QPixmap &pixmap);
    void onOcrFinished(const QString &text);
    void onTranslationFinished(const QString &original, const QString &translated);
    void onOcrError(const QString &msg);
    void onTranslateError(const QString &msg);
    void onClipboardTextChanged(const QString &text);
    void onClipboardImageChanged(const QPixmap &pix);
    void onShowSettings();
    void onShowHistory();
    void onPinRequested(const QString &text);
    void onQuit();

private:
    void loadConfig();
    void setupTrayIcon();
    void setupHotkey();
    void setupClipboardListener();
    void showResultWidget(const QString &originalHint = QString());

    TrayIcon *m_trayIcon;
    HotkeyManager *m_hotkeyMgr;
    OcrEngine *m_ocr;
    TranslateEngine *m_translator;
    ClipboardListener *m_clipListener;
    HistoryDialog *m_historyDlg;
    ResultWidget *m_currentResult;
    QString m_targetLanguage;
    bool m_autoCopy;
    bool m_showNotification;
    bool m_clipboardMonitor;
    QString m_lastRecognizedText;
    bool m_ignoreClipboardOnce;
};

#endif // MAINCONTROLLER_H
