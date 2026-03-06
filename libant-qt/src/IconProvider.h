#pragma once

#include <QByteArray>
#include <QColor>
#include <QIcon>
#include <QPixmap>
#include <QSize>
#include <QString>

#include "icons.h"

namespace ant {

enum class IconRole {
	None = 0,
	ThemeSun,
	ThemeMoon,
	WindowMin,
	WindowMax,
	WindowClose,
	WindowRestore,
	NavHome,
	NavHomeActive,
	NavFunction,
	NavFunctionActive,
	NavSettings,
	NavSettingsActive,
	NavAbout,
	NavAboutActive,
	MsgInfo,
	MsgSuccess,
	MsgError,
	MsgWarning,
	AvatarGuest,
	BrandGithub,
	Back,
	TabClose,
	Search,
	Prev,
	PrevDisabled,
	Next,
	NextDisabled,
	ArrowUp,
	ArrowDown,
	ArrowRight,
	ArrowRightSmall,
	NoData,
	GoTo,
	Eye,
	EyeSlash,
	CarouselArrow
};

class IconProvider final
{
public:
	static void setDynamicRoot(const QString& rootPath);

	static QIcon icon(IconRole role, bool dark = false, QSize size = QSize(16, 16), QColor tint = {});
	static QPixmap pixmap(IconRole role, bool dark, QSize size, QColor tint = {});
	static QByteArray svgData(IconRole role, bool dark = false, QColor tint = {});
	static QString resourcePath(IconRole role, bool dark = false);

	static QIcon icon(const QString& path, QSize size = QSize(16, 16), QColor tint = {});
	static QPixmap pixmap(const QString& path, QSize size, QColor tint = {});
	static QByteArray svgData(const QString& path, QColor tint = {});

private:
	static ::icon::Icon coreIcon(IconRole role);
};

} // namespace ant
