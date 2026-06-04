#ifndef HOTKEYMANAGER_H
#define HOTKEYMANAGER_H

#include <QObject>
#include <QKeySequence>

class QAbstractNativeEventFilter;

class HotkeyManager : public QObject
{
    Q_OBJECT
public:
    explicit HotkeyManager(QObject *parent = nullptr);
    ~HotkeyManager();

    bool registerHotkey(const QKeySequence &sequence);
    void unregisterHotkey();
    bool isRegistered() const;

signals:
    void activated();

private:
    bool registerNativeKey(int modifiersVk, int keyVk);
    void unregisterNativeKey();

    int m_modifiers;
    int m_key;
    bool m_registered;
    class WinEventFilter *m_eventFilter;
};

#endif // HOTKEYMANAGER_H
