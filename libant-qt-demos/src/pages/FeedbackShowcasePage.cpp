#include "FeedbackShowcasePage.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "AntButton.h"
#include "AntMessage.h"
#include "AntMessageManager.h"
#include "AntScrollArea.h"
#include "BadgeWidget.h"
#include "DesignSystem.h"
#include "MaterialProgressBar.h"
#include "MaterialSpinner.h"
#include "NoDataWidget.h"
#include "SkeletonWidget.h"

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

} // namespace

FeedbackShowcasePage::FeedbackShowcasePage(QWidget* parent)
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

    QVBoxLayout* progressBody = nullptr;
    auto* progressSection = createSection(
        QStringLiteral("Progress and Waiting"),
        QStringLiteral("Show determinate progress, indeterminate progress and a spinner in the same area."),
        progressBody,
        content);

    auto* progressRow = new QHBoxLayout();
    progressRow->setSpacing(18);
    auto* progressBar = new MaterialProgressBar(progressSection);
    progressBar->setFixedWidth(320);
    progressBar->setFixedHeight(12);
    progressBar->setRealProgress(72);
    auto* indeterminateBar = new MaterialProgressBar(progressSection);
    indeterminateBar->setFixedWidth(320);
    indeterminateBar->setFixedHeight(12);
    indeterminateBar->startIndeterminate();
    auto* spinner = new MaterialSpinner(QSize(40, 40), 4, DesignSystem::instance()->primaryColor(), progressSection);
    progressRow->addWidget(progressBar);
    progressRow->addWidget(indeterminateBar);
    progressRow->addWidget(spinner);
    progressRow->addStretch();
    progressBody->addLayout(progressRow);

    QVBoxLayout* messageBody = nullptr;
    auto* messageSection = createSection(
        QStringLiteral("Global Messages"),
        QStringLiteral("Trigger AntMessageManager states to verify stacking and exit animation behavior."),
        messageBody,
        content);

    auto* messageRow = new QHBoxLayout();
    messageRow->setSpacing(12);
    auto* infoButton = new AntButton(QStringLiteral("Info"), 12, messageSection);
    auto* successButton = new AntButton(QStringLiteral("Success"), 12, messageSection);
    auto* errorButton = new AntButton(QStringLiteral("Error"), 12, messageSection);
    auto* warningButton = new AntButton(QStringLiteral("Warning"), 12, messageSection);
    for (AntButton* button : {infoButton, successButton, errorButton, warningButton})
    {
        button->setFixedSize(120, 48);
        messageRow->addWidget(button);
    }
    messageRow->addStretch();
    messageBody->addLayout(messageRow);

    connect(infoButton, &AntButton::clicked, this, []() {
        AntMessageManager::instance()->showMessage(AntMessage::Info, QStringLiteral("This is an info message"));
    });
    connect(successButton, &AntButton::clicked, this, []() {
        AntMessageManager::instance()->showMessage(AntMessage::Success, QStringLiteral("The action completed successfully"));
    });
    connect(errorButton, &AntButton::clicked, this, []() {
        AntMessageManager::instance()->showMessage(AntMessage::Error, QStringLiteral("This is an error-state message"));
    });
    connect(warningButton, &AntButton::clicked, this, []() {
        AntMessageManager::instance()->showMessage(AntMessage::Warning, QStringLiteral("Please continue checking the configuration"));
    });

    QVBoxLayout* statusBody = nullptr;
    auto* statusSection = createSection(
        QStringLiteral("Status Components"),
        QStringLiteral("Cover badge, skeleton and empty-state widgets for loading and business-state placeholders."),
        statusBody,
        content);

    auto* badgeRow = new QHBoxLayout();
    badgeRow->setSpacing(16);
    const QList<QPair<QString, BadgeWidget::BadgeStatus>> badgeItems = {
        {QStringLiteral("Success"), BadgeWidget::Success},
        {QStringLiteral("Error"), BadgeWidget::Error},
        {QStringLiteral("Running"), BadgeWidget::Running},
        {QStringLiteral("Warning"), BadgeWidget::Warning},
        {QStringLiteral("Default"), BadgeWidget::Default}
    };
    for (const auto& badgeItem : badgeItems)
    {
        auto* wrapper = new QWidget(statusSection);
        auto* wrapperLayout = new QVBoxLayout(wrapper);
        wrapperLayout->setContentsMargins(0, 0, 0, 0);
        wrapperLayout->setSpacing(8);
        auto* badge = new BadgeWidget(12, 12, wrapper);
        badge->setBadge(true, badgeItem.second);
        auto* label = new QLabel(badgeItem.first, wrapper);
        label->setAlignment(Qt::AlignCenter);
        wrapperLayout->addWidget(badge, 0, Qt::AlignCenter);
        wrapperLayout->addWidget(label);
        badgeRow->addWidget(wrapper);
    }
    badgeRow->addStretch();
    statusBody->addLayout(badgeRow);

    auto* skeletonHost = new QFrame(statusSection);
    skeletonHost->setFixedSize(320, 132);
    auto* realContent = new QLabel(QStringLiteral("Real content is now available. Replace this with cards, list items or a metric summary."), skeletonHost);
    realContent->setWordWrap(true);
    realContent->setGeometry(20, 20, 280, 92);
    realContent->hide();
    auto* skeleton = new SkeletonWidget(10, skeletonHost);
    skeleton->setGeometry(0, 0, 320, 132);
    skeleton->startSkeleton();
    auto* skeletonButton = new AntButton(QStringLiteral("Finish loading"), 12, statusSection);
    skeletonButton->setFixedSize(132, 46);
    auto* skeletonRow = new QHBoxLayout();
    skeletonRow->setSpacing(14);
    skeletonRow->addWidget(skeletonHost);
    skeletonRow->addWidget(skeletonButton, 0, Qt::AlignTop);
    skeletonRow->addStretch();
    statusBody->addLayout(skeletonRow);

    auto* noDataWidget = new NoDataWidget(statusSection);
    noDataWidget->setText(QStringLiteral("No records match the current filter"));
    noDataWidget->setFixedSize(320, 220);
    statusBody->addWidget(noDataWidget);

    connect(skeletonButton, &AntButton::clicked, this, [skeleton, realContent]() {
        skeleton->stopSkeleton();
        realContent->show();
    });

    auto refreshTextStyle = [badgeRow, realContent]() {
        const Theme theme = DesignSystem::instance()->currentTheme();
        const auto labels = badgeRow->parentWidget()->findChildren<QLabel*>();
        for (QLabel* label : labels)
        {
            label->setStyleSheet(QStringLiteral("color:%1;").arg(theme.popupTextColor.name()));
        }
        realContent->setStyleSheet(QStringLiteral("color:%1;").arg(theme.popupTextColor.name()));
    };
    connect(DesignSystem::instance(), &DesignSystem::themeChanged, this, refreshTextStyle);
    refreshTextStyle();

    contentLayout->addWidget(progressSection);
    contentLayout->addWidget(messageSection);
    contentLayout->addWidget(statusSection);
    contentLayout->addStretch();
}

} // namespace ant