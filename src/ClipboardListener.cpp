#include "ClipboardListener.h"
#include <QClipboard>
#include <QGuiApplication>
#include <QMimeData>
#include <QPixmap>

ClipboardListener::ClipboardListener(QObject *parent)
    : QObject(parent)
    , m_clipboard(QGuiApplication::clipboard())
    , m_debounceTimer(nullptr)
{
    m_debounceTimer = new QTimer(this);
    m_debounceTimer->setSingleShot(true);
    connect(m_debounceTimer, &QTimer::timeout, this, &ClipboardListener::onDebounceTimeout);
}

void ClipboardListener::start(int debounceMs)
{
    m_debounceMs = debounceMs;
    connect(m_clipboard, &QClipboard::dataChanged, this, &ClipboardListener::onClipboardDataChanged);
}

void ClipboardListener::stop()
{
    disconnect(m_clipboard, nullptr, this, nullptr);
    m_debounceTimer->stop();
}

void ClipboardListener::onClipboardDataChanged()
{
    m_debounceTimer->start(m_debounceMs);
}

void ClipboardListener::onDebounceTimeout()
{
    const QMimeData *mime = m_clipboard->mimeData();
    if (!mime) return;

    if (mime->hasText()) {
        QString text = m_clipboard->text();
        if (!text.isEmpty() && text != m_lastText) {
            m_lastText = text;
            emit textChanged(text);
        }
    } else if (mime->hasImage()) {
        QImage img = mime->imageData().value<QImage>();
        if (!img.isNull()) {
            QPixmap pix = QPixmap::fromImage(img);
            emit imageChanged(pix);
        }
    }
}
