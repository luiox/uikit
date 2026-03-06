#pragma once

#include <QToolButton>
#include <QSvgRenderer>
#include <QEvent>
#include <QMouseEvent>
#include "IconProvider.h"

namespace ant {

class CustomToolButton : public QToolButton
{
	Q_OBJECT

public:
	explicit CustomToolButton(QSize iconSize, QWidget* parent);
	~CustomToolButton();

	void setSvgIcons(const QString& normalPath, const QString& activePath); // 设置图标
	void setIconRoles(IconRole normalRole, IconRole activeRole);

	bool isBtnChecked() const { return m_checked; };
	void setBtnChecked(bool checked)
	{
		m_checked = checked;
		update();
	}

protected:
	void paintEvent(QPaintEvent* event) override;
	void enterEvent(QEnterEvent* event) override;
	void leaveEvent(QEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
private:
	bool hovered;
	bool m_checked = false;
	QSize m_iconSize;
	QString m_normalPath;
	QString m_activePath;
	IconRole m_normalRole = IconRole::None;
	IconRole m_activeRole = IconRole::None;
};

} // namespace ant

using ant::CustomToolButton;
