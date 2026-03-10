#include "CommonControlsPage.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "AntButton.h"
#include "AntComboBox.h"
#include "AntDoubleNumberInput.h"
#include "AntInput.h"
#include "AntMessageManager.h"
#include "AntNumberInput.h"
#include "AntRadioButton.h"
#include "AntScrollArea.h"
#include "AntSlider.h"
#include "AntToggleButton.h"
#include "DesignSystem.h"
#include "MaterialLineEdit.h"
#include "Win11CheckButton.h"

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

CommonControlsPage::CommonControlsPage(QWidget* parent)
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

    QVBoxLayout* actionBody = nullptr;
    auto* actionSection = createSection(
        QStringLiteral("Actions and State"),
        QStringLiteral("A compact area for buttons, toggles, radio options and checkbox state."),
        actionBody,
        content);

    auto* actionRow = new QHBoxLayout();
    actionRow->setSpacing(14);
    auto* primaryButton = new AntButton(QStringLiteral("Submit"), 12, actionSection);
    primaryButton->setFixedSize(128, 48);
    auto* backButton = new AntButton(QString(), 12, actionSection);
    backButton->setIconRole(IconRole::Back);
    backButton->setFixedSize(50, 48);
    auto* toggle = new AntToggleButton(QSize(64, 28), actionSection);
    toggle->setShowText(true);
    auto* stateLabel = new QLabel(QStringLiteral("Current state: idle"), actionSection);
    actionRow->addWidget(primaryButton);
    actionRow->addWidget(backButton);
    actionRow->addWidget(toggle);
    actionRow->addWidget(stateLabel);
    actionRow->addStretch();
    actionBody->addLayout(actionRow);

    auto* choiceRow = new QHBoxLayout();
    choiceRow->setSpacing(18);
    auto* radioA = new AntRadioButton(actionSection);
    auto* radioB = new AntRadioButton(actionSection);
    auto* check = new Win11CheckButton(actionSection);
    radioA->setText(QStringLiteral("Strategy A"));
    radioB->setText(QStringLiteral("Strategy B"));
    check->setText(QStringLiteral("Enable extra validation"));
    radioA->setChecked(true);
    choiceRow->addWidget(radioA);
    choiceRow->addWidget(radioB);
    choiceRow->addWidget(check);
    choiceRow->addStretch();
    actionBody->addLayout(choiceRow);

    QVBoxLayout* fieldBody = nullptr;
    auto* fieldSection = createSection(
        QStringLiteral("Input and Filtering"),
        QStringLiteral("Cover text input, search, single-level combos and multi-level combos in one form area."),
        fieldBody,
        content);

    auto* fieldGrid = new QGridLayout();
    fieldGrid->setHorizontalSpacing(18);
    fieldGrid->setVerticalSpacing(14);

    auto* emailInput = new MaterialLineEdit(fieldSection);
    emailInput->setLabelText(QStringLiteral("Email"));
    emailInput->setFixedHeight(44);
    auto* passwordInput = new MaterialLineEdit(fieldSection);
    passwordInput->setLabelText(QStringLiteral("Password"));
    passwordInput->setPasswordToggleEnabled(true);
    passwordInput->setFixedHeight(44);

    auto* searchInput = new AntInput(220, {
        QStringLiteral("OpenAI"),
        QStringLiteral("NVIDIA"),
        QStringLiteral("Qt Widgets"),
        QStringLiteral("DuiLib"),
        QStringLiteral("Launcher")
    }, fieldSection);
    searchInput->setPlaceholderText(QStringLiteral("Search stacks or demo items"));
    searchInput->setFixedSize(260, 46);

    auto* simpleCombo = new AntComboBox(
        QStringLiteral("Select environment"),
        {QStringLiteral("Dev"), QStringLiteral("QA"), QStringLiteral("Staging"), QStringLiteral("Prod")},
        fieldSection);
    simpleCombo->setFixedSize(220, 46);

    QMap<QString, QStringList> multiLevelItems = {
        {QStringLiteral("Input Controls"), {QStringLiteral("AntInput"), QStringLiteral("MaterialLineEdit"), QStringLiteral("AntComboBox")}},
        {QStringLiteral("Feedback Controls"), {QStringLiteral("MaterialProgressBar"), QStringLiteral("AntMessage"), QStringLiteral("NoDataWidget")}},
        {QStringLiteral("Display Controls"), {QStringLiteral("AnimatedNumber"), QStringLiteral("QrCodeWidget"), QStringLiteral("GlassWidget")}}
    };
    auto* multiCombo = new AntComboBox(
        QStringLiteral("Browse by category"),
        {QStringLiteral("Input Controls"), QStringLiteral("Feedback Controls"), QStringLiteral("Display Controls")},
        fieldSection,
        220,
        true,
        multiLevelItems);
    multiCombo->setFixedSize(220, 46);

    fieldGrid->addWidget(new QLabel(QStringLiteral("Basic text input"), fieldSection), 0, 0);
    fieldGrid->addWidget(emailInput, 1, 0);
    fieldGrid->addWidget(new QLabel(QStringLiteral("Password input"), fieldSection), 0, 1);
    fieldGrid->addWidget(passwordInput, 1, 1);
    fieldGrid->addWidget(new QLabel(QStringLiteral("Search popup"), fieldSection), 2, 0);
    fieldGrid->addWidget(searchInput, 3, 0);
    fieldGrid->addWidget(new QLabel(QStringLiteral("Single-level combo"), fieldSection), 2, 1);
    fieldGrid->addWidget(simpleCombo, 3, 1);
    fieldGrid->addWidget(new QLabel(QStringLiteral("Multi-level combo"), fieldSection), 4, 0);
    fieldGrid->addWidget(multiCombo, 5, 0);
    fieldBody->addLayout(fieldGrid);

    QVBoxLayout* quantitativeBody = nullptr;
    auto* quantitativeSection = createSection(
        QStringLiteral("Numeric and Range"),
        QStringLiteral("Integer input, decimal input and a slider form a small parameter tuning panel."),
        quantitativeBody,
        content);

    auto* quantitativeRow = new QHBoxLayout();
    quantitativeRow->setSpacing(16);
    auto* numberInput = new AntNumberInput(quantitativeSection);
    numberInput->setFixedSize(96, 46);
    numberInput->input()->setRange(0, 100);
    numberInput->input()->setValue(18);

    auto* doubleInput = new AntDoubleNumberInput(quantitativeSection);
    doubleInput->setFixedSize(110, 46);
    doubleInput->input()->setRange(0.0, 100.0);
    doubleInput->input()->setDecimals(1);
    doubleInput->input()->setValue(42.5);

    auto* slider = new AntSlider(0, 100, 64, quantitativeSection);
    slider->setFixedWidth(320);
    auto* sliderLabel = new QLabel(QStringLiteral("Threshold: 64"), quantitativeSection);

    quantitativeRow->addWidget(new QLabel(QStringLiteral("Integer"), quantitativeSection));
    quantitativeRow->addWidget(numberInput);
    quantitativeRow->addWidget(new QLabel(QStringLiteral("Decimal"), quantitativeSection));
    quantitativeRow->addWidget(doubleInput);
    quantitativeRow->addWidget(new QLabel(QStringLiteral("Slider"), quantitativeSection));
    quantitativeRow->addWidget(slider);
    quantitativeRow->addWidget(sliderLabel);
    quantitativeRow->addStretch();
    quantitativeBody->addLayout(quantitativeRow);

    QVBoxLayout* composedBody = nullptr;
    auto* composedSection = createSection(
        QStringLiteral("Composed Example"),
        QStringLiteral("Combine several controls into a small Create Task panel that can later expand into a real form."),
        composedBody,
        content);

    auto* composedPanel = new QFrame(composedSection);
    auto* composedLayout = new QGridLayout(composedPanel);
    composedLayout->setHorizontalSpacing(16);
    composedLayout->setVerticalSpacing(12);

    auto* taskTitle = new MaterialLineEdit(composedPanel);
    taskTitle->setLabelText(QStringLiteral("Task title"));
    taskTitle->setFixedHeight(44);
    auto* ownerSearch = new AntInput(180, {QStringLiteral("Canrad"), QStringLiteral("OpenAI"), QStringLiteral("NAssistant")}, composedPanel);
    ownerSearch->setPlaceholderText(QStringLiteral("Search owner"));
    ownerSearch->setFixedSize(240, 46);
    auto* executionModeCombo = new AntComboBox(
        QStringLiteral("Select mode"),
        {QStringLiteral("Serial"), QStringLiteral("Parallel"), QStringLiteral("Manual Review")},
        composedPanel);
    executionModeCombo->setFixedSize(220, 46);
    auto* createButton = new AntButton(QStringLiteral("Create task"), 12, composedPanel);
    createButton->setFixedSize(132, 46);
    auto* createSummary = new QLabel(QStringLiteral("Summary: not submitted yet"), composedPanel);
    createSummary->setWordWrap(true);

    composedLayout->addWidget(new QLabel(QStringLiteral("Title"), composedPanel), 0, 0);
    composedLayout->addWidget(taskTitle, 1, 0, 1, 2);
    composedLayout->addWidget(new QLabel(QStringLiteral("Owner"), composedPanel), 2, 0);
    composedLayout->addWidget(ownerSearch, 3, 0);
    composedLayout->addWidget(new QLabel(QStringLiteral("Execution mode"), composedPanel), 2, 1);
    composedLayout->addWidget(executionModeCombo, 3, 1);
    composedLayout->addWidget(createButton, 4, 0);
    composedLayout->addWidget(createSummary, 4, 1);
    composedBody->addWidget(composedPanel);

    auto refreshText = [stateLabel, toggle]() {
        stateLabel->setText(QStringLiteral("Current state: %1").arg(toggle->isChecked() ? QStringLiteral("enabled") : QStringLiteral("idle")));
    };

    connect(toggle, &AntToggleButton::toggled, this, [refreshText](bool) {
        refreshText();
    });
    connect(primaryButton, &AntButton::clicked, this, []() {
        AntMessageManager::instance()->showMessage(AntMessage::Success, QStringLiteral("Submit action demo triggered"));
    });
    connect(backButton, &AntButton::clicked, this, []() {
        AntMessageManager::instance()->showMessage(AntMessage::Info, QStringLiteral("Icon button responded"));
    });
    connect(slider, &QSlider::valueChanged, this, [sliderLabel](int value) {
        sliderLabel->setText(QStringLiteral("Threshold: %1").arg(value));
    });
    connect(createButton, &AntButton::clicked, this, [taskTitle, slider, createSummary]() {
        createSummary->setText(QStringLiteral("Summary: created task \"%1\" with threshold %2")
            .arg(taskTitle->text().isEmpty() ? QStringLiteral("Untitled Task") : taskTitle->text())
            .arg(slider->value()));
        AntMessageManager::instance()->showMessage(AntMessage::Success, QStringLiteral("Composed form submitted demo data"));
    });

    auto refreshTextStyle = [stateLabel, sliderLabel, createSummary]() {
        const Theme theme = DesignSystem::instance()->currentTheme();
        stateLabel->setStyleSheet(QStringLiteral("color:%1;").arg(theme.popupTextColor.name()));
        sliderLabel->setStyleSheet(QStringLiteral("color:%1;").arg(theme.popupTextColor.name()));
        createSummary->setStyleSheet(QStringLiteral("color:%1;").arg(theme.popupTextColor.name()));
    };
    connect(DesignSystem::instance(), &DesignSystem::themeChanged, this, refreshTextStyle);
    refreshText();
    refreshTextStyle();

    contentLayout->addWidget(actionSection);
    contentLayout->addWidget(fieldSection);
    contentLayout->addWidget(quantitativeSection);
    contentLayout->addWidget(composedSection);
    contentLayout->addStretch();
}

} // namespace ant