#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H

#include <QDialog>
#include <QJsonArray>
#include <QDateTime>

class QTableWidget;
class QTextEdit;

struct HistoryEntry {
    QString originalText;
    QString translatedText;
    QString sourceLang;
    QString targetLang;
    QDateTime timestamp;
};

class HistoryDialog : public QDialog
{
    Q_OBJECT
public:
    explicit HistoryDialog(QWidget *parent = nullptr);

    void addEntry(const HistoryEntry &entry);
    void loadFromFile();
    void saveToFile();

private slots:
    void onItemClicked(int row, int col);
    void onClearAll();

private:
    void setupUi();
    static QString historyFilePath();

    QTableWidget *m_table;
    QList<HistoryEntry> m_entries;
};

#endif // HISTORYDIALOG_H
