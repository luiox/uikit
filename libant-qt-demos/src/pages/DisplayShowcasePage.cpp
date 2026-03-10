#include "DisplayShowcasePage.h"

#include <QDateTime>
#include <QDir>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QUrl>
#include <QVBoxLayout>

#include "AnimatedNumber.h"
#include "AntButton.h"
#include "AntScrollArea.h"
#include "CardWidget.h"
#include "DesignSystem.h"
#include "FlowLayout.h"
#include "GlassWidget.h"
#include "LogoWidget.h"
#include "PaginationWidget.h"
#include "QrCodeWidget.h"
#include "TagWidget.h"

namespace ant {

namespace {

QWidget* createSection(const QString& title, const QString& description, QVBoxLayout*& bodyLayout, QWidget* parent)
{
    auto* section = new QFrame(parent);
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

QImage buildBackgroundImage()
{
    QImage image(QSize(420, 220), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    QLinearGradient gradient(0, 0, image.width(), image.height());
    gradient.setColorAt(0.0, QColor(9, 105, 218));
    gradient.setColorAt(0.5, QColor(56, 189, 248));
    gradient.setColorAt(1.0, QColor(16, 185, 129));
    painter.fillRect(image.rect(), gradient);

    painter.setBrush(QColor(255, 255, 255, 60));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(70, 52), 44, 44);
    painter.drawEllipse(QPointF(340, 180), 70, 70);
    painter.drawRoundedRect(QRectF(76, 58, 228, 90), 18, 18);

    painter.setPen(QColor(255, 255, 255));
    QFont font;
    font.setPointSizeF(16);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(QRectF(98, 86, 200, 24), QStringLiteral("Display Showcase"));

    return image;
}

QString demoImagePath()
{
    return QDir::cleanPath(QDir::current().filePath(QStringLiteral("libant-qt-demo/assets/images/openai.jpg")));
}

} // namespace

DisplayShowcasePage::DisplayShowcasePage(QWidget* parent)
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

    QVBoxLayout* heroBody = nullptr;
    auto* heroSection = createSection(
        QStringLiteral("Display Composition"),
        QStringLiteral("Combine GlassWidget, LogoWidget, CardWidget and AnimatedNumber into a richer display block."),
        heroBody,
        content);

    auto* heroRow = new QHBoxLayout();
    heroRow->setSpacing(18);

    auto* glassWidget = new GlassWidget(10.0f, 3, heroSection);
    glassWidget->setFixedSize(420, 220);
    glassWidget->setBackgroundImage(buildBackgroundImage());
    glassWidget->setPixmapAndText(demoImagePath(), QStringLiteral("Core + Display"), QStringLiteral("Wrap a generated blurred background card and attach an external link."));
    glassWidget->setUrl(QUrl(QStringLiteral("https://github.com")));

    auto* sideColumn = new QVBoxLayout();
    sideColumn->setSpacing(14);
    auto* logoWidget = new LogoWidget(heroSection);
    logoWidget->setFixedSize(132, 132);
    auto* animatedNumber = new AnimatedNumber(heroSection);
    animatedNumber->setFontSzie(18);
    animatedNumber->setTextWidth(180);
    animatedNumber->animateTo(20260310);
    auto* cardWidget = new CardWidget(QStringLiteral("Last visit: %1").arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm"))),
        QStringLiteral("Total demo interactions: 128"), heroSection);
    cardWidget->setFixedSize(320, 180);
    cardWidget->setImageFile(demoImagePath());

    sideColumn->addWidget(logoWidget, 0, Qt::AlignLeft);
    sideColumn->addWidget(animatedNumber, 0, Qt::AlignLeft);
    sideColumn->addWidget(cardWidget);

    heroRow->addWidget(glassWidget);
    heroRow->addLayout(sideColumn, 1);
    heroBody->addLayout(heroRow);

    QVBoxLayout* tagsBody = nullptr;
    auto* tagsSection = createSection(
        QStringLiteral("Tags and Flow Layout"),
        QStringLiteral("Use FlowLayout to arrange TagWidget items as compact filters or category chips."),
        tagsBody,
        content);

    auto* flowHost = new QWidget(tagsSection);
    auto* flowLayout = new FlowLayout(flowHost, 0, 8);
    const QList<QPair<QString, QColor>> tagItems = {
        {QStringLiteral("core"), QColor(22, 119, 255)},
        {QStringLiteral("input"), QColor(16, 185, 129)},
        {QStringLiteral("feedback"), QColor(245, 158, 11)},
        {QStringLiteral("display"), QColor(236, 72, 153)},
        {QStringLiteral("xmake"), QColor(99, 102, 241)},
        {QStringLiteral("demo"), QColor(14, 165, 233)}
    };
    for (const auto& tagItem : tagItems)
    {
        flowLayout->addWidget(new TagWidget(tagItem.first, 11.5, tagItem.second, flowHost, false));
    }
    tagsBody->addWidget(flowHost);

    QVBoxLayout* utilityBody = nullptr;
    auto* utilitySection = createSection(
        QStringLiteral("Utility Widgets"),
        QStringLiteral("QrCodeWidget and PaginationWidget fit naturally in docs, lists and operational pages."),
        utilityBody,
        content);

    auto* utilityRow = new QHBoxLayout();
    utilityRow->setSpacing(18);

    auto* qrCode = new QrCodeWidget(utilitySection);
    qrCode->setFixedSize(220, 220);
    qrCode->setData(QStringLiteral("https://github.com"));

    auto* paginationColumn = new QVBoxLayout();
    paginationColumn->setSpacing(12);
    auto* currentPageLabel = new QLabel(QStringLiteral("Current page: 1 / 12"), utilitySection);
    auto* pagination = new PaginationWidget(QSize(36, 36), utilitySection);
    pagination->setTotalPages(12);
    pagination->setCurrentPage(1);
    auto* jumpButton = new AntButton(QStringLiteral("Jump to page 6"), 12, utilitySection);
    jumpButton->setFixedSize(132, 46);

    paginationColumn->addWidget(currentPageLabel);
    paginationColumn->addWidget(pagination);
    paginationColumn->addWidget(jumpButton, 0, Qt::AlignLeft);
    paginationColumn->addStretch();

    utilityRow->addWidget(qrCode);
    utilityRow->addLayout(paginationColumn, 1);
    utilityBody->addLayout(utilityRow);

    connect(pagination, &PaginationWidget::currentPageChanged, this, [currentPageLabel](int page) {
        currentPageLabel->setText(QStringLiteral("Current page: %1 / 12").arg(page));
    });
    connect(jumpButton, &AntButton::clicked, this, [pagination]() {
        pagination->setCurrentPage(6);
    });

    auto refreshTextStyle = [currentPageLabel]() {
        const Theme theme = DesignSystem::instance()->currentTheme();
        currentPageLabel->setStyleSheet(QStringLiteral("color:%1;").arg(theme.popupTextColor.name()));
    };
    connect(DesignSystem::instance(), &DesignSystem::themeChanged, this, refreshTextStyle);
    refreshTextStyle();

    contentLayout->addWidget(heroSection);
    contentLayout->addWidget(tagsSection);
    contentLayout->addWidget(utilitySection);
    contentLayout->addStretch();
}

} // namespace ant