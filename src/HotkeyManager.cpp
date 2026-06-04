#include "HotkeyManager.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include <QAbstractNativeEventFilter>
#include <QApplication>
#include <QDebug>

class WinEventFilter : public QAbstractNativeEventFilter
{
public:
    WinEventFilter(HotkeyManager *manager) : m_manager(manager) {}

#ifdef Q_OS_WIN
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override
    {
        Q_UNUSED(eventType);
        Q_UNUSED(result);
        MSG *msg = static_cast<MSG *>(message);
        if (msg->message == WM_HOTKEY) {
            if (m_manager) {
                emit m_manager->activated();
            }
            return true;
        }
        return false;
    }
#else
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override
    {
        Q_UNUSED(eventType);
        Q_UNUSED(message);
        Q_UNUSED(result);
        return false;
    }
#endif

private:
    HotkeyManager *m_manager;
};

static int qtModToWinMod(Qt::KeyboardModifiers qtMod)
{
    int winMod = 0;
    if (qtMod & Qt::ControlModifier) winMod |= MOD_CONTROL;
    if (qtMod & Qt::AltModifier)     winMod |= MOD_ALT;
    if (qtMod & Qt::ShiftModifier)   winMod |= MOD_SHIFT;
    if (qtMod & Qt::MetaModifier)    winMod |= MOD_WIN;
    return winMod;
}

static int qtKeyToVk(int qtKey)
{
    if (qtKey >= Qt::Key_F1 && qtKey <= Qt::Key_F24)
        return VK_F1 + (qtKey - Qt::Key_F1);
    switch (qtKey) {
    case Qt::Key_Return: return VK_RETURN;
    case Qt::Key_Escape: return VK_ESCAPE;
    case Qt::Key_Tab:    return VK_TAB;
    case Qt::Key_Space:  return VK_SPACE;
    case Qt::Key_Delete: return VK_DELETE;
    case Qt::Key_Insert: return VK_INSERT;
    case Qt::Key_Home:   return VK_HOME;
    case Qt::Key_End:    return VK_END;
    case Qt::Key_PageUp: return VK_PRIOR;
    case Qt::Key_PageDown: return VK_NEXT;
    case Qt::Key_Left:   return VK_LEFT;
    case Qt::Key_Right:  return VK_RIGHT;
    case Qt::Key_Up:     return VK_UP;
    case Qt::Key_Down:   return VK_DOWN;
    default:
        if (qtKey >= Qt::Key_A && qtKey <= Qt::Key_Z)
            return qtKey;
        if (qtKey >= Qt::Key_0 && qtKey <= Qt::Key_9)
            return qtKey;
        if (qtKey >= Qt::Key_F1 && qtKey <= Qt::Key_F24)
            return VK_F1 + (qtKey - Qt::Key_F1);
        return 0;
    }
}

HotkeyManager::HotkeyManager(QObject *parent)
    : QObject(parent), m_modifiers(0), m_key(0), m_registered(false)
{
    m_eventFilter = new WinEventFilter(this);
    qApp->installNativeEventFilter(m_eventFilter);
}

HotkeyManager::~HotkeyManager()
{
    unregisterNativeKey();
    if (m_eventFilter) {
        qApp->removeNativeEventFilter(m_eventFilter);
        delete m_eventFilter;
    }
}

bool HotkeyManager::registerHotkey(const QKeySequence &sequence)
{
    if (sequence.isEmpty()) return false;

    unregisterNativeKey();

    Qt::KeyboardModifiers qtMod = sequence[0].keyboardModifiers();
    int qtKey = sequence[0].key();

    if (qtKey == Qt::Key_Control || qtKey == Qt::Key_Shift ||
        qtKey == Qt::Key_Alt || qtKey == Qt::Key_Meta) {
        return false;
    }

    m_modifiers = qtModToWinMod(qtMod);
    m_key = qtKeyToVk(qtKey);

    if (m_key == 0) return false;

    return registerNativeKey(m_modifiers, m_key);
}

bool HotkeyManager::registerNativeKey(int modifiersVk, int keyVk)
{
#ifdef Q_OS_WIN
    bool ok = RegisterHotKey(nullptr, 1, modifiersVk, keyVk);
    m_registered = ok;
    return ok;
#else
    Q_UNUSED(modifiersVk);
    Q_UNUSED(keyVk);
    return false;
#endif
}

void HotkeyManager::unregisterNativeKey()
{
#ifdef Q_OS_WIN
    if (m_registered) {
        UnregisterHotKey(nullptr, 1);
        m_registered = false;
    }
#endif
    m_modifiers = 0;
    m_key = 0;
}

void HotkeyManager::unregisterHotkey()
{
    unregisterNativeKey();
}

bool HotkeyManager::isRegistered() const
{
    return m_registered;
}
