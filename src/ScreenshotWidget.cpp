#include "ScreenshotWidget.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QScreen>
#include <QGuiApplication>
#include <QRect>
#include <QPainter>
#include <QPainterPath>
#include <QFont>

ScreenshotWidget::ScreenshotWidget(QWidget *parent)
    : QWidget(parent), rubberBand(nullptr), isSelecting(false), isFinished(false)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setCursor(Qt::CrossCursor);

    const auto screens = QGuiApplication::screens();
    for (QScreen *screen : screens)
        m_virtualRect = m_virtualRect.united(screen->geometry());
    setGeometry(m_virtualRect);

    QScreen *primary = QGuiApplication::primaryScreen();

    // grabWindow returns pixmap at logical size with devicePixelRatio set.
    // Qt will draw it at full physical resolution — no manual scaling needed.
    m_nativePixmap = primary->grabWindow(0,
        m_virtualRect.x(), m_virtualRect.y(),
        m_virtualRect.width(), m_virtualRect.height());

    show();
}

ScreenshotWidget::~ScreenshotWidget() {}

void ScreenshotWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        origin = event->position().toPoint();
        currentPos = origin;
        if (!rubberBand) {
            rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
            rubberBand->hide(); // we draw our own visuals
        }
        rubberBand->setGeometry(QRect(origin, QSize()));
        m_selectionRect = QRect();
        isSelecting = true;
        isFinished = false;
    }
}

void ScreenshotWidget::mouseMoveEvent(QMouseEvent *event)
{
    currentPos = event->position().toPoint();
    if (isSelecting && rubberBand) {
        m_selectionRect = QRect(origin, event->position().toPoint()).normalized();
        rubberBand->setGeometry(m_selectionRect);
    }
    update();
}

void ScreenshotWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isSelecting) {
        isSelecting = false;
        QRect sel = m_selectionRect;

        if (sel.width() < 5 || sel.height() < 5) {
            emit canceled();
            close();
            return;
        }

        isFinished = true;

        // grabWindow takes logical coords in Qt 6.
        QScreen *primary = QGuiApplication::primaryScreen();
        QPixmap pixmap = primary->grabWindow(0,
            m_virtualRect.x() + sel.x(),
            m_virtualRect.y() + sel.y(),
            sel.width(), sel.height());
        // Strip DPR so the pixmap contains raw physical pixels for OCR
        pixmap.setDevicePixelRatio(1.0);

        emit screenshotTaken(pixmap);
        close();
    }
}

void ScreenshotWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        emit canceled();
        close();
    } else {
        QWidget::keyPressEvent(event);
    }
}

void ScreenshotWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    // Draw full screen at native resolution — Qt's DPR handling keeps it crisp
    painter.drawPixmap(0, 0, m_nativePixmap);

    if (isSelecting && !m_selectionRect.isNull()) {
        QRect sel = m_selectionRect;

        // Dark overlay with a hole at the selection (odd-even fill)
        QPainterPath overlay;
        overlay.setFillRule(Qt::OddEvenFill);
        overlay.addRect(QRectF(rect()));
        overlay.addRect(QRectF(sel));
        painter.fillPath(overlay, QColor(0, 0, 0, 100));

        // Fluorescent blue selection border
        QPen borderPen(QColor(0, 212, 255), 2);
        painter.setPen(borderPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(sel);
    }

    if (isSelecting && !m_selectionRect.isNull()) {
        drawInfoText(painter);
        drawMagnifier(painter, currentPos);
    }
}

void ScreenshotWidget::drawMagnifier(QPainter &painter, const QPoint &pos)
{
    int size = 80;
    int scale = 2;
    int half = size / 2;
    int srcSz = size / scale;

    QRect magRect(pos.x() - half, pos.y() - half - size - 10, size, size);
    QRect sb = rect();
    if (magRect.left() < sb.left()) magRect.moveLeft(sb.left());
    if (magRect.top()  < sb.top())  magRect.moveTop(sb.top());
    if (magRect.right()  > sb.right())  magRect.moveRight(sb.right());
    if (magRect.bottom() > sb.bottom()) magRect.moveBottom(sb.bottom());

    // copy() on a DPR pixmap uses logical (device-independent) coords
    QRect srcRect(
        qMax(0, pos.x() - srcSz / 2),
        qMax(0, pos.y() - srcSz / 2),
        srcSz, srcSz
    );
    QPixmap magnified = m_nativePixmap.copy(srcRect)
        .scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    painter.setPen(QPen(QColor(255, 255, 255), 2));
    painter.setBrush(QColor(30, 30, 46, 200));
    painter.drawRoundedRect(magRect.adjusted(-4, -4, 4, 4), 8, 8);
    painter.drawPixmap(magRect, magnified);

    painter.setPen(QPen(QColor(0, 212, 255), 1));
    int cx = magRect.center().x();
    int cy = magRect.center().y();
    painter.drawLine(cx - 10, cy, cx + 10, cy);
    painter.drawLine(cx, cy - 10, cx, cy + 10);
}

void ScreenshotWidget::drawInfoText(QPainter &painter)
{
    QRect sel = m_selectionRect;
    QString info = QString("%1 x %2").arg(sel.width()).arg(sel.height());

    painter.setFont(QFont("Consolas", 11));
    QColor borderColor(0, 212, 255);
    painter.setPen(borderColor);
    painter.setBrush(Qt::NoBrush);
    QRect textRect(sel.left() + 4, sel.top() - 20, sel.width(), 18);
    if (textRect.top() < 0) textRect.moveTop(sel.bottom() + 4);
    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, info);
}
