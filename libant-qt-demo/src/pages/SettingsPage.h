#pragma once

#include <QWidget>

namespace ant {

class SettingsPage  : public QWidget
{
	Q_OBJECT

public:
	SettingsPage(QWidget *parent);
	~SettingsPage();
};

} // namespace ant

