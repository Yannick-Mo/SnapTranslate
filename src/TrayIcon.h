#ifndef TRAYICON_H
#define TRAYICON_H

#include <QSystemTrayIcon>
#include <QMenu>

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT
public:
    explicit TrayIcon(QObject *parent = nullptr);
    void showNotify(const QString &title, const QString &msg,
                    QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information,
                    int timeout = 2000);

signals:
    void triggerScreenshot();
    void showSettings();
    void showHistory();
    void quitApp();

private slots:
    void onActivated(QSystemTrayIcon::ActivationReason reason);

private:
    QMenu *m_menu;
};

#endif // TRAYICON_H
