#include "DemoWindow.h"

#include <QLabel>
#include <QResizeEvent>
#include <QVBoxLayout>

#include "CommonControlsPage.h"
#include "CoreCapabilitiesPage.h"
#include "DesignSystem.h"
#include "DisplayShowcasePage.h"
#include "FeedbackShowcasePage.h"
#include "MaskWidget.h"
#include "TransparentMask.h"

namespace ant {

DemoWindow::DemoWindow(QWidget* parent)
    : QWidget(parent)
{
    auto* designSystem = DesignSystem::instance();
    designSystem->setThemeMode(DesignSystem::Light);
    designSystem->setMainWindow(this);
    designSystem->setTransparentMask(new TransparentMask(this));
    designSystem->setDarkMask(new MaskWidget(width(), height(), this));

    setObjectName(QStringLiteral("DemoWindow"));
    setWindowTitle(QStringLiteral("libant-qt-demos"));
    resize(1360, 920);
    setMinimumSize(1180, 780);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);

    auto* title = new QLabel(QStringLiteral("libant-qt-demos"), this);
    QFont titleFont = title->font();
    titleFont.setPointSizeF(26);
    titleFont.setBold(true);
    title->setFont(titleFont);

    auto* subtitle = new QLabel(QStringLiteral("Category-based Qt Widgets demos covering core capabilities and richer common control combinations."), this);
    subtitle->setWordWrap(true);

    m_tabs = new MaterialTabWidget(this);
    m_tabs->getLayout()->setContentsMargins(0, 0, 0, 0);
    m_tabs->addTab(new CoreCapabilitiesPage(m_tabs), QStringLiteral("Core"));
    m_tabs->addTab(new CommonControlsPage(m_tabs), QStringLiteral("Common"));
    m_tabs->addTab(new FeedbackShowcasePage(m_tabs), QStringLiteral("Feedback"));
    m_tabs->addTab(new DisplayShowcasePage(m_tabs), QStringLiteral("Display"));

    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addWidget(m_tabs, 1);

    auto refreshChrome = [this, title, subtitle]() {
        const Theme theme = DesignSystem::instance()->currentTheme();
        setStyleSheet(QStringLiteral("#DemoWindow { background-color: %1; }").arg(theme.backgroundColor.name()));
        title->setStyleSheet(QStringLiteral("color: %1;").arg(theme.popupTextColor.name()));
        subtitle->setStyleSheet(QStringLiteral("color: %1;").arg(theme.listItemDescColor.name()));
    };

    connect(designSystem, &DesignSystem::themeChanged, this, refreshChrome);
    refreshChrome();
}

void DemoWindow::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    if (auto* transparentMask = DesignSystem::instance()->getTransparentMask())
    {
        transparentMask->resize(size());
        transparentMask->move(0, 0);
    }

    if (auto* darkMask = DesignSystem::instance()->getDarkMask())
    {
        darkMask->resize(size());
        darkMask->move(0, 0);
    }

    DesignSystem::instance()->setContentSize(size());
}

} // namespace ant