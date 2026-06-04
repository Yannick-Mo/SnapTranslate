#ifndef CLIPBOARDLISTENER_H
#define CLIPBOARDLISTENER_H

#include <QObject>
#include <QTimer>
#include <QPixmap>

class QClipboard;

class ClipboardListener : public QObject
{
    Q_OBJECT
public:
    explicit ClipboardListener(QObject *parent = nullptr);
    void start(int debounceMs = 300);
    void stop();

signals:
    void textChanged(const QString &text);
    void imageChanged(const QPixmap &pixmap);

private slots:
    void onClipboardDataChanged();
    void onDebounceTimeout();

private:
    QClipboard *m_clipboard;
    QTimer *m_debounceTimer;
    int m_debounceMs;
    QString m_lastText;
};

#endif // CLIPBOARDLISTENER_H
