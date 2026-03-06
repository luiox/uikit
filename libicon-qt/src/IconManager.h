#pragma once

#include <QByteArray>
#include <QColor>
#include <QHash>
#include <QIcon>
#include <QPixmap>
#include <QSize>
#include <QString>

#include <mutex>

#include "icons.h"

namespace icon::qt {

class IconManager final {
public:
    static IconManager& instance();

    void setDynamicRoot(const QString& rootPath);
    QString dynamicRoot() const;

    QIcon icon(::icon::Icon iconId, QSize size = QSize(16, 16), QColor tint = {});
    QIcon icon(const char* iconId, QSize size = QSize(16, 16), QColor tint = {});

    QPixmap pixmap(::icon::Icon iconId, QSize size, QColor tint = {});
    QPixmap pixmap(const char* iconId, QSize size, QColor tint = {});

    void clearCache();

private:
    IconManager() = default;

    static QSize normalizedSize(const QSize& size);
    static QString makeCacheKey(::icon::Icon iconId, const QSize& size, const QColor& tint);
    static QByteArray applyTint(const QByteArray& svgData, const QColor& tint);

    QByteArray loadSvg(::icon::Icon iconId) const;

private:
    mutable std::mutex mutex_;
    QString dynamicRoot_;
    QHash<QString, QIcon> iconCache_;
    QHash<QString, QPixmap> pixmapCache_;
};

} // namespace icon::qt
