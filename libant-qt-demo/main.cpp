#include <QApplication>
#include <QFontDatabase>
#include <QFileInfo>
#include <QDir>

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

QString loadUiFontFamily()
{
	#ifdef Q_OS_WIN
	QFontDatabase fontDb;
	const QStringList installedFamilies = fontDb.families();
	const QStringList preferredSystemFamilies = {
		QStringLiteral("Microsoft YaHei UI"),
		QStringLiteral("Microsoft YaHei"),
		QStringLiteral("Segoe UI")
	};

	for (const QString& family : preferredSystemFamilies)
	{
		if (installedFamilies.contains(family))
		{
			return family;
		}
	}
	#endif

	const QStringList fontResources = {
		QStringLiteral(":/fonts/NotoSansSC-Regular.ttf"),
		QStringLiteral(":/fonts/NotoSansSC-Medium.ttf"),
		QStringLiteral(":/fonts/NotoSansSC-Bold.ttf")
	};

	QString primaryFamily;
	for (const QString& fontResource : fontResources)
	{
		const int fontId = QFontDatabase::addApplicationFont(fontResource);
		if (fontId < 0)
		{
			qWarning("Failed to load font resource: %ls", reinterpret_cast<const wchar_t*>(fontResource.utf16()));
			continue;
		}

		const QStringList families = QFontDatabase::applicationFontFamilies(fontId);
		if (primaryFamily.isEmpty() && !families.isEmpty())
		{
			primaryFamily = families.front();
		}
	}

	return primaryFamily;
}

QFont buildApplicationFont()
{
	const QString family = loadUiFontFamily();
	QFont font;

	if (!family.isEmpty())
	{
		font.setFamily(family);
	}
	else
	{
		font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
	}

	font.setPointSizeF(10.5);
	font.setStyleHint(QFont::SansSerif);
	font.setHintingPreference(QFont::PreferDefaultHinting);
	font.setStyleStrategy(QFont::PreferAntialias);
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