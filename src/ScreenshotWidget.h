#ifndef SCREENSHOTWIDGET_H
#define SCREENSHOTWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QPoint>
#include <QRubberBand>

class ScreenshotWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ScreenshotWidget(QWidget *parent = nullptr);
    ~ScreenshotWidget();

signals:
    void screenshotTaken(const QPixmap &pixmap);
    void canceled();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void drawInfoText(QPainter &painter);
    void drawMagnifier(QPainter &painter, const QPoint &pos);

    QPixmap m_nativePixmap;
    QRubberBand *rubberBand;
    QPoint origin, currentPos;
    QRect m_selectionRect;
    bool isSelecting, isFinished;
    QRect m_virtualRect;
};

#endif // SCREENSHOTWIDGET_H
