#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once

#include <QMainWindow>
#include <filesystem>
#include <QTableWidget>
#include <CSVParser.h>
#include <Windows.h>
#include <QShortcut>

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget* parent = nullptr);
	~MainWindow();

	void initLogger();

	void closeEvent(QCloseEvent* event);

	bool fillItemList();
	bool fillMemberList();
	bool fillConsumationList();

	void saveCopyOfTablewidgetItems();

public slots:
	void onButtonAddItemClick();
	void onButtonAddMemberClick();
	void onButtonNewCalculationClick();
	void onButtonPrintToPdfClick();
	void onItemTableWidgetItemsChanged(QTableWidgetItem*);
	void onItemTableWidgetMemberChanged(QTableWidgetItem*);
	void onItemTableWidgetConsumeChanged(QTableWidgetItem*);
	void onItemTableWidgetConsumeDoubleClicked(QTableWidgetItem*);
	void onTableWidgetItemsRowInserted();
	void onTableWidgetItemsRowDeleted();
	void onTableWidgetConsumeHorizontalScroll();

	void shortcutCtrlF();
private:
	Ui::MainWindow* ui;

	QShortcut* keyCtrlF;

	std::vector<std::vector<QString*>> tableWidgetItemsCopy;
};

#endif // MAINWINDOW_H
