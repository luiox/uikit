#pragma once

#include <QWidget>

class HomePage : public QWidget
{
	Q_OBJECT

public:
	HomePage(QWidget* parent);
	~HomePage();
protected:
	void showEvent(QShowEvent* event) override;
};
