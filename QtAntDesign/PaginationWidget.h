#pragma once

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include "IconProvider.h"

namespace ant {

class PaginationWidget : public QWidget
{
	Q_OBJECT
public:
	PaginationWidget(QSize buttonSize, QWidget* parent = nullptr);
	void setTotalPages(int pages);
	void setCurrentPage(int page);
	int currentPage() const { return m_currentPage; }

signals:
	void currentPageChanged(int page);
private:
	void updateBtnIcon(QPushButton* btn, IconRole normalRole, IconRole disabledRole);
private:
	void refreshButtons();
	QPushButton* createNavButton(IconRole normalRole, IconRole disabledRole);

	QSize btnSize;
	int m_totalPages;
	int m_currentPage;
	QHBoxLayout* m_layout;
	QPushButton* m_prevButton;
	QPushButton* m_nextButton;
};

} // namespace ant

using ant::PaginationWidget;
