#include "AboutPage.h"
#include <QVBoxLayout>
#include <QLabel>

namespace ant {

AboutPage::AboutPage(QWidget* parent)
    : QWidget(parent)
{
	setObjectName("AboutPage");

	QLabel* label = new QLabel(QStringLiteral("关于"), this);
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
AboutPage::~AboutPage()
{}

} // namespace ant
