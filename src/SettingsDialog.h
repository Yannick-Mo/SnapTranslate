#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QJsonObject>

class QLineEdit;
class QComboBox;
class QCheckBox;
class QKeySequenceEdit;

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    QString hotkey() const;
    QString targetLanguage() const;
    QString ocrEngine() const;
    bool autoCopy() const;
    bool showNotification() const;
    bool clipboardMonitor() const;

    void setHotkey(const QString &key);
    void setTargetLanguage(const QString &lang);
    void setOcrEngine(const QString &mode);
    void setAutoCopy(bool val);
    void setShowNotification(bool val);
    void setClipboardMonitor(bool val);

    static QJsonObject loadSettings();
    static bool saveSettings(const QJsonObject &settings);

private:
    void setupUi();

    QKeySequenceEdit *m_hotkeyEdit;
    QComboBox *m_langCombo;
    QComboBox *m_ocrEngineCombo;
    QCheckBox *m_autoCopyCheck;
    QCheckBox *m_notifyCheck;
    QCheckBox *m_clipboardCheck;
};

#endif // SETTINGSDIALOG_H
