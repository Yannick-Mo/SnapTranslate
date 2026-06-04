#include "HistoryDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>

HistoryDialog::HistoryDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("翻译历史记录");
    setMinimumSize(600, 400);
    setStyleSheet(
        "QDialog { background-color: #1a1a2e; color: #e0e0e0; }"
        "QTableWidget { background: #16213e; color: #c0c0d0; border: 1px solid #2a2a4a; border-radius: 8px; gridline-color: #2a2a4a; }"
        "QTableWidget::item { padding: 6px; }"
        "QTableWidget::item:selected { background: #0f3460; color: white; }"
        "QHeaderView::section { background: #0f3460; color: white; padding: 6px; border: none; font-weight: bold; }"
        "QPushButton { background: #0f3460; color: white; border: none; border-radius: 6px; padding: 8px 16px; }"
        "QPushButton:hover { background: #1a4a8a; }"
        "QPushButton#btnClear { background: #4a1a1a; }"
        "QPushButton#btnClear:hover { background: #6a2a2a; }"
    );
    setupUi();
    loadFromFile();
}

void HistoryDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    m_table = new QTableWidget(0, 3);
    m_table->setHorizontalHeaderLabels({"时间", "原文", "译文"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->hide();
    m_table->setShowGrid(true);
    m_table->setAlternatingRowColors(true);
    m_table->setStyleSheet(
        "QTableWidget { alternate-background-color: #1e1e3a; }"
    );

    connect(m_table, &QTableWidget::cellClicked, this, &HistoryDialog::onItemClicked);
    mainLayout->addWidget(m_table);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    QPushButton *clearBtn = new QPushButton("清空历史");
    clearBtn->setObjectName("btnClear");
    QPushButton *closeBtn = new QPushButton("关闭");
    btnLayout->addWidget(clearBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    mainLayout->addLayout(btnLayout);

    connect(clearBtn, &QPushButton::clicked, this, &HistoryDialog::onClearAll);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void HistoryDialog::addEntry(const HistoryEntry &entry)
{
    m_entries.prepend(entry);
    m_table->insertRow(0);
    m_table->setItem(0, 0, new QTableWidgetItem(entry.timestamp.toString("yyyy-MM-dd HH:mm")));
    m_table->setItem(0, 1, new QTableWidgetItem(entry.originalText.left(50)));
    m_table->setItem(0, 2, new QTableWidgetItem(entry.translatedText.left(50)));
    saveToFile();
}

void HistoryDialog::onItemClicked(int row, int col)
{
    Q_UNUSED(col);
    if (row >= 0 && row < m_entries.size()) {
        const HistoryEntry &e = m_entries[row];
        QMessageBox::information(this, "翻译详情",
            QString("时间: %1\n\n原文:\n%2\n\n译文:\n%3")
                .arg(e.timestamp.toString("yyyy-MM-dd HH:mm:ss"))
                .arg(e.originalText)
                .arg(e.translatedText));
    }
}

void HistoryDialog::onClearAll()
{
    if (QMessageBox::question(this, "确认", "确定要清空所有历史记录吗？") == QMessageBox::Yes) {
        m_entries.clear();
        m_table->setRowCount(0);
        saveToFile();
    }
}

QString HistoryDialog::historyFilePath()
{
    return "history.json";
}

void HistoryDialog::loadFromFile()
{
    QFile file(historyFilePath());
    if (!file.open(QIODevice::ReadOnly))
        return;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonArray arr = doc.array();
    for (const QJsonValue &val : arr) {
        QJsonObject obj = val.toObject();
        HistoryEntry entry;
        entry.originalText = obj["original"].toString();
        entry.translatedText = obj["translated"].toString();
        entry.sourceLang = obj["source_lang"].toString();
        entry.targetLang = obj["target_lang"].toString();
        entry.timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
        m_entries.append(entry);
    }
    for (int i = 0; i < m_entries.size(); i++) {
        const HistoryEntry &e = m_entries[i];
        m_table->insertRow(i);
        m_table->setItem(i, 0, new QTableWidgetItem(e.timestamp.toString("yyyy-MM-dd HH:mm")));
        m_table->setItem(i, 1, new QTableWidgetItem(e.originalText.left(50)));
        m_table->setItem(i, 2, new QTableWidgetItem(e.translatedText.left(50)));
    }
}

void HistoryDialog::saveToFile()
{
    QJsonArray arr;
    for (const HistoryEntry &e : m_entries) {
        QJsonObject obj;
        obj["original"] = e.originalText;
        obj["translated"] = e.translatedText;
        obj["source_lang"] = e.sourceLang;
        obj["target_lang"] = e.targetLang;
        obj["timestamp"] = e.timestamp.toString(Qt::ISODate);
        arr.append(obj);
    }
    QFile file(historyFilePath());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(arr).toJson());
    }
}
