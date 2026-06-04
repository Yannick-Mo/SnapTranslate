#ifndef RESULTWIDGET_H
#define RESULTWIDGET_H

#include <QWidget>
#include <QPoint>
#include <QPixmap>

class QTextEdit;
class QPushButton;
class QStackedWidget;

class ResultWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ResultWidget(QWidget *parent = nullptr);
    void setResult(const QString &originalText, const QString &translatedText,
                   const QString &sourceLang = "auto", const QString &targetLang = "zh");
    void setOriginalText(const QString &text);
    void setTranslatedText(const QString &text);
    void showNearCursor();

    QString originalText() const { return m_originalText; }
    QString translatedText() const { return m_translatedText; }

signals:
    void closed();
    void pinRequested(const QString &text);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onCopyOriginal();
    void onCopyTranslation();
    void onPinClicked();
    void onCloseClicked();

private:
    void setupUi();
    void setActiveTab(int index);

    QPushButton *m_tabOriginal;
    QPushButton *m_tabTranslation;
    QStackedWidget *m_stack;
    QTextEdit *m_originalEdit;
    QTextEdit *m_translationEdit;
    QPushButton *m_copyOriginalBtn;
    QPushButton *m_copyTranslationBtn;
    QPushButton *m_pinBtn;
    QPushButton *m_closeBtn;
    QPoint m_dragStart;
    bool m_dragging;

    QString m_originalText;
    QString m_translatedText;
    int m_activeTab;
};

#endif // RESULTWIDGET_H
