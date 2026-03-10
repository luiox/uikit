#pragma once
#include <QWidget>
#include <QSvgRenderer>
#include "IconProvider.h"

namespace ant {

class NoDataWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NoDataWidget(QWidget* parent = nullptr);

    void setSvgPath(const QString& path) { m_iconRole = IconRole::None; m_svgPath = path; m_svgRenderer.load(m_svgPath); update(); }
    void setIconRole(IconRole role) { m_iconRole = role; updateRenderer(); update(); }
    void setText(const QString& text) { m_text = text; update(); }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void updateRenderer();

    QSvgRenderer m_svgRenderer;
    QString m_svgPath;
    IconRole m_iconRole = IconRole::NoData;
    QString m_text = QStringLiteral("暂无数据");
    QColor m_bgColor;
    // svg图有问题 因此要缩放
    float m_scale = 0.65f;    // 高度缩放比例
    float m_factor = 0.18f;   // 整体缩放比例
};

} // namespace ant

using ant::NoDataWidget;
