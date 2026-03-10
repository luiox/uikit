#pragma once

#include <QWidget>

#include "MaterialTabWidget.h"

namespace ant {

class DemoWindow : public QWidget
{
    Q_OBJECT

public:
    explicit DemoWindow(QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    MaterialTabWidget* m_tabs = nullptr;
};

} // namespace ant