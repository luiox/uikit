#include "AntBaseDoubleSpinBox.h"
#include <QStyle>
#include <QStyleOptionSpinBox>
#include <QLineEdit>
#include "StyleSheet.h"

namespace ant {

AntBaseDoubleSpinBox::AntBaseDoubleSpinBox(QWidget* parent)
	: QDoubleSpinBox(parent), m_buttonX(0)
{
	setObjectName("AntBaseDoubleSpinBox");
	setButtonSymbols(QAbstractSpinBox::NoButtons);
	setContextMenuPolicy(Qt::NoContextMenu);
	const bool isDark = DesignSystem::instance()->themeMode() == DesignSystem::Dark;

	setStyleSheet(StyleSheet::AntBaseDoubleSpinBoxQss(theme.borderColor, theme.primaryColor));

	m_plusBtn = new QToolButton(this);
	m_plusBtn->setIcon(IconProvider::icon(IconRole::ArrowUp, isDark, QSize(14, 14), theme.textColor));
	m_plusBtn->setCursor(Qt::PointingHandCursor);
	m_plusBtn->setStyleSheet("QToolButton { border: none; background: transparent; }");
	connect(m_plusBtn, &QToolButton::clicked, this, [this]() { stepBy(1); });

	m_minusBtn = new QToolButton(this);
	m_minusBtn->setIcon(IconProvider::icon(IconRole::ArrowDown, isDark, QSize(14, 14), theme.textColor));
	m_minusBtn->setCursor(Qt::PointingHandCursor);
	m_minusBtn->setStyleSheet("QToolButton { border: none; background: transparent; }");
	connect(m_minusBtn, &QToolButton::clicked, this, [this]() { stepBy(-1); });

	m_animation = new QPropertyAnimation(this, "buttonX", this);
	m_animation->setDuration(300);
	m_animation->setEasingCurve(QEasingCurve::InOutCubic);

	QFont font;
	font.setPointSizeF(10.5);
	setFont(font);

	connect(DesignSystem::instance(), &DesignSystem::themeChanged, this, [this]()
		{
			auto theme = DesignSystem::instance()->currentTheme();
			setStyleSheet(StyleSheet::AntBaseDoubleSpinBoxQss(theme.borderColor, theme.primaryColor));
			const bool isDark = DesignSystem::instance()->themeMode() == DesignSystem::Dark;
			m_plusBtn->setIcon(IconProvider::icon(IconRole::ArrowUp, isDark, QSize(14, 14), theme.textColor));
			m_minusBtn->setIcon(IconProvider::icon(IconRole::ArrowDown, isDark, QSize(14, 14), theme.textColor));
		});
}

AntBaseDoubleSpinBox::~AntBaseDoubleSpinBox()
{
	// 资源由 Qt parent-child 系统自动清理
}

void AntBaseDoubleSpinBox::enterEvent(QEnterEvent* event)
{
	if (m_animation->state() == QAbstractAnimation::Running)
		m_animation->stop();
	m_animation->setDirection(QAbstractAnimation::Forward);
	m_animation->start();

	QDoubleSpinBox::enterEvent(event);
}

void AntBaseDoubleSpinBox::leaveEvent(QEvent* event)
{
	if (m_animation->state() == QAbstractAnimation::Running)
		m_animation->stop();
	m_animation->setDirection(QAbstractAnimation::Backward);
	m_animation->start();

	QDoubleSpinBox::leaveEvent(event);
}

void AntBaseDoubleSpinBox::stepBy(int steps)
{
	QDoubleSpinBox::stepBy(steps);
	lineEdit()->deselect();
	lineEdit()->setCursorPosition(lineEdit()->text().length());
}

void AntBaseDoubleSpinBox::resizeEvent(QResizeEvent* event)
{
	QDoubleSpinBox::resizeEvent(event);

	int btnHeight = height() / 2;
	int btnWidth = btnHeight;

	m_plusBtn->setFixedSize(btnWidth, btnHeight);
	m_plusBtn->setIconSize(QSize(btnWidth * 1.3, btnHeight * 1.3));

	m_minusBtn->setFixedSize(btnWidth, btnHeight);
	m_minusBtn->setIconSize(QSize(btnWidth * 1.3, btnHeight * 1.3));

	int startX = width() + btnWidth;
	int endX = width() - btnWidth - 2;

	if (m_animation->state() != QAbstractAnimation::Running)
	{
		m_buttonX = startX;
		m_animation->setStartValue(startX);
		m_animation->setEndValue(endX);
		updateButtonsPosition();
	}
}

void AntBaseDoubleSpinBox::updateButtonsPosition()
{
	int btnWidth = m_plusBtn->width();
	int btnHeight = m_plusBtn->height();

	int btnYTop = 0;
	int btnYBottom = height() / 2;

	m_plusBtn->move(m_buttonX, btnYTop);
	m_minusBtn->move(m_buttonX, btnYBottom);
}

// 添加属性动画相关
int AntBaseDoubleSpinBox::buttonX() const { return m_buttonX; }

void AntBaseDoubleSpinBox::setButtonX(int x)
{
	m_buttonX = x;
	updateButtonsPosition();
}

void AntBaseDoubleSpinBox::focusInEvent(QFocusEvent* event)
{
	QDoubleSpinBox::focusInEvent(event);
	if (parentWidget())
		parentWidget()->update();
}

void AntBaseDoubleSpinBox::focusOutEvent(QFocusEvent* event)
{
	QDoubleSpinBox::focusOutEvent(event);
	if (parentWidget())
		parentWidget()->update();
}

} // namespace ant