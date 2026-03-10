#include "SettingsPage.h"
#include <QVBoxLayout>
#include <QLabel>

namespace ant {

SettingsPage::SettingsPage(QWidget* parent)
    : QWidget(parent)
{
	setObjectName("SettingsPage");
    
	QLabel* label = new QLabel(QStringLiteral("设置"), this);
    QFont font = label->font();
    font.setPointSize(52);
    font.setBold(true);
    label->setFont(font);
    label->setAlignment(Qt::AlignCenter);

    QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(6, 0, 6, 0);
    layout->addWidget(label);
    setStyleSheet("background-color: transparent;");
}


SettingsPage::~SettingsPage()
{}

} // namespace ant
