#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QFontDatabase>
#include <QStringList>

#include "LibAntQtDemoWindow.h"

namespace {

QString findProjectRoot(const QString& startPath)
{
	QDir dir(startPath);
	while (dir.exists())
	{
		if (dir.exists(QStringLiteral("libant-qt"))
			&& dir.exists(QStringLiteral("libant-qt-demo"))
			&& dir.exists(QStringLiteral("xmake.lua")))
		{
			return dir.absolutePath();
		}

		if (!dir.cdUp())
		{
			break;
		}
	}

	return startPath;
}

QStringList buildUiFontFamilies()
{
	QStringList families;

	const QStringList fontResources = {
		QStringLiteral(":/fonts/NotoSansSC-Regular.ttf"),
		QStringLiteral(":/fonts/NotoSansSC-Medium.ttf"),
		QStringLiteral(":/fonts/NotoSansSC-Bold.ttf")
	};

	for (const QString& fontResource : fontResources)
	{
		const int fontId = QFontDatabase::addApplicationFont(fontResource);
		if (fontId < 0)
		{
			qWarning("Failed to load font resource: %ls", reinterpret_cast<const wchar_t*>(fontResource.utf16()));
			continue;
		}

		const QStringList resourceFamilies = QFontDatabase::applicationFontFamilies(fontId);
		for (const QString& family : resourceFamilies)
		{
			if (!family.isEmpty() && !families.contains(family))
			{
				families.push_back(family);
			}
		}
	}

	QFontDatabase fontDb;
	const QStringList installedFamilies = fontDb.families();
	const QStringList preferredSystemFamilies = {
		QStringLiteral("Microsoft YaHei UI"),
		QStringLiteral("Microsoft YaHei"),
		QStringLiteral("Segoe UI"),
		QStringLiteral("Arial")
	};

	for (const QString& family : preferredSystemFamilies)
	{
		if (installedFamilies.contains(family) && !families.contains(family))
		{
			families.push_back(family);
		}
	}

	return families;
}

QFont buildApplicationFont()
{
	const QStringList families = buildUiFontFamilies();
	QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);

	if (!families.isEmpty())
	{
		font.setFamilies(families);
		font.setFamily(families.front());
	}

	font.setPointSizeF(10.5);
	font.setStyleHint(QFont::SansSerif);
	font.setHintingPreference(QFont::PreferDefaultHinting);
	font.setStyleStrategy(QFont::PreferAntialias);

	#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
	if (!families.isEmpty())
	{
		QFontDatabase::setApplicationFallbackFontFamilies(QChar::Script_Han, families);
	}
	#endif

	return font;
}

} // namespace

int main(int argc, char* argv[])
{
#ifdef Q_OS_LINUX
	qputenv("QT_QPA_PLATFORM", "xcb"); // 避免 Wayland
#endif
	QApplication a(argc, argv);

	const QString projectRoot = findProjectRoot(QCoreApplication::applicationDirPath());
	QDir::setCurrent(projectRoot);
	QApplication::setFont(buildApplicationFont());

	ant::LibAntQtDemoWindow w;
	w.show();

	return a.exec();
}
