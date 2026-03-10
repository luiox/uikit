#include "CoreCapabilitiesPage.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "AntScrollArea.h"
#include "DesignSystem.h"
#include "GaussianBlur.h"
#include "StyleSheet.h"

namespace ant {

namespace {

QWidget* createSection(const QString& title, const QString& description, QVBoxLayout*& bodyLayout, QWidget* parent)
{
    auto* section = new QFrame(parent);
    section->setFrameShape(QFrame::NoFrame);

    auto* layout = new QVBoxLayout(section);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(14);

    auto* titleLabel = new QLabel(title, section);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSizeF(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    auto* descLabel = new QLabel(description, section);
    descLabel->setWordWrap(true);

    auto* bodyWidget = new QWidget(section);
    bodyLayout = new QVBoxLayout(bodyWidget);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(14);

    layout->addWidget(titleLabel);
    layout->addWidget(descLabel);
    layout->addWidget(bodyWidget);

    auto refreshStyle = [section, titleLabel, descLabel]() {
        const Theme theme = DesignSystem::instance()->currentTheme();
        section->setStyleSheet(QStringLiteral("background-color:%1; border:1px solid %2; border-radius:16px;")
            .arg(theme.widgetBgColor.name(), theme.borderColor.name()));
        titleLabel->setStyleSheet(QStringLiteral("color:%1;").arg(theme.popupTextColor.name()));
        descLabel->setStyleSheet(QStringLiteral("color:%1;").arg(theme.listItemDescColor.name()));
    };

    QObject::connect(DesignSystem::instance(), &DesignSystem::themeChanged, section, refreshStyle);
    refreshStyle();
    return section;
}

QString describeColor(const QColor& color)
{
    return QStringLiteral("%1  rgb(%2, %3, %4)")
        .arg(color.name())
        .arg(color.red())
        .arg(color.green())
        .arg(color.blue());
}

QImage buildShowcaseImage()
{
    QImage image(QSize(360, 200), QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(244, 248, 252));

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    QLinearGradient background(0, 0, image.width(), image.height());
    background.setColorAt(0.0, QColor(22, 119, 255));
    background.setColorAt(1.0, QColor(112, 161, 255));
    painter.fillRect(image.rect(), background);

    painter.setBrush(QColor(255, 255, 255, 80));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(86, 68), 48, 48);
    painter.drawEllipse(QPointF(286, 148), 66, 66);

    painter.setBrush(QColor(255, 255, 255, 220));
    painter.drawRoundedRect(QRectF(54, 52, 252, 96), 20, 20);

    painter.setPen(QColor(16, 42, 67));
    QFont titleFont;
    titleFont.setPointSizeF(18);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.drawText(QRectF(76, 70, 220, 26), QStringLiteral("GaussianBlur"));

    QFont bodyFont;
    bodyFont.setPointSizeF(10.5);
    painter.setFont(bodyFont);
    painter.setPen(QColor(72, 101, 129));
    painter.drawText(QRectF(76, 102, 220, 24), QStringLiteral("Core preview generated in code"));

    return image;
}

} // namespace

CoreCapabilitiesPage::CoreCapabilitiesPage(QWidget* parent)
    : QWidget(parent)
{
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    auto* scrollArea = new AntScrollArea(AntScrollArea::ScrollVertical, this);
    auto* content = new QWidget(scrollArea);
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(18);
    scrollArea->addWidget(content);
    outerLayout->addWidget(scrollArea);

    QVBoxLayout* themeBody = nullptr;
    auto* themeSection = createSection(
        QStringLiteral("DesignSystem"),
        QStringLiteral("Preview the active palette and switch between light and dark themes to verify core theme propagation."),
        themeBody,
        content);

    auto* themeHeaderRow = new QHBoxLayout();
    m_themeModeLabel = new QLabel(themeSection);
    auto* themeButton = new QPushButton(QStringLiteral("Toggle Theme"), themeSection);
    themeHeaderRow->addWidget(m_themeModeLabel);
    themeHeaderRow->addStretch();
    themeHeaderRow->addWidget(themeButton);
    themeBody->addLayout(themeHeaderRow);

    auto* colorGrid = new QGridLayout();
    colorGrid->setHorizontalSpacing(18);
    colorGrid->setVerticalSpacing(14);

    const QStringList colorNames = {
        QStringLiteral("Primary"),
        QStringLiteral("Background"),
        QStringLiteral("Border"),
        QStringLiteral("Popup Text")
    };

    for (int index = 0; index < colorNames.size(); ++index)
    {
        auto* cell = new QWidget(themeSection);
        auto* cellLayout = new QHBoxLayout(cell);
        cellLayout->setContentsMargins(0, 0, 0, 0);
        cellLayout->setSpacing(10);

        auto* swatch = new QFrame(cell);
        swatch->setFixedSize(28, 28);
        swatch->setFrameShape(QFrame::NoFrame);
        auto* label = new QLabel(cell);

        cellLayout->addWidget(swatch);
        cellLayout->addWidget(label, 1);

        m_colorSwatches.append(swatch);
        m_colorLabels.append(label);
        colorGrid->addWidget(cell, index / 2, index % 2);
    }
    themeBody->addLayout(colorGrid);

    QVBoxLayout* iconBody = nullptr;
    auto* iconSection = createSection(
        QStringLiteral("IconProvider"),
        QStringLiteral("Resolve icons through libicon-core and libicon-qt instead of depending on scattered raw resource paths."),
        iconBody,
        content);

    auto* iconGrid = new QGridLayout();
    iconGrid->setHorizontalSpacing(14);
    iconGrid->setVerticalSpacing(14);
    m_iconRoles = {
        IconRole::ThemeSun,
        IconRole::WindowClose,
        IconRole::Search,
        IconRole::NavSettings,
        IconRole::NoData,
        IconRole::GoTo
    };
    const QStringList iconNames = {
        QStringLiteral("ThemeSun"),
        QStringLiteral("WindowClose"),
        QStringLiteral("Search"),
        QStringLiteral("NavSettings"),
        QStringLiteral("NoData"),
        QStringLiteral("GoTo")
    };

    for (int index = 0; index < iconNames.size(); ++index)
    {
        auto* tile = new QFrame(iconSection);
        auto* tileLayout = new QVBoxLayout(tile);
        tileLayout->setContentsMargins(12, 12, 12, 12);
        tileLayout->setSpacing(8);

        auto* iconLabel = new QLabel(tile);
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setFixedSize(68, 68);
        auto* nameLabel = new QLabel(iconNames[index], tile);
        nameLabel->setAlignment(Qt::AlignCenter);
        nameLabel->setWordWrap(true);

        tileLayout->addWidget(iconLabel, 0, Qt::AlignCenter);
        tileLayout->addWidget(nameLabel);
        m_iconLabels.append(iconLabel);

        auto refreshTile = [tile, nameLabel]() {
            const Theme theme = DesignSystem::instance()->currentTheme();
            tile->setStyleSheet(QStringLiteral("background-color:%1; border:1px solid %2; border-radius:12px;")
                .arg(theme.backgroundColor.name(), theme.borderColor.name()));
            nameLabel->setStyleSheet(QStringLiteral("color:%1;").arg(theme.popupTextColor.name()));
        };
        connect(DesignSystem::instance(), &DesignSystem::themeChanged, tile, refreshTile);
        refreshTile();

        iconGrid->addWidget(tile, index / 3, index % 3);
    }
    iconBody->addLayout(iconGrid);

    QVBoxLayout* blurBody = nullptr;
    auto* blurSection = createSection(
        QStringLiteral("GaussianBlur"),
        QStringLiteral("Generate a preview image in code and create a blurred version through core/GaussianBlur without external assets."),
        blurBody,
        content);

    auto* blurRow = new QHBoxLayout();
    blurRow->setSpacing(16);
    m_originalPreview = new QLabel(blurSection);
    m_blurredPreview = new QLabel(blurSection);
    m_originalPreview->setScaledContents(true);
    m_blurredPreview->setScaledContents(true);
    m_originalPreview->setFixedSize(320, 180);
    m_blurredPreview->setFixedSize(320, 180);
    blurRow->addWidget(m_originalPreview);
    blurRow->addWidget(m_blurredPreview);
    blurBody->addLayout(blurRow);

    QVBoxLayout* styleBody = nullptr;
    auto* styleSection = createSection(
        QStringLiteral("StyleSheet"),
        QStringLiteral("StyleSheet remains the shared QSS helper entry. This section shows button previews and generated style text."),
        styleBody,
        content);

    auto* previewRow = new QHBoxLayout();
    previewRow->setSpacing(12);
    auto* infoButton = new QPushButton(QStringLiteral("Info Button"), styleSection);
    auto* warningButton = new QPushButton(QStringLiteral("Warning Button"), styleSection);
    auto* logoutButton = new QPushButton(QStringLiteral("Logout Button"), styleSection);
    infoButton->setObjectName(QStringLiteral("infoButton"));
    warningButton->setObjectName(QStringLiteral("warningButton"));
    logoutButton->setObjectName(QStringLiteral("logoutButton"));
    infoButton->setStyleSheet(StyleSheet::infoBtnQss());
    warningButton->setStyleSheet(StyleSheet::warningBtnQss());
    logoutButton->setStyleSheet(StyleSheet::logoutBtnQss());
    previewRow->addWidget(infoButton);
    previewRow->addWidget(warningButton);
    previewRow->addWidget(logoutButton);
    previewRow->addStretch();
    styleBody->addLayout(previewRow);

    auto* qssPreview = new QPlainTextEdit(styleSection);
    qssPreview->setReadOnly(true);
    qssPreview->setMinimumHeight(140);
    styleBody->addWidget(qssPreview);

    auto refreshQssPreview = [qssPreview]() {
        const Theme theme = DesignSystem::instance()->currentTheme();
        qssPreview->setPlainText(
            QStringLiteral("StyleSheet::hTabQss(primary, text)\n\n%1\n\nStyleSheet::gradientQss(start, mid, end)\n\n%2")
                .arg(StyleSheet::hTabQss(theme.primaryColor, theme.tabTextColor))
                .arg(StyleSheet::gradientQss(theme.vipGradientStartColor, theme.vipGradientMidColor, theme.vipGradientEndColor)));
    };

    contentLayout->addWidget(themeSection);
    contentLayout->addWidget(iconSection);
    contentLayout->addWidget(blurSection);
    contentLayout->addWidget(styleSection);
    contentLayout->addStretch();

    connect(themeButton, &QPushButton::clicked, this, []() {
        DesignSystem::instance()->switchTheme();
        emit DesignSystem::instance()->themeChanged();
    });
    connect(DesignSystem::instance(), &DesignSystem::themeChanged, this, &CoreCapabilitiesPage::refreshThemePreview);
    connect(DesignSystem::instance(), &DesignSystem::themeChanged, this, &CoreCapabilitiesPage::refreshIconPreview);
    connect(DesignSystem::instance(), &DesignSystem::themeChanged, this, refreshQssPreview);

    refreshThemePreview();
    refreshIconPreview();
    buildBlurPreview();
    refreshQssPreview();
}

void CoreCapabilitiesPage::refreshThemePreview()
{
    const Theme theme = DesignSystem::instance()->currentTheme();
    const QList<QColor> colors = {
        theme.primaryColor,
        theme.backgroundColor,
        theme.borderColor,
        theme.popupTextColor
    };
    const QStringList names = {
        QStringLiteral("Primary"),
        QStringLiteral("Background"),
        QStringLiteral("Border"),
        QStringLiteral("Popup Text")
    };

    m_themeModeLabel->setText(QStringLiteral("Current theme: %1")
        .arg(DesignSystem::instance()->themeMode() == DesignSystem::Dark ? QStringLiteral("Dark") : QStringLiteral("Light")));
    m_themeModeLabel->setStyleSheet(QStringLiteral("color:%1;").arg(theme.popupTextColor.name()));

    for (int index = 0; index < colors.size(); ++index)
    {
        m_colorSwatches[index]->setStyleSheet(QStringLiteral("background-color:%1; border:1px solid %2; border-radius:6px;")
            .arg(colors[index].name(), theme.borderColor.name()));
        m_colorLabels[index]->setText(QStringLiteral("%1\n%2").arg(names[index], describeColor(colors[index])));
        m_colorLabels[index]->setStyleSheet(QStringLiteral("color:%1;").arg(theme.popupTextColor.name()));
    }
}

void CoreCapabilitiesPage::refreshIconPreview()
{
    const bool isDark = DesignSystem::instance()->themeMode() == DesignSystem::Dark;
    const Theme theme = DesignSystem::instance()->currentTheme();

    for (int index = 0; index < m_iconLabels.size(); ++index)
    {
        m_iconLabels[index]->setPixmap(IconProvider::pixmap(m_iconRoles[index], isDark, QSize(32, 32), theme.primaryColor));
    }
}

void CoreCapabilitiesPage::buildBlurPreview()
{
    const QImage original = buildShowcaseImage();
    GaussianBlur blur(10.0f, 3);
    const QImage blurred = blur.blur(original);

    m_originalPreview->setPixmap(QPixmap::fromImage(original));
    m_blurredPreview->setPixmap(QPixmap::fromImage(blurred));
}

} // namespace ant