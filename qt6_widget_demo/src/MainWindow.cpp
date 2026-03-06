#include "MainWindow.h"

#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle(QStringLiteral("Qt6 Widget Demo (xmake)"));
    resize(760, 520);

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 24, 24, 24);
    rootLayout->setSpacing(16);

    m_titleLabel = new QLabel(QStringLiteral("Qt6 Widgets + xmake Demo"), this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);

    auto* descLabel = new QLabel(
        QStringLiteral("This demo verifies that xmake can detect Qt 6.8.3 and build a minimal Widgets app."),
        this);
    descLabel->setWordWrap(true);

    auto* inputRow = new QHBoxLayout();
    inputRow->setSpacing(10);

    m_input = new QLineEdit(this);
    m_input->setPlaceholderText(QStringLiteral("Type a todo item, for example: Build QtIconProvider first"));

    m_addButton = new QPushButton(QStringLiteral("Add"), this);
    m_clearButton = new QPushButton(QStringLiteral("Clear"), this);

    inputRow->addWidget(m_input, 1);
    inputRow->addWidget(m_addButton);
    inputRow->addWidget(m_clearButton);

    m_counterLabel = new QLabel(this);
    m_list = new QListWidget(this);
    m_list->addItem(QStringLiteral("Verify the Qt6 xmake build pipeline"));
    m_list->addItem(QStringLiteral("Next: add a unified icon access layer to libant-qt"));
    m_list->addItem(QStringLiteral("Then: migrate core widgets to libicon-core"));

    rootLayout->addWidget(m_titleLabel);
    rootLayout->addWidget(descLabel);
    rootLayout->addLayout(inputRow);
    rootLayout->addWidget(m_counterLabel);
    rootLayout->addWidget(m_list, 1);

    setStyleSheet(R"(
        QWidget {
            background: #f7f9fc;
            color: #1f2937;
            font-size: 14px;
        }
        QLineEdit {
            background: white;
            border: 1px solid #d0d7e2;
            border-radius: 8px;
            padding: 8px 10px;
        }
        QLineEdit:focus {
            border: 1px solid #1677ff;
        }
        QPushButton {
            background: #1677ff;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 14px;
            min-width: 84px;
        }
        QPushButton:hover {
            background: #4096ff;
        }
        QListWidget {
            background: white;
            border: 1px solid #d0d7e2;
            border-radius: 10px;
            padding: 8px;
        }
    )");

    connect(m_addButton, &QPushButton::clicked, this, [this]() {
        addItemFromInput();
    });

    connect(m_clearButton, &QPushButton::clicked, this, [this]() {
        m_list->clear();
        updateCounter();
    });

    connect(m_input, &QLineEdit::returnPressed, this, [this]() {
        addItemFromInput();
    });

    updateCounter();
}

void MainWindow::addItemFromInput()
{
    const QString text = m_input->text().trimmed();
    if (text.isEmpty())
    {
        return;
    }

    m_list->addItem(text);
    m_input->clear();
    updateCounter();
}

void MainWindow::updateCounter()
{
    m_counterLabel->setText(QStringLiteral("Current item count: %1").arg(m_list->count()));
}
