#ifndef STICKYNOTEWIDGET_H
#define STICKYNOTEWIDGET_H

#include <QWidget>
#include <QPoint>
#include <QPixmap>

class QLabel;
class QSlider;

class StickyNoteWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StickyNoteWidget(const QString &text, QWidget *parent = nullptr);
    explicit StickyNoteWidget(const QPixmap &pixmap, QWidget *parent = nullptr);
    ~StickyNoteWidget();

    void setOpacityLevel(qreal opacity);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    void setupUi();
    QLabel *m_label;
    QPixmap m_pixmap;
    QString m_text;
    bool m_isImage;
    QPoint m_dragStart;
    bool m_dragging;
    qreal m_opacity;
};

#endif // STICKYNOTEWIDGET_H
