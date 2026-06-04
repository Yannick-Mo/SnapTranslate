#include "SettingsDialog.h"
#include "Utils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QKeySequenceEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QJsonObject>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("设置");
    setFixedSize(420, 340);
    setupUi();
}

void SettingsDialog::setupUi()
{
    setStyleSheet(
        "QDialog { background-color: #1a1a2e; color: #e0e0e0; }"
        "QLabel { color: #c0c0d0; }"
        "QGroupBox { color: #00d4ff; font-weight: bold; border: 1px solid #2a2a4a; border-radius: 8px; margin-top: 12px; padding-top: 16px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; }"
        "QComboBox { background: #16213e; color: #e0e0e0; border: 1px solid #2a2a4a; border-radius: 4px; padding: 4px 8px; }"
        "QComboBox::drop-down { border: none; }"
        "QComboBox QAbstractItemView { background: #16213e; color: #e0e0e0; selection-background-color: #0f3460; }"
        "QCheckBox { color: #c0c0d0; spacing: 8px; }"
        "QCheckBox::indicator { width: 16px; height: 16px; border-radius: 3px; border: 1px solid #3a3a5a; }"
        "QCheckBox::indicator:checked { background: #00d4ff; border-color: #00d4ff; }"
        "QKeySequenceEdit { background: #16213e; color: #e0e0e0; border: 1px solid #2a2a4a; border-radius: 4px; padding: 4px 8px; }"
        "QPushButton { background: #0f3460; color: white; border: none; border-radius: 6px; padding: 8px 24px; font-weight: bold; }"
        "QPushButton:hover { background: #1a4a8a; }"
        "QPushButton#btnCancel { background: #2a2a4a; }"
        "QPushButton#btnCancel:hover { background: #3a3a5a; }"
    );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);

    QTabWidget *tabs = new QTabWidget;
    tabs->setStyleSheet(
        "QTabWidget::pane { border: none; background: transparent; }"
        "QTabBar::tab { background: #16213e; color: #8888aa; padding: 8px 20px; border-radius: 6px 6px 0 0; margin-right: 2px; }"
        "QTabBar::tab:selected { background: #0f3460; color: white; }"
        "QTabBar::tab:hover { color: #e0e0e0; }"
    );

    // === General Tab ===
    QWidget *generalTab = new QWidget;
    QVBoxLayout *generalLayout = new QVBoxLayout(generalTab);
    generalLayout->setSpacing(12);

    QGroupBox *hotkeyGroup = new QGroupBox("全局热键");
    QVBoxLayout *hotkeyLayout = new QVBoxLayout(hotkeyGroup);
    m_hotkeyEdit = new QKeySequenceEdit;
    m_hotkeyEdit->setClearButtonEnabled(true);
    hotkeyLayout->addWidget(new QLabel("点击下方输入框后按下快捷键:"));
    hotkeyLayout->addWidget(m_hotkeyEdit);
    generalLayout->addWidget(hotkeyGroup);

    QGroupBox *langGroup = new QGroupBox("翻译目标语言");
    QVBoxLayout *langLayout = new QVBoxLayout(langGroup);
    m_langCombo = new QComboBox;
    m_langCombo->addItem("中文", "zh");
    m_langCombo->addItem("英文", "en");
    m_langCombo->addItem("日语", "jp");
    m_langCombo->addItem("韩语", "kor");
    m_langCombo->addItem("法语", "fra");
    m_langCombo->addItem("德语", "de");
    m_langCombo->addItem("西班牙语", "spa");
    langLayout->addWidget(m_langCombo);
    generalLayout->addWidget(langGroup);
    generalLayout->addStretch();

    // === Feature Tab ===
    QWidget *featureTab = new QWidget;
    QVBoxLayout *featureLayout = new QVBoxLayout(featureTab);
    featureLayout->setSpacing(8);

    m_autoCopyCheck = new QCheckBox("自动复制译文到剪贴板");
    m_notifyCheck = new QCheckBox("显示系统通知");
    m_clipboardCheck = new QCheckBox("监听剪贴板（自动翻译复制的文本）");

    featureLayout->addWidget(m_autoCopyCheck);
    featureLayout->addWidget(m_notifyCheck);
    featureLayout->addWidget(m_clipboardCheck);

    QGroupBox *ocrGroup = new QGroupBox("OCR 引擎");
    QVBoxLayout *ocrLayout = new QVBoxLayout(ocrGroup);
    m_ocrEngineCombo = new QComboBox;
    m_ocrEngineCombo->addItem("自动（百度→EasyOCR→天翼OCR）", "auto");
    m_ocrEngineCombo->addItem("仅百度 OCR", "baidu");
    m_ocrEngineCombo->addItem("仅 EasyOCR", "easyocr");
    m_ocrEngineCombo->addItem("仅天翼OCR", "tianyi");
    ocrLayout->addWidget(m_ocrEngineCombo);
    featureLayout->addWidget(ocrGroup);

    featureLayout->addStretch();

    tabs->addTab(generalTab, "通用");
    tabs->addTab(featureTab, "功能");
    mainLayout->addWidget(tabs);

    // === Buttons ===
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    QPushButton *cancelBtn = new QPushButton("取消");
    cancelBtn->setObjectName("btnCancel");
    QPushButton *saveBtn = new QPushButton("保存");
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);
    mainLayout->addLayout(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, this, &QDialog::accept);
}

QString SettingsDialog::hotkey() const { return m_hotkeyEdit->keySequence().toString(); }
QString SettingsDialog::targetLanguage() const { return m_langCombo->currentData().toString(); }
QString SettingsDialog::ocrEngine() const { return m_ocrEngineCombo->currentData().toString(); }
bool SettingsDialog::autoCopy() const { return m_autoCopyCheck->isChecked(); }
bool SettingsDialog::showNotification() const { return m_notifyCheck->isChecked(); }
bool SettingsDialog::clipboardMonitor() const { return m_clipboardCheck->isChecked(); }

void SettingsDialog::setHotkey(const QString &key) { m_hotkeyEdit->setKeySequence(QKeySequence(key)); }
void SettingsDialog::setTargetLanguage(const QString &lang) {
    int idx = m_langCombo->findData(lang);
    if (idx >= 0) m_langCombo->setCurrentIndex(idx);
}
void SettingsDialog::setOcrEngine(const QString &mode) {
    int idx = m_ocrEngineCombo->findData(mode);
    if (idx >= 0) m_ocrEngineCombo->setCurrentIndex(idx);
}
void SettingsDialog::setAutoCopy(bool val) { m_autoCopyCheck->setChecked(val); }
void SettingsDialog::setShowNotification(bool val) { m_notifyCheck->setChecked(val); }
void SettingsDialog::setClipboardMonitor(bool val) { m_clipboardCheck->setChecked(val); }

QJsonObject SettingsDialog::loadSettings()
{
    return Utils::loadJsonConfig("config.json");
}

bool SettingsDialog::saveSettings(const QJsonObject &settings)
{
    return Utils::saveJsonConfig("config.json", settings);
}
