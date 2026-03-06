#include "CustomToolButton.h"
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QMouseEvent>
#include "DesignSystem.h"

namespace ant {

CustomToolButton::CustomToolButton(QSize iconSize, QWidget* parent)
	: QToolButton(parent), hovered(false), m_iconSize(iconSize)
{
	setCheckable(false);     // 非 toggle 状态
	setCursor(Qt::PointingHandCursor);
	setAttribute(Qt::WA_Hover); // 开启 hover 事件
	setStyleSheet("border: none; background: transparent;"); // 去除边框
}

CustomToolButton::~CustomToolButton()
{
}

void CustomToolButton::setSvgIcons(const QString& normalPath, const QString& activePath)
{
	m_normalRole = IconRole::None;
	m_activeRole = IconRole::None;
	m_normalPath = normalPath;
	m_activePath = activePath;
	update();
}

void CustomToolButton::setIconRoles(IconRole normalRole, IconRole activeRole)
{
	m_normalRole = normalRole;
	m_activeRole = activeRole;
	m_normalPath.clear();
	m_activePath.clear();
	update();
}

void CustomToolButton::enterEvent(QEnterEvent* event)
{
	if (!isEnabled()) return;
	hovered = true;
	update();
	QToolButton::enterEvent(event);
}

void CustomToolButton::leaveEvent(QEvent* event)
{
	hovered = false;
	update();
	QToolButton::leaveEvent(event);
}

void CustomToolButton::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_checked = true;
		update(); // 刷新图标
	}

	QToolButton::mousePressEvent(event); // 保留原事件逻辑
}

void CustomToolButton::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	painter.setPen(Qt::NoPen);
	painter.setRenderHint(QPainter::Antialiasing);

	// 只有 hovered 或 checked 时才绘制背景
	if (hovered || m_checked)
	{
		// 背景颜色更亮一些
		QColor bgColor = DesignSystem::instance()->currentTheme().toolBtnBgColor;

		// 缩小绘制圆角背景
		QRectF rect = this->rect().adjusted(7, 7, -7, -7);
		QPainterPath path;
		path.addRoundedRect(rect, 8, 8);
		painter.fillPath(path, bgColor);
	}

	const bool isDark = DesignSystem::instance()->themeMode() == DesignSystem::Dark;
	const IconRole currentRole = m_checked ? m_activeRole : m_normalRole;
	const QString currentPath = m_checked ? m_activePath : m_normalPath;
	const QColor tint = m_checked
		? DesignSystem::instance()->primaryColor()
		: DesignSystem::instance()->currentTheme().tabTextColor;

	QPixmap pm;
	if (currentRole != IconRole::None)
	{
		pm = IconProvider::pixmap(currentRole, isDark, m_iconSize, tint);
	}
	else if (!currentPath.isEmpty())
	{
		pm = IconProvider::pixmap(currentPath, m_iconSize, tint);
	}

	if (!pm.isNull())
	{
		QPoint iconTopLeft((width() - m_iconSize.width()) / 2, (height() - m_iconSize.height()) / 2);
		QRect iconRect(iconTopLeft, m_iconSize);
		painter.drawPixmap(iconRect, pm);
	}
}

} // namespace ant