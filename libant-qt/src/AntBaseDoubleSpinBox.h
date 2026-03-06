#pragma once

#include <QDoubleSpinBox>
#include <QToolButton>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include "DesignSystem.h"

namespace ant {

class AntBaseDoubleSpinBox : public QDoubleSpinBox
{
	Q_OBJECT
		Q_PROPERTY(int buttonX READ buttonX WRITE setButtonX)

public:
	AntBaseDoubleSpinBox(QWidget* parent = nullptr);
	~AntBaseDoubleSpinBox();

protected:
	void enterEvent(QEnterEvent* event) override;
	void leaveEvent(QEvent* event) override;
	void stepBy(int steps) override;
	void resizeEvent(QResizeEvent* event) override;
	void focusInEvent(QFocusEvent* event) override;
	void focusOutEvent(QFocusEvent* event) override;
private:
	void updateButtonsPosition();
	int buttonX() const;
	void setButtonX(int x);

private:
	QToolButton* m_plusBtn;
	QToolButton* m_minusBtn;
	QPropertyAnimation* m_animation;
	QGraphicsDropShadowEffect* m_shadowEffect;
	Theme theme = DesignSystem::instance()->currentTheme();

	int m_buttonX;
};

} // namespace ant

using ant::AntBaseDoubleSpinBox;
