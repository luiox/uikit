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
	if (const ::icon::Icon iconId = coreIcon(role); iconId != ::icon::Icon::None)
	{
		return icon::qt::IconManager::instance().pixmap(iconId, size, tint);
	}

	return pixmap(resourcePath(role, dark), size, tint);
}

QByteArray IconProvider::svgData(IconRole role, bool dark, QColor tint)
{
	if (const ::icon::Icon iconId = coreIcon(role); iconId != ::icon::Icon::None)
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
	case IconRole::ThemeSun: return QStringLiteral(":/Imgs/sun.svg");
	case IconRole::ThemeMoon: return QStringLiteral(":/Imgs/moon.svg");
	case IconRole::WindowMin: return dark ? QStringLiteral(":/Imgs/MinimizeDark.svg") : QStringLiteral(":/Imgs/Minimize.svg");
	case IconRole::WindowMax: return dark ? QStringLiteral(":/Imgs/MaximizeDark.svg") : QStringLiteral(":/Imgs/Maximize.svg");
	case IconRole::WindowClose: return dark ? QStringLiteral(":/Imgs/ShutDownDark.svg") : QStringLiteral(":/Imgs/ShutDown.svg");
	case IconRole::WindowRestore: return dark ? QStringLiteral(":/Imgs/RestoreDark.svg") : QStringLiteral(":/Imgs/Restore.svg");
	case IconRole::NavHome: return dark ? QStringLiteral(":/Imgs/homeDark.svg") : QStringLiteral(":/Imgs/home.svg");
	case IconRole::NavHomeActive: return dark ? QStringLiteral(":/Imgs/home_activeDark.svg") : QStringLiteral(":/Imgs/home_active.svg");
	case IconRole::NavFunction: return dark ? QStringLiteral(":/Imgs/functionDark.svg") : QStringLiteral(":/Imgs/function.svg");
	case IconRole::NavFunctionActive: return dark ? QStringLiteral(":/Imgs/functionDark_active.svg") : QStringLiteral(":/Imgs/function_active.svg");
	case IconRole::NavSettings: return dark ? QStringLiteral(":/Imgs/settingsDark.svg") : QStringLiteral(":/Imgs/settings.svg");
	case IconRole::NavSettingsActive: return dark ? QStringLiteral(":/Imgs/settings_activeDark.svg") : QStringLiteral(":/Imgs/settings_active.svg");
	case IconRole::NavAbout: return dark ? QStringLiteral(":/Imgs/aboutDark.svg") : QStringLiteral(":/Imgs/about.svg");
	case IconRole::NavAboutActive: return dark ? QStringLiteral(":/Imgs/about_activeDark.svg") : QStringLiteral(":/Imgs/about_active.svg");
	case IconRole::MsgInfo: return QStringLiteral(":/Imgs/info.svg");
	case IconRole::MsgSuccess: return QStringLiteral(":/Imgs/true.svg");
	case IconRole::MsgError: return QStringLiteral(":/Imgs/error.svg");
	case IconRole::MsgWarning: return QStringLiteral(":/Imgs/warning.svg");
	case IconRole::AvatarGuest: return QStringLiteral(":/Imgs/noLogin.svg");
	case IconRole::BrandGithub: return QStringLiteral(":/Imgs/github.svg");
	case IconRole::Back: return QStringLiteral(":/Imgs/back.svg");
	case IconRole::TabClose: return dark ? QStringLiteral(":/Imgs/closeItemDark.svg") : QStringLiteral(":/Imgs/closeItem.svg");
	case IconRole::Search: return QStringLiteral(":/Imgs/search.svg");
	case IconRole::Prev: return dark ? QStringLiteral(":/Imgs/PreviousBtnDark.svg") : QStringLiteral(":/Imgs/PreviousBtn.svg");
	case IconRole::PrevDisabled: return dark ? QStringLiteral(":/Imgs/previousBtnDisableDark.svg") : QStringLiteral(":/Imgs/previousBtnDisable.svg");
	case IconRole::Next: return dark ? QStringLiteral(":/Imgs/nextBtnDark.svg") : QStringLiteral(":/Imgs/nextBtn.svg");
	case IconRole::NextDisabled: return dark ? QStringLiteral(":/Imgs/nextBtnDisableDark.svg") : QStringLiteral(":/Imgs/nextBtnDisable.svg");
	case IconRole::ArrowUp: return QStringLiteral(":/Imgs/upArrow.svg");
	case IconRole::ArrowDown: return QStringLiteral(":/Imgs/downArrow.svg");
	case IconRole::ArrowRightSmall: return QStringLiteral(":/Imgs/smallRightArrow.svg");
	case IconRole::NoData: return QStringLiteral(":/Imgs/noData.svg");
	case IconRole::GoTo: return QStringLiteral(":/Imgs/goTo.svg");
	case IconRole::Eye: return QStringLiteral(":/Imgs/eye-filled.svg");
	case IconRole::EyeSlash: return QStringLiteral(":/Imgs/eye-slash-filled.svg");
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

::icon::Icon IconProvider::coreIcon(IconRole role)
{
	switch (role)
	{
	case IconRole::WindowMin: return ::icon::Icon::Minimize;
	case IconRole::WindowMax: return ::icon::Icon::Maximize;
	case IconRole::WindowClose: return ::icon::Icon::Close;
	case IconRole::WindowRestore: return ::icon::Icon::Restore;
	case IconRole::NavHome:
	case IconRole::NavHomeActive: return ::icon::Icon::Home;
	case IconRole::NavSettings:
	case IconRole::NavSettingsActive: return ::icon::Icon::Settings;
	case IconRole::MsgInfo: return ::icon::Icon::Info;
	case IconRole::MsgSuccess: return ::icon::Icon::Check;
	case IconRole::MsgError: return ::icon::Icon::Error;
	case IconRole::MsgWarning: return ::icon::Icon::Warning;
	case IconRole::Back: return ::icon::Icon::Back;
	case IconRole::Search: return ::icon::Icon::Search;
	case IconRole::Prev: return ::icon::Icon::Prev;
	case IconRole::Next: return ::icon::Icon::Next;
	case IconRole::ArrowUp: return ::icon::Icon::Up;
	case IconRole::None:
	case IconRole::ThemeSun:
	case IconRole::ThemeMoon:
	case IconRole::NavFunction:
	case IconRole::NavFunctionActive:
	case IconRole::NavAbout:
	case IconRole::NavAboutActive:
	case IconRole::AvatarGuest:
	case IconRole::BrandGithub:
	case IconRole::TabClose:
	case IconRole::PrevDisabled:
	case IconRole::NextDisabled:
	case IconRole::ArrowDown:
	case IconRole::ArrowRightSmall:
	case IconRole::NoData:
	case IconRole::GoTo:
	case IconRole::Eye:
	case IconRole::EyeSlash:
	default:
		return ::icon::Icon::None;
	}
}

} // namespace ant
