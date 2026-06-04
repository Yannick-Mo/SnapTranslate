#include "TrayIcon.h"
#include <QAction>
#include <QIcon>

TrayIcon::TrayIcon(QObject *parent) : QSystemTrayIcon(parent)
{
    setIcon(QIcon(":/icon.png"));
    setToolTip("SnapTranslate - 截图翻译工具");

    m_menu = new QMenu();
    QAction *screenshotAction = new QAction("截图翻译 (Alt+Q)", this);
    QAction *historyAction = new QAction("历史记录", this);
    QAction *settingsAction = new QAction("设置", this);
    QAction *quitAction = new QAction("退出", this);

    connect(screenshotAction, &QAction::triggered, this, &TrayIcon::triggerScreenshot);
    connect(historyAction, &QAction::triggered, this, &TrayIcon::showHistory);
    connect(settingsAction, &QAction::triggered, this, &TrayIcon::showSettings);
    connect(quitAction, &QAction::triggered, this, &TrayIcon::quitApp);

    m_menu->addAction(screenshotAction);
    m_menu->addSeparator();
    m_menu->addAction(historyAction);
    m_menu->addAction(settingsAction);
    m_menu->addSeparator();
    m_menu->addAction(quitAction);

    setContextMenu(m_menu);
    connect(this, &QSystemTrayIcon::activated, this, &TrayIcon::onActivated);
}

void TrayIcon::showNotify(const QString &title, const QString &msg,
                          QSystemTrayIcon::MessageIcon icon, int timeout)
{
    showMessage(title, msg, icon, timeout);
}

void TrayIcon::onActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        emit triggerScreenshot();
    }
}
