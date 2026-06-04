#include "StickyNoteWidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QMenu>
#include <QAction>
#include <QPainter>
#include <QSlider>
#include <QDialog>
#include <QPushButton>

StickyNoteWidget::StickyNoteWidget(const QString &text, QWidget *parent)
    : QWidget(parent), m_text(text), m_isImage(false), m_dragging(false), m_opacity(0.9)
{
    setupUi();
    m_label->setText(text);
    m_label->setWordWrap(true);
    adjustSize();
}

StickyNoteWidget::StickyNoteWidget(const QPixmap &pixmap, QWidget *parent)
    : QWidget(parent), m_pixmap(pixmap), m_isImage(true), m_dragging(false), m_opacity(0.9)
{
    setupUi();
    m_label->setPixmap(pixmap.scaled(400, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    adjustSize();
}

StickyNoteWidget::~StickyNoteWidget() {}

void StickyNoteWidget::setupUi()
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);

    m_label = new QLabel;
    m_label->setStyleSheet(
        "QLabel {"
        "  color: #e0e0e0;"
        "  font-size: 13px;"
        "  background: transparent;"
        "}"
    );
    layout->addWidget(m_label);

    setMinimumSize(100, 60);
    setMouseTracking(true);
}

void StickyNoteWidget::setOpacityLevel(qreal opacity)
{
    m_opacity = opacity;
    setWindowOpacity(opacity);
}

void StickyNoteWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragStart = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void StickyNoteWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragStart);
        event->accept();
    }
}

void StickyNoteWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        event->accept();
    }
}

void StickyNoteWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(30, 30, 50, int(200 * m_opacity)));
    painter.setPen(QPen(QColor(0, 212, 255, int(100 * m_opacity)), 2));
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 12, 12);
}

void StickyNoteWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    QAction *closeAction = menu.addAction("关闭");
    QAction *opacityAction = menu.addAction("调节透明度");

    QAction *selected = menu.exec(event->globalPos());
    if (selected == closeAction) {
        close();
        deleteLater();
    } else if (selected == opacityAction) {
        QDialog dlg(this);
        dlg.setWindowTitle("透明度");
        QVBoxLayout *l = new QVBoxLayout(&dlg);
        QSlider *slider = new QSlider(Qt::Horizontal);
        slider->setRange(20, 100);
        slider->setValue(int(m_opacity * 100));
        l->addWidget(slider);
        QPushButton *ok = new QPushButton("确定");
        l->addWidget(ok);
        connect(ok, &QPushButton::clicked, &dlg, &QDialog::accept);
        if (dlg.exec() == QDialog::Accepted) {
            setOpacityLevel(slider->value() / 100.0);
        }
    }
}
