#include "HomePage.h"
#include <QLabel>
#include <QLayout>
#include "DesignSystem.h"

HomePage::HomePage(QWidget* parent)
	: QWidget(parent)
{
	setObjectName("HomePage");
	setStyleSheet("#HomePage { background-color: transparent; }");

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(40, 40, 40, 40);
	layout->setSpacing(16);

	QLabel* title = new QLabel(QStringLiteral("libant-qt 组件演示"), this);
	QFont titleFont;
	titleFont.setPointSizeF(28);
	titleFont.setBold(true);
	title->setFont(titleFont);
	title->setStyleSheet(QStringLiteral("color: %1;").arg(DesignSystem::instance()->currentTheme().popupTextColor.name()));

	QLabel* desc = new QLabel(QStringLiteral("这个演示工程已经从 libant-qt 中独立拆出，并通过 xmake 单独构建，用来集中验证组件样式、字体与交互效果。"), this);
	desc->setWordWrap(true);
	desc->setStyleSheet(QStringLiteral("color: %1;").arg(DesignSystem::instance()->currentTheme().listItemDescColor.name()));

	layout->addStretch();
	layout->addWidget(title);
	layout->addWidget(desc);
	layout->addStretch();
}

HomePage::~HomePage()
{
}

void HomePage::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	resize(DesignSystem::instance()->contentSize());
}
