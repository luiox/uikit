#pragma once

#include <QWidget>

namespace ant {

class TabContentWidget : public QWidget
{
	Q_OBJECT

public:
	TabContentWidget(const QString& text, QWidget* parent);
	~TabContentWidget();
};

} // namespace ant
