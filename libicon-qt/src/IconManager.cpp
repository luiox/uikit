#include "IconManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QRectF>
#include <QSvgRenderer>

namespace {
constexpr int kDefaultIconExtent = 16;

QString defaultDynamicRoot()
{
    return QDir::currentPath();
}

QString resolveDynamicPath(const QString& configuredRoot, const QString& relativePath)
{
    if (relativePath.isEmpty())
    {
        return {};
    }

    const QFileInfo directInfo(relativePath);
    if (directInfo.isAbsolute() && directInfo.exists())
    {
        return directInfo.absoluteFilePath();
    }

    const QStringList candidates = {
        configuredRoot,
        QDir::currentPath(),
        QCoreApplication::applicationDirPath()
    };

    for (const QString& root : candidates)
    {
        if (root.isEmpty())
        {
            continue;
        }
        const QString candidate = QDir(root).filePath(relativePath);
        if (QFileInfo::exists(candidate))
        {
            return QDir::cleanPath(candidate);
        }
    }

    return {};
}

QString colorToken(const QColor& tint)
{
    return tint.isValid() ? tint.name(QColor::HexRgb) : QString();
}

} // namespace

namespace icon::qt {

IconManager& IconManager::instance()
{
    static IconManager manager;
    return manager;
}

void IconManager::setDynamicRoot(const QString& rootPath)
{
    std::scoped_lock lock(mutex_);
    dynamicRoot_ = QDir::fromNativeSeparators(rootPath);
    iconCache_.clear();
    pixmapCache_.clear();
}

QString IconManager::dynamicRoot() const
{
    std::scoped_lock lock(mutex_);
    return dynamicRoot_.isEmpty() ? defaultDynamicRoot() : dynamicRoot_;
}

QIcon IconManager::icon(::icon::Icon iconId, QSize size, QColor tint)
{
    const QSize normalized = normalizedSize(size);
    const QString key = makeCacheKey(iconId, normalized, tint);

    {
        std::scoped_lock lock(mutex_);
        const auto it = iconCache_.constFind(key);
        if (it != iconCache_.cend())
        {
            return *it;
        }
    }

    const QPixmap pm = pixmap(iconId, normalized, tint);
    if (pm.isNull())
    {
        return {};
    }

    const QIcon built(pm);
    std::scoped_lock lock(mutex_);
    iconCache_.insert(key, built);
    return built;
}

QIcon IconManager::icon(const char* iconId, QSize size, QColor tint)
{
    const ::icon::IconAsset* asset = ::icon::FindIcon(iconId);
    if (asset == nullptr)
    {
        return {};
    }
    return icon(asset->icon, size, tint);
}

QPixmap IconManager::pixmap(::icon::Icon iconId, QSize size, QColor tint)
{
    const QSize normalized = normalizedSize(size);
    const QString key = makeCacheKey(iconId, normalized, tint);

    {
        std::scoped_lock lock(mutex_);
        const auto it = pixmapCache_.constFind(key);
        if (it != pixmapCache_.cend())
        {
            return *it;
        }
    }

    QByteArray svgData = loadSvg(iconId);
    if (svgData.isEmpty())
    {
        return {};
    }

    svgData = applyTint(svgData, tint);

    QSvgRenderer renderer(svgData);
    if (!renderer.isValid())
    {
        return {};
    }

    QPixmap pm(normalized);
    pm.fill(Qt::transparent);

    QPainter painter(&pm);
    renderer.render(&painter, QRectF(0.0, 0.0, normalized.width(), normalized.height()));
    painter.end();

    std::scoped_lock lock(mutex_);
    pixmapCache_.insert(key, pm);
    return pm;
}

QPixmap IconManager::pixmap(const char* iconId, QSize size, QColor tint)
{
    const ::icon::IconAsset* asset = ::icon::FindIcon(iconId);
    if (asset == nullptr)
    {
        return {};
    }
    return pixmap(asset->icon, size, tint);
}

void IconManager::clearCache()
{
    std::scoped_lock lock(mutex_);
    iconCache_.clear();
    pixmapCache_.clear();
}

QSize IconManager::normalizedSize(const QSize& size)
{
    const int width = size.width() > 0 ? size.width() : kDefaultIconExtent;
    const int height = size.height() > 0 ? size.height() : kDefaultIconExtent;
    return QSize(width, height);
}

QString IconManager::makeCacheKey(::icon::Icon iconId, const QSize& size, const QColor& tint)
{
    const QString tintValue = tint.isValid()
        ? QString::number(static_cast<quint32>(tint.rgba()), 16)
        : QStringLiteral("none");
    return QStringLiteral("%1:%2x%3:%4")
        .arg(static_cast<int>(iconId))
        .arg(size.width())
        .arg(size.height())
        .arg(tintValue);
}

QByteArray IconManager::applyTint(const QByteArray& svgData, const QColor& tint)
{
    if (!tint.isValid())
    {
        return svgData;
    }

    QString text = QString::fromUtf8(svgData);
    const QString color = colorToken(tint);
    const QStringList replacements = {
        QStringLiteral("currentColor"),
        QStringLiteral("#000000"),
        QStringLiteral("#000"),
        QStringLiteral("black"),
        QStringLiteral("rgb(0,0,0)")
    };

    for (const QString& token : replacements)
    {
        text.replace(token, color, Qt::CaseInsensitive);
    }

    return text.toUtf8();
}

QByteArray IconManager::loadSvg(::icon::Icon iconId) const
{
    if (iconId == ::icon::Icon::None)
    {
        return {};
    }

#if LIBICON_CORE_ENABLE_EMBED
    if (const ::icon::IconAsset* asset = ::icon::FindIcon(iconId))
    {
        if (asset->svg != nullptr && asset->size > 0)
        {
            return QByteArray(asset->svg, static_cast<int>(asset->size));
        }
    }
#endif

    const char* relativePath = ::icon::GetDynamicPath(iconId);
    if (relativePath == nullptr || relativePath[0] == '\0')
    {
        return {};
    }

    const QString path = resolveDynamicPath(dynamicRoot(), QString::fromUtf8(relativePath));
    if (path.isEmpty())
    {
        return {};
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        return {};
    }

    return file.readAll();
}

} // namespace icon::qt
