#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFont>

#include "DemoWindow.h"

namespace {

QString findProjectRoot(const QString& startPath)
{
    QDir dir(startPath);
    while (dir.exists())
    {
        if (dir.exists(QStringLiteral("libant-qt"))
            && dir.exists(QStringLiteral("libant-qt-demos"))
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

} // namespace

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("libant-qt-demos"));

    const QString projectRoot = findProjectRoot(QCoreApplication::applicationDirPath());
    QDir::setCurrent(projectRoot);

    QFont font(QStringLiteral("Microsoft YaHei UI"));
    font.setPointSizeF(10.5);
    QApplication::setFont(font);

    ant::DemoWindow window;
    window.show();
    return app.exec();
}