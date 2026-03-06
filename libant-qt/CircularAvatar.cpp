#include "CircularAvatar.h"
#include <qpainterpath.h>
#include <QEvent>
#include <QEnterEvent>
#include <QTimer>
#include "AntTooltipManager.h"

namespace ant {

CircularAvatar::CircularAvatar(QSize size, QString prevImgPath, QString afterImgPath, QWidget* parent)
	: QWidget(parent)
{
	setFixedSize(size);

	if (!m_bubble)
	{
		m_bubble = new BubbleViewController(this);
		connect(this, &CircularAvatar::playAnim, m_bubble, &BubbleViewController::showAnimated);
		connect(this, &CircularAvatar::hideAnim, m_bubble, &BubbleViewController::hideAnimated);
		connect(m_bubble, &BubbleViewController::requestHide, this, &CircularAvatar::checkShouldHideBubble);
		connect(m_bubble, &BubbleViewController::exitLogin, this, &CircularAvatar::allowLogin);
	}

	setImgs(prevImgPath, afterImgPath);
}

CircularAvatar::CircularAvatar(QSize size, IconRole prevRole, IconRole afterRole, QWidget* parent)
	: CircularAvatar(size, QString(), QString(), parent)
{
	setImgs(prevRole, afterRole);
}

CircularAvatar::~CircularAvatar()
{
}

void CircularAvatar::setImgs(QString prevImgPath, QString afterImgPath)
{
	if (m_prevSvg)
	{
		delete m_prevSvg;
		m_prevSvg = nullptr;
	}
	if (m_afterSvg)
	{
		delete m_afterSvg;
		m_afterSvg = nullptr;
	}

	const QByteArray prevBytes = IconProvider::svgData(prevImgPath);
	const QByteArray afterBytes = IconProvider::svgData(afterImgPath);
	if (!prevBytes.isEmpty())
	{
		m_prevSvg = new QSvgRenderer(prevBytes, this);
	}
	else if (!prevImgPath.trimmed().isEmpty())
	{
		m_prevSvg = new QSvgRenderer(prevImgPath, this);
	}

	if (!afterBytes.isEmpty())
	{
		m_afterSvg = new QSvgRenderer(afterBytes, this);
	}
	else if (!afterImgPath.trimmed().isEmpty())
	{
		m_afterSvg = new QSvgRenderer(afterImgPath, this);
	}
	update();
}

void CircularAvatar::setImgs(IconRole prevRole, IconRole afterRole)
{
	if (m_prevSvg)
	{
		delete m_prevSvg;
		m_prevSvg = nullptr;
	}
	if (m_afterSvg)
	{
		delete m_afterSvg;
		m_afterSvg = nullptr;
	}

	const QByteArray prevBytes = IconProvider::svgData(prevRole);
	const QByteArray afterBytes = IconProvider::svgData(afterRole);
	m_prevSvg = prevBytes.isEmpty() ? nullptr : new QSvgRenderer(prevBytes, this);
	m_afterSvg = afterBytes.isEmpty() ? nullptr : new QSvgRenderer(afterBytes, this);
	update();
}

void CircularAvatar::addDialog(DialogViewController* dialog)
{
	m_dialogView = dialog;
	connect(this, &CircularAvatar::showDialog, m_dialogView, &DialogViewController::showAnim);
}

void CircularAvatar::allowLogin(bool loginState)
{
	m_isLogin = loginState;
	m_isClicked = loginState ? true : false;
	update();
}

void CircularAvatar::setAvatar(QString svgFilePath)
{
	Q_UNUSED(svgFilePath);
}

void CircularAvatar::paintEvent(QPaintEvent* e)
{
	Q_UNUSED(e);

	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing, true);

	QPainterPath path;
	path.addEllipse(rect());
	p.setClipPath(path);

	if (m_prevSvg && m_prevSvg->isValid() && m_afterSvg && m_afterSvg->isValid())
	{
		if (!m_isClicked)
		{
			m_prevSvg->render(&p, rect());
		}
		else
		{
			m_afterSvg->render(&p, rect());
		}
	}
}

void CircularAvatar::enterEvent(QEnterEvent* e)
{
	QWidget::enterEvent(e);

	setCursor(Qt::PointingHandCursor);
	if (!m_lastEnterTime.isValid() || m_lastEnterTime.elapsed() > m_enterIntervalMs)
	{
		m_lastEnterTime.restart();

		if (!m_isEnter)
		{
			m_isEnter = true;

			if (m_isLogin)
			{
				QPoint rightPos = mapToGlobal(rect().topRight());
				emit playAnim(QPoint(rightPos.x(), rightPos.y() - 6));
			}
			else
			{
				AntTooltipManager::instance()->showTooltip(this, "未登录,登录每日签到可获得积分.", AntTooltipManager::Position::Right);
			}
		}
	}
}

void CircularAvatar::leaveEvent(QEvent* e)
{
	QWidget::leaveEvent(e);
	setCursor(Qt::ArrowCursor);
	QTimer::singleShot(100, this, [this]()
		{
			checkShouldHideBubble();
		});
	AntTooltipManager::instance()->hideTooltip();
}

void CircularAvatar::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		emit showDialog(m_isLogin ? MaterialDialog::Profile : MaterialDialog::Login);
	}
}

void CircularAvatar::checkShouldHideBubble()
{
	const bool avatarHovered = this->underMouse();
	const bool bubbleHovered = m_bubble && m_bubble->underMouse();

	if (!avatarHovered && !bubbleHovered)
	{
		m_isEnter = false;
		emit hideAnim();
	}
}

} // namespace ant