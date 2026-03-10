#include "IconProvider.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QRectF>
#include <QSvgRenderer>
#include <QStringList>

#include "IconManager.h"

namespace {

QString applyTint(const QString& text, const QColor& tint)
{
	if (!tint.isValid())
	{
		return text;
	}

	QString tinted = text;
	const QString color = tint.name(QColor::HexRgb);
	const QStringList replacements = {
		QStringLiteral("currentColor"),
		QStringLiteral("#000000"),
		QStringLiteral("#000"),
		QStringLiteral("black"),
		QStringLiteral("rgb(0,0,0)")
	};

	for (const QString& token : replacements)
	{
		tinted.replace(token, color, Qt::CaseInsensitive);
	}

	return tinted;
}

QByteArray loadSvgFile(const QString& path, const QColor& tint)
{
	if (path.isEmpty())
	{
		return {};
	}

	QFile file(path);
	if (!file.open(QIODevice::ReadOnly))
	{
		return {};
	}

	const QByteArray bytes = file.readAll();
	if (!tint.isValid())
	{
		return bytes;
	}

	return applyTint(QString::fromUtf8(bytes), tint).toUtf8();
}

QString resolveDynamicPath(const QString& relativePath)
{
	if (relativePath.isEmpty())
	{
		return {};
	}

	const QFileInfo direct(relativePath);
	if (direct.isAbsolute() && direct.exists())
	{
		return direct.absoluteFilePath();
	}

	const QStringList candidates = {
		QDir::currentPath(),
		QCoreApplication::applicationDirPath()
	};

	for (const QString& root : candidates)
	{
		const QString candidate = QDir(root).filePath(relativePath);
		if (QFileInfo::exists(candidate))
		{
			return QDir::cleanPath(candidate);
		}
	}

	return {};
}

} // namespace

namespace ant {

void IconProvider::setDynamicRoot(const QString& rootPath)
{
	icon::qt::IconManager::instance().setDynamicRoot(rootPath);
}

QIcon IconProvider::icon(IconRole role, bool dark, QSize size, QColor tint)
{
	const QPixmap pm = pixmap(role, dark, size, tint);
	return pm.isNull() ? QIcon() : QIcon(pm);
}

QPixmap IconProvider::pixmap(IconRole role, bool dark, QSize size, QColor tint)
{
	if (const ant_core_icon::Icon iconId = coreIcon(role); iconId != ant_core_icon::Icon::None)
	{
		return icon::qt::IconManager::instance().pixmap(iconId, size, tint);
	}

	return pixmap(resourcePath(role, dark), size, tint);
}

QByteArray IconProvider::svgData(IconRole role, bool dark, QColor tint)
{
	if (const ant_core_icon::Icon iconId = coreIcon(role); iconId != ant_core_icon::Icon::None)
	{
#if LIBICON_CORE_ENABLE_EMBED
		if (const char* embedded = ::icon::GetEmbeddedSvg(iconId))
		{
			QByteArray bytes(embedded);
			if (!tint.isValid())
			{
				return bytes;
			}
			return applyTint(QString::fromUtf8(bytes), tint).toUtf8();
		}
#endif
		if (const char* dynamic = ::icon::GetDynamicPath(iconId))
		{
			return loadSvgFile(resolveDynamicPath(QString::fromUtf8(dynamic)), tint);
		}
	}

	return svgData(resourcePath(role, dark), tint);
}

QString IconProvider::resourcePath(IconRole role, bool dark)
{
	switch (role)
	{
	case IconRole::ThemeSun:
	case IconRole::ThemeMoon:
	case IconRole::WindowMin:
	case IconRole::WindowMax:
	case IconRole::WindowClose:
	case IconRole::WindowRestore:
	case IconRole::NavHome:
	case IconRole::NavHomeActive:
	case IconRole::NavFunction:
	case IconRole::NavFunctionActive:
	case IconRole::NavSettings:
	case IconRole::NavSettingsActive:
	case IconRole::NavAbout:
	case IconRole::NavAboutActive:
	case IconRole::MsgInfo:
	case IconRole::MsgSuccess:
	case IconRole::MsgError:
	case IconRole::MsgWarning:
	case IconRole::AvatarGuest:
	case IconRole::BrandGithub:
	case IconRole::Back:
	case IconRole::TabClose:
	case IconRole::Search:
	case IconRole::Prev:
	case IconRole::PrevDisabled:
	case IconRole::Next:
	case IconRole::NextDisabled:
	case IconRole::ArrowUp:
	case IconRole::ArrowDown:
	case IconRole::ArrowRight:
	case IconRole::ArrowRightSmall:
	case IconRole::NoData:
	case IconRole::GoTo:
	case IconRole::Eye:
	case IconRole::EyeSlash:
	case IconRole::CarouselArrow:
		return {};
	case IconRole::None:
	default:
		return {};
	}
}

QIcon IconProvider::icon(const QString& path, QSize size, QColor tint)
{
	const QPixmap pm = pixmap(path, size, tint);
	return pm.isNull() ? QIcon() : QIcon(pm);
}

QPixmap IconProvider::pixmap(const QString& path, QSize size, QColor tint)
{
	if (path.isEmpty())
	{
		return {};
	}

	const QSize actualSize = QSize(size.width() > 0 ? size.width() : 16, size.height() > 0 ? size.height() : 16);
	const QFileInfo info(path);
	const QString suffix = info.suffix().toLower();

	if (suffix == QStringLiteral("svg") || path.startsWith(QLatin1String(":/")))
	{
		const QByteArray bytes = svgData(path, tint);
		if (!bytes.isEmpty())
		{
			QSvgRenderer renderer(bytes);
			if (renderer.isValid())
			{
				QPixmap pm(actualSize);
				pm.fill(Qt::transparent);
				QPainter painter(&pm);
				renderer.render(&painter, QRectF(0.0, 0.0, actualSize.width(), actualSize.height()));
				return pm;
			}
		}
	}

	QPixmap pm(path);
	if (pm.isNull())
	{
		return {};
	}
	return pm.scaled(actualSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QByteArray IconProvider::svgData(const QString& path, QColor tint)
{
	return loadSvgFile(path, tint);
}

ant_core_icon::Icon IconProvider::coreIcon(IconRole role)
{
	switch (role)
	{
	case IconRole::ThemeSun: return ant_core_icon::Icon::ANT_ThemeSun;
	case IconRole::ThemeMoon: return ant_core_icon::Icon::ANT_ThemeMoon;
	case IconRole::WindowMin: return ant_core_icon::Icon::ANT_WindowMin;
	case IconRole::WindowMax: return ant_core_icon::Icon::ANT_WindowMax;
	case IconRole::WindowClose: return ant_core_icon::Icon::ANT_WindowClose;
	case IconRole::WindowRestore: return ant_core_icon::Icon::ANT_WindowRestore;
	case IconRole::NavHome: return ant_core_icon::Icon::ANT_NavHome;
	case IconRole::NavHomeActive: return ant_core_icon::Icon::ANT_NavHomeActive;
	case IconRole::NavFunction: return ant_core_icon::Icon::ANT_NavFunction;
	case IconRole::NavFunctionActive: return ant_core_icon::Icon::ANT_NavFunctionActive;
	case IconRole::NavSettings: return ant_core_icon::Icon::ANT_NavSettings;
	case IconRole::NavSettingsActive: return ant_core_icon::Icon::ANT_NavSettingsActive;
	case IconRole::NavAbout: return ant_core_icon::Icon::ANT_NavAbout;
	case IconRole::NavAboutActive: return ant_core_icon::Icon::ANT_NavAboutActive;
	case IconRole::MsgInfo: return ant_core_icon::Icon::ANT_MsgInfo;
	case IconRole::MsgSuccess: return ant_core_icon::Icon::ANT_MsgSuccess;
	case IconRole::MsgError: return ant_core_icon::Icon::ANT_MsgError;
	case IconRole::MsgWarning: return ant_core_icon::Icon::ANT_MsgWarning;
	case IconRole::AvatarGuest: return ant_core_icon::Icon::ANT_AvatarGuest;
	case IconRole::BrandGithub: return ant_core_icon::Icon::ANT_BrandGithub;
	case IconRole::Back: return ant_core_icon::Icon::ANT_Back;
	case IconRole::TabClose: return ant_core_icon::Icon::ANT_TabClose;
	case IconRole::Search: return ant_core_icon::Icon::ANT_Search;
	case IconRole::Prev:
	case IconRole::PrevDisabled: return ant_core_icon::Icon::ANT_Prev;
	case IconRole::Next:
	case IconRole::NextDisabled: return ant_core_icon::Icon::ANT_Next;
	case IconRole::ArrowUp: return ant_core_icon::Icon::ANT_ArrowUp;
	case IconRole::ArrowDown: return ant_core_icon::Icon::ANT_ArrowDown;
	case IconRole::ArrowRight: return ant_core_icon::Icon::ANT_ArrowRight;
	case IconRole::ArrowRightSmall: return ant_core_icon::Icon::ANT_ArrowRightSmall;
	case IconRole::NoData: return ant_core_icon::Icon::ANT_NoData;
	case IconRole::GoTo: return ant_core_icon::Icon::ANT_GoTo;
	case IconRole::Eye: return ant_core_icon::Icon::ANT_Eye;
	case IconRole::EyeSlash: return ant_core_icon::Icon::ANT_EyeSlash;
	case IconRole::CarouselArrow: return ant_core_icon::Icon::ANT_CarouselArrow;
	case IconRole::None:
	default:
		return ant_core_icon::Icon::None;
	}
}

} // namespace ant
