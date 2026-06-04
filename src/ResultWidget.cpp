#include "ResultWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
#include <QMouseEvent>
#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QTimer>
#include <QScreen>
#include <QFont>

ResultWidget::ResultWidget(QWidget *parent)
    : QWidget(parent), m_dragging(false), m_activeTab(0)
{
    setupUi();
}

void ResultWidget::setupUi()
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedWidth(420);
    setMinimumHeight(160);
    setMaximumHeight(400);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Main container with rounded corners and shadow
    QWidget *container = new QWidget(this);
    container->setObjectName("resultContainer");
    container->setStyleSheet(
        "#resultContainer {"
        "  background-color: rgba(22, 22, 46, 230);"
        "  border: 1px solid rgba(255, 255, 255, 0.08);"
        "  border-radius: 12px;"
        "}"
    );

    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    // === Tab Header ===
    QWidget *tabBar = new QWidget;
    tabBar->setStyleSheet("background: transparent;");
    QHBoxLayout *tabLayout = new QHBoxLayout(tabBar);
    tabLayout->setContentsMargins(12, 8, 12, 0);
    tabLayout->setSpacing(0);

    QString tabBaseStyle =
        "QPushButton {"
        "  padding: 4px 16px;"
        "  border-radius: 6px 6px 0 0;"
        "  font-size: 12px;"
        "  font-weight: bold;"
        "  border: none;"
        "}";

    m_tabOriginal = new QPushButton("原文");
    m_tabOriginal->setCursor(Qt::PointingHandCursor);
    m_tabOriginal->setFixedHeight(32);
    m_tabOriginal->setStyleSheet(
        tabBaseStyle +
        "QPushButton {"
        "  color: #8888aa;"
        "  background: transparent;"
        "}"
        "QPushButton:hover { color: #c0c0d0; }"
    );

    m_tabTranslation = new QPushButton("译文");
    m_tabTranslation->setCursor(Qt::PointingHandCursor);
    m_tabTranslation->setFixedHeight(32);
    m_tabTranslation->setStyleSheet(
        tabBaseStyle +
        "QPushButton {"
        "  color: #00d4ff;"
        "  background: rgba(0, 212, 255, 0.08);"
        "}"
        "QPushButton:hover { color: #33ddff; }"
    );

    tabLayout->addWidget(m_tabOriginal);
    tabLayout->addWidget(m_tabTranslation);
    tabLayout->addStretch();

    containerLayout->addWidget(tabBar);

    // === Divider ===
    QWidget *divider = new QWidget;
    divider->setFixedHeight(1);
    divider->setStyleSheet("background: rgba(255, 255, 255, 0.06);");
    containerLayout->addWidget(divider);

    // === Content Area (Stacked) ===
    m_stack = new QStackedWidget;
    m_stack->setStyleSheet("background: transparent;");

    // Original text page
    QWidget *origPage = new QWidget;
    origPage->setStyleSheet("background: transparent;");
    QVBoxLayout *origLayout = new QVBoxLayout(origPage);
    origLayout->setContentsMargins(12, 8, 12, 8);
    m_originalEdit = new QTextEdit;
    m_originalEdit->setReadOnly(true);
    m_originalEdit->setStyleSheet(
        "QTextEdit {"
        "  background: rgba(0, 0, 0, 0.2);"
        "  color: #c0c0d0;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 8px;"
        "  font-size: 13px;"
        "  selection-background-color: #0f3460;"
        "}"
    );
    m_originalEdit->setMinimumHeight(60);
    origLayout->addWidget(m_originalEdit);

    // Translated text page
    QWidget *transPage = new QWidget;
    transPage->setStyleSheet("background: transparent;");
    QVBoxLayout *transLayout = new QVBoxLayout(transPage);
    transLayout->setContentsMargins(12, 8, 12, 8);
    m_translationEdit = new QTextEdit;
    m_translationEdit->setReadOnly(true);
    m_translationEdit->setStyleSheet(
        "QTextEdit {"
        "  background: rgba(0, 212, 255, 0.05);"
        "  color: #ffffff;"
        "  border: 1px solid rgba(0, 212, 255, 0.15);"
        "  border-radius: 8px;"
        "  padding: 8px;"
        "  font-size: 13px;"
        "  font-weight: bold;"
        "  selection-background-color: #0f3460;"
        "}"
    );
    m_translationEdit->setMinimumHeight(60);
    transLayout->addWidget(m_translationEdit);

    m_stack->addWidget(origPage);
    m_stack->addWidget(transPage);
    containerLayout->addWidget(m_stack);

    // === Action Bar ===
    QWidget *actionBar = new QWidget;
    actionBar->setStyleSheet("background: transparent;");
    QHBoxLayout *actionLayout = new QHBoxLayout(actionBar);
    actionLayout->setContentsMargins(12, 4, 12, 10);
    actionLayout->setSpacing(6);

    m_copyOriginalBtn = new QPushButton("复制原文");
    m_copyTranslationBtn = new QPushButton("复制译文");
    m_pinBtn = new QPushButton("📌 贴图");
    m_closeBtn = new QPushButton("✕");

    QString btnStyle =
        "QPushButton {"
        "  background: rgba(255, 255, 255, 0.06);"
        "  color: #c0c0d0;"
        "  border: 1px solid rgba(255, 255, 255, 0.08);"
        "  border-radius: 6px;"
        "  padding: 5px 12px;"
        "  font-size: 11px;"
        "}"
        "QPushButton:hover {"
        "  background: rgba(255, 255, 255, 0.12);"
        "  color: white;"
        "}";

    QString closeBtnStyle =
        "QPushButton {"
        "  background: transparent;"
        "  color: #8888aa;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 5px 8px;"
        "  font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "  background: rgba(0, 212, 255, 0.2);"
        "  color: #00d4ff;"
        "}";

    m_copyOriginalBtn->setStyleSheet(btnStyle);
    m_copyTranslationBtn->setStyleSheet(btnStyle);
    m_pinBtn->setStyleSheet(btnStyle);
    m_closeBtn->setStyleSheet(closeBtnStyle);

    actionLayout->addWidget(m_copyOriginalBtn);
    actionLayout->addWidget(m_copyTranslationBtn);
    actionLayout->addWidget(m_pinBtn);
    actionLayout->addStretch();
    actionLayout->addWidget(m_closeBtn);

    containerLayout->addWidget(actionBar);

    mainLayout->addWidget(container);

    // === Connections ===
    connect(m_tabOriginal, &QPushButton::clicked, [this]() { setActiveTab(0); });
    connect(m_tabTranslation, &QPushButton::clicked, [this]() { setActiveTab(1); });
    connect(m_copyOriginalBtn, &QPushButton::clicked, this, &ResultWidget::onCopyOriginal);
    connect(m_copyTranslationBtn, &QPushButton::clicked, this, &ResultWidget::onCopyTranslation);
    connect(m_pinBtn, &QPushButton::clicked, this, &ResultWidget::onPinClicked);
    connect(m_closeBtn, &QPushButton::clicked, this, &ResultWidget::onCloseClicked);

    setFocusPolicy(Qt::StrongFocus);
    setActiveTab(1);
}

void ResultWidget::setResult(const QString &originalText, const QString &translatedText,
                             const QString &sourceLang, const QString &targetLang)
{
    Q_UNUSED(sourceLang);
    Q_UNUSED(targetLang);
    m_originalText = originalText;
    m_translatedText = translatedText;
    m_originalEdit->setPlainText(originalText);
    m_translationEdit->setPlainText(translatedText);
    setActiveTab(1);
    adjustSize();
}

void ResultWidget::setActiveTab(int index)
{
    m_activeTab = index;
    m_stack->setCurrentIndex(index);

    QString tabBase =
        "QPushButton {"
        "  padding: 4px 16px;"
        "  border-radius: 6px 6px 0 0;"
        "  font-size: 12px;"
        "  font-weight: bold;"
        "  border: none;"
        "}";
    QString inactiveTabStyle = tabBase +
        "QPushButton {"
        "  color: #8888aa;"
        "  background: transparent;"
        "}"
        "QPushButton:hover { color: #c0c0d0; }";
    QString activeTabStyle = tabBase +
        "QPushButton {"
        "  color: #00d4ff;"
        "  background: rgba(0, 212, 255, 0.08);"
        "}"
        "QPushButton:hover { color: #33ddff; }";

    m_tabOriginal->setStyleSheet(index == 0 ? activeTabStyle : inactiveTabStyle);
    m_tabTranslation->setStyleSheet(index == 1 ? activeTabStyle : inactiveTabStyle);
}

void ResultWidget::showNearCursor()
{
    adjustSize();
    QPoint cursorPos = QCursor::pos();
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenRect = screen->availableGeometry();
    int x = cursorPos.x() + 15;
    int y = cursorPos.y() + 15;
    if (x + width() > screenRect.right())
        x = cursorPos.x() - width() - 15;
    if (y + height() > screenRect.bottom())
        y = cursorPos.y() - height() - 15;
    move(x, y);
    show();
    raise();
    activateWindow();
}

void ResultWidget::setOriginalText(const QString &text)
{
    m_originalText = text;
    m_originalEdit->setPlainText(text);
}

void ResultWidget::setTranslatedText(const QString &text)
{
    m_translatedText = text;
    m_translationEdit->setPlainText(text);
    setActiveTab(1);
    adjustSize();
}

void ResultWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragStart = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void ResultWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragStart);
        event->accept();
    }
}

void ResultWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        event->accept();
    }
}

void ResultWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
    } else {
        QWidget::keyPressEvent(event);
    }
}

void ResultWidget::onCopyOriginal()
{
    QGuiApplication::clipboard()->setText(m_originalText);
    m_copyOriginalBtn->setText("✓ 已复制");
    QTimer::singleShot(1500, [this]() { m_copyOriginalBtn->setText("复制原文"); });
}

void ResultWidget::onCopyTranslation()
{
    QGuiApplication::clipboard()->setText(m_translatedText);
    m_copyTranslationBtn->setText("✓ 已复制");
    QTimer::singleShot(1500, [this]() { m_copyTranslationBtn->setText("复制译文"); });
}

void ResultWidget::onPinClicked()
{
    emit pinRequested(m_activeTab == 0 ? m_originalText : m_translatedText);
}

void ResultWidget::onCloseClicked()
{
    close();
    emit closed();
}
