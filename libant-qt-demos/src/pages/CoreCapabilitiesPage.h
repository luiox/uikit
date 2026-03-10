#pragma once

#include <QFrame>
#include <QLabel>
#include <QWidget>

#include "IconProvider.h"

namespace ant {

class CoreCapabilitiesPage : public QWidget
{
    Q_OBJECT

public:
    explicit CoreCapabilitiesPage(QWidget* parent = nullptr);

private:
    void refreshThemePreview();
    void refreshIconPreview();
    void buildBlurPreview();

    QLabel* m_themeModeLabel = nullptr;
    QList<QFrame*> m_colorSwatches;
    QList<QLabel*> m_colorLabels;
    QList<QLabel*> m_iconLabels;
    QList<IconRole> m_iconRoles;
    QLabel* m_originalPreview = nullptr;
    QLabel* m_blurredPreview = nullptr;
};

} // namespace ant