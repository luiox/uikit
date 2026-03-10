#pragma once

#include <QWidget>

namespace ant {

class CommonControlsPage : public QWidget
{
    Q_OBJECT

public:
    explicit CommonControlsPage(QWidget* parent = nullptr);
};

} // namespace ant