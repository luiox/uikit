#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;

class MainWindow : public QWidget
{
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void addItemFromInput();
    void updateCounter();

private:
    QLabel* m_titleLabel = nullptr;
    QLineEdit* m_input = nullptr;
    QPushButton* m_addButton = nullptr;
    QPushButton* m_clearButton = nullptr;
    QLabel* m_counterLabel = nullptr;
    QListWidget* m_list = nullptr;
};
