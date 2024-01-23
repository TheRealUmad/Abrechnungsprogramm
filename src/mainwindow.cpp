#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <fstream>
#include <QTableWidget>
#include <AddItemDialog.h>
#include <AddMemberDialog.h>
#include <SearchMemberDialog.h>
#include <qmessagebox.h>
#include <FileHelper.h>
#include <spdlog/spdlog.h>
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "ConfigHandler.h"
#include "ErrorHandler.h"
#include <TableWidgetHelper.h>
#include <CalculationHelper.h>
#include <StringHelper.h>
#include <PDFHelper.h>
#include <StatisticsHelper.h>
#include <QCloseEvent>
#include <QScrollBar>

MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{

	try
	{
	    ui->setupUi(this);

		ErrorHandler::CreateInstance(this);

		ConfigHandler::CreateInstance();

		if (ConfigHandler::GetInstance()->GetAppConfig()->isDebugComputer)
		{
			::ShowWindow(::GetConsoleWindow(), SW_SHOW);
		}

		initLogger();

		ErrorHandler::CreateInstance(this);

		ui->tableWidget_consume->setAlternatingRowColors(true);
		ui->tableWidget_items->setAlternatingRowColors(true);
		ui->tableWidget_member->setAlternatingRowColors(true);

		if (!fillItemList())
		{
			std::string message = "Could not fill item list!";
			spdlog::error(message);
			throw std::exception(message.c_str());
		}

		saveCopyOfTablewidgetItems();

		if (!fillMemberList())
		{
			std::string message = "Could not fill member list!";
			spdlog::error(message);
			throw std::exception(message.c_str());
		}

		if (!fillConsumationList())
		{
			std::string message = "Could not fill consumation list!";
			spdlog::error(message);
			throw std::exception(message.c_str());
		}

		QObject::connect(ui->btn_addItem, &QPushButton::clicked, this, &MainWindow::onButtonAddItemClick);
		QObject::connect(ui->btn_addMember, &QPushButton::clicked, this, &MainWindow::onButtonAddMemberClick);
		QObject::connect(ui->btn_newCalculation, &QPushButton::clicked, this, &MainWindow::onButtonNewCalculationClick);
		QObject::connect(ui->btn_saveAsPdf, &QPushButton::clicked, this, &MainWindow::onButtonPrintToPdfClick);
		QObject::connect(ui->tableWidget_items, &QTableWidget::itemChanged, this, &MainWindow::onItemTableWidgetItemsChanged);
		QObject::connect(ui->tableWidget_member, &QTableWidget::itemChanged, this, &MainWindow::onItemTableWidgetMemberChanged);
		QObject::connect(ui->tableWidget_consume, &QTableWidget::itemChanged, this, &MainWindow::onItemTableWidgetConsumeChanged);
		QObject::connect(ui->tableWidget_consume, &QTableWidget::itemDoubleClicked, this, &MainWindow::onItemTableWidgetConsumeDoubleClicked);
		QObject::connect(ui->tableWidget_items->model(), &QAbstractTableModel::rowsInserted, this, &MainWindow::onTableWidgetItemsRowInserted);
		QObject::connect(ui->tableWidget_items->model(), &QAbstractTableModel::rowsRemoved, this, &MainWindow::onTableWidgetItemsRowDeleted);
		QObject::connect(ui->tableWidget_consume->horizontalScrollBar(), &QScrollBar::valueChanged, this, &MainWindow::onTableWidgetConsumeHorizontalScroll);

		//shortcuts
		keyCtrlF = new QShortcut(this);
		keyCtrlF->setKey(Qt::CTRL + Qt::Key_F);

		connect(keyCtrlF, SIGNAL(activated()), this, SLOT(shortcutCtrlF()));
	}
	catch (const std::exception& ex)
	{
		std::string message = "Error during inizialisation! Exception: " + (std::string)ex.what();
		spdlog::critical(message);
		ErrorHandler::GetInstance()->ShowErrorMessageDialog(message);
	}

}

MainWindow::~MainWindow()
{
	try
	{
		delete ui;
	}
	catch (std::exception ex)
	{
		spdlog::error("Could not delete ui! Exception: " + (std::string)ex.what());
	}
}

void MainWindow::initLogger()
{
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

	if (ConfigHandler::GetInstance()->GetAppConfig()->isDebugComputer)
		console_sink->set_level(spdlog::level::debug);
	else
		console_sink->set_level(spdlog::level::warn);

	auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(ConfigHandler::GetInstance()->GetAppConfig()->logFileDir + "/logfile", 23, 59);
	file_sink->set_level(spdlog::level::debug);

	spdlog::sinks_init_list sink_list = { file_sink, console_sink };

	spdlog::logger logger("logger", sink_list.begin(), sink_list.end());
	logger.set_level(spdlog::level::debug);
	logger.flush_on(spdlog::level::err);

	spdlog::set_default_logger(std::make_shared<spdlog::logger>(logger));

	spdlog::info("Logger started");
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	spdlog::info("Main window closing.");
	spdlog::default_logger()->flush();
	event->accept();
}

void MainWindow::onButtonPrintToPdfClick()
{
	spdlog::info("Button print to PDF clicked.");

	QTableWidget* consumeTable = ui->tableWidget_consume;

	FileHelper::checkFile(ConfigHandler::GetInstance()->GetAppConfig()->pdfFilePath);
	PDFHelper::printCalculationToPdf(consumeTable, ConfigHandler::GetInstance()->GetAppConfig()->pdfFilePath);

	FileHelper::checkFile(ConfigHandler::GetInstance()->GetAppConfig()->statisticsPath);
	FileHelper::checkFile(ConfigHandler::GetInstance()->GetAppConfig()->statisticsDebugPath);
	StatisticsHelper::gatherStatistics(consumeTable, ui->tableWidget_items);
}

void MainWindow::onButtonNewCalculationClick()
{
	spdlog::info("Button new calculation clicked.");

	std::string message = "Alle Konsumationen und Einzahlungen werden auf 0 gesetzt. Aktuelle Guthaben bzw. Schulden werden in die Spalte Uebertrag gesetzt. \n Willst du wirklich eine neue Abrechnung beginnen?";

	auto reply = QMessageBox::question(this, QString::fromStdString("Warnung"), QString::fromStdString(message), QMessageBox::Ok | QMessageBox::Cancel);

	if (reply == QMessageBox::Ok)
	{
		spdlog::info("User requested start of new calculation.");

		QTableWidget* consumeTable = ui->tableWidget_consume;
		QTableWidget* itemsTable = ui->tableWidget_items;

		int columnCarryover;
		int columnDeposits;
		int columnTurnover;
		int columnDebt;
		int columnCredit;
		int columnItemsStart;
		int columnItemsEnd;

		if (!TableWidgetHelper::findRelevantColumnIndexes(consumeTable, itemsTable, columnCarryover, columnDeposits, columnTurnover, columnDebt, columnCredit, columnItemsStart, columnItemsEnd))
		{
			return;
		}

		spdlog::info("Trying to create a save file.");

		FileHelper::deleteFile(ConfigHandler::GetInstance()->GetAppConfig()->saveFilePath); //delete old save file
		FileHelper::copyFile(ConfigHandler::GetInstance()->GetAppConfig()->consumationCSVPath, ConfigHandler::GetInstance()->GetAppConfig()->saveFilePath); //create save file

		spdlog::info("Trying to prepare consume table for new calculation");

		consumeTable->blockSignals(true);

		TableWidgetHelper::copyColumnOfTable(consumeTable, columnDebt, columnCarryover); //copy debt and credit values into carryover
		TableWidgetHelper::copyColumnOfTable(consumeTable, columnCredit, columnCarryover);

		for (int column = columnItemsStart; column <= columnItemsEnd; column++)
		{
			TableWidgetHelper::clearColumnOfTable(consumeTable, column); //delete all item values
		}

		TableWidgetHelper::clearColumnOfTable(consumeTable, columnTurnover); //delete turnover values

		TableWidgetHelper::clearColumnOfTable(consumeTable, columnDeposits); //delete deposit values

		TableWidgetHelper::clearColumnOfTable(consumeTable, columnDebt); //delete credit and debt values
		TableWidgetHelper::clearColumnOfTable(consumeTable, columnCredit);

		CalculationHelper::calculateAndUpdateConsumeTable(consumeTable, itemsTable, columnCarryover, columnDeposits, columnTurnover, columnDebt, columnCredit, columnItemsStart, columnItemsEnd);
		consumeTable->blockSignals(false);

		TableWidgetHelper::writeTableWidgetToCSVfile(ConfigHandler::GetInstance()->GetAppConfig()->consumationCSVPath, consumeTable);

		spdlog::info("Ready for new calculation.");
	}
	else
	{
		spdlog::info("User aborted start of new calculation.");
	}
}

void MainWindow::onButtonAddItemClick()
{
	spdlog::info("Button add item clicked.");

	std::vector<std::string> newRow;
	std::string itemName;

	QStringList list = AddItemDialog::getStrings(this);
	if (!list.isEmpty())
	{
		itemName = list.first().toStdString();
		if (!itemName.empty())
		{
			spdlog::info("New item name: " + itemName);
			newRow.push_back(itemName);
		}
		else
		{
			spdlog::warn("Item name must not be empty!");
			return;
		}
		std::string itemValue = list.last().toStdString();
		if (!itemValue.empty())
		{
			spdlog::info("New item value: " + itemValue);

			if (!StringHelper::checkDoubleDigitString(itemValue))
			{
				spdlog::warn("Item value was not correct!");
				return;
			}
			else
			{
				newRow.push_back(itemValue);
			}
		}
		else
		{
			spdlog::warn("Item name must not be empty!");
			return;
		}

	}
	else
	{
		spdlog::warn("Item name and value must not be empty!");
		return;
	}

	ui->tableWidget_items->blockSignals(true);
	ui->tableWidget_consume->blockSignals(true);

	TableWidgetHelper::addRowToTableWidget(ui->tableWidget_items, newRow);
	TableWidgetHelper::addItemToConsumeTableHeader(ui->tableWidget_consume,itemName);
	TableWidgetHelper::writeTableWidgetToCSVfile(ConfigHandler::GetInstance()->GetAppConfig()->itemCSVPath, ui->tableWidget_items);
	TableWidgetHelper::writeTableWidgetToCSVfile(ConfigHandler::GetInstance()->GetAppConfig()->consumationCSVPath, ui->tableWidget_consume);

	ui->tableWidget_consume->blockSignals(false);
	ui->tableWidget_items->blockSignals(false);

}

void MainWindow::onButtonAddMemberClick()
{
	spdlog::info("Button add member clicked.");

	std::vector<std::string> newRow;

	QStringList list = AddMemberDialog::getStrings(this);

	if (!list.isEmpty())
	{
		std::string memberAlias = list.first().toStdString();
		if (!memberAlias.empty())
		{
			spdlog::info("New member alias: " + memberAlias);
			newRow.push_back(memberAlias);
		}
		else
		{
			spdlog::warn("Member alias must not be empty!");
			ui->statusbar->showMessage("Coleurname darf nicht leer sein!");
			return;
		}

		std::string memberFirstName = list.at(1).toStdString();
		std::string memberLastName = list.last().toStdString();
		if (!memberFirstName.empty() && !memberLastName.empty())
		{
			spdlog::info("New member name: " + memberFirstName + " " + memberLastName);
			newRow.push_back(memberFirstName);
			newRow.push_back(memberLastName);
		}
		else
		{
			spdlog::warn("Name must not be empty!");
			ui->statusbar->showMessage("Name darf nicht leer sein!");
			return;
		}
	}
	else
	{
		spdlog::warn("Name and/or alias must not be empty!");
		ui->statusbar->showMessage("Es wurde kein Coleurname und/oder Name eingegeben!");
		return;
	}

	TableWidgetHelper::addRowToTableWidget(ui->tableWidget_member, newRow);
}

void MainWindow::onItemTableWidgetItemsChanged(QTableWidgetItem* changedItem)
{
	QObject* obj = sender();
	QTableWidget* tableWidget = qobject_cast<QTableWidget*>(obj);

	spdlog::info("Item: " + changedItem->text().toStdString() + " changed in table widget: " + tableWidget->objectName().toStdString());

	bool rowIsEmpty = true;
	bool rowHasEmptyItem = false;
	for (int column = 0; column < tableWidget->columnCount(); column++)
	{
		QTableWidgetItem* item = tableWidget->item(changedItem->row(), column);
		if (item == nullptr || item->text().isEmpty())
		{
			rowIsEmpty = true;
			rowHasEmptyItem = true;
		}
		else
		{
			rowIsEmpty = false;
			break;
		}
	}

	if (!rowHasEmptyItem)
	{
		saveCopyOfTablewidgetItems();
	}

	if (rowIsEmpty)
	{
		if (TableWidgetHelper::deleteEmptyColumnOfTableWidget(this, ui->tableWidget_consume))//remove header from consume
		{
			ConfigHandler::GetInstance()->GetAppConfig()->itemsInConsumeTableIndex--; //so next new item will be inserted correctly

			spdlog::info("Removing empty row with index: " + std::to_string(tableWidget->currentRow()) + " from table widget: " + tableWidget->objectName().toStdString());
			tableWidget->removeRow(changedItem->row());

			int columnCarryover;
			int columnDeposits;
			int columnTurnover;
			int columnDebt;
			int columnCredit;
			int columnItemsStart;
			int columnItemsEnd;

			TableWidgetHelper::findRelevantColumnIndexes(ui->tableWidget_consume, ui->tableWidget_items, columnCarryover, columnDeposits, columnTurnover, columnDebt, columnCredit, columnItemsStart, columnItemsEnd);

			ui->tableWidget_consume->blockSignals(true);
			CalculationHelper::calculateAndUpdateConsumeTable(ui->tableWidget_consume, ui->tableWidget_items, columnCarryover, columnDeposits, columnTurnover, columnDebt, columnCredit, columnItemsStart, columnItemsEnd);
			ui->tableWidget_consume->blockSignals(false);
		}
		else
		{
			spdlog::info("Restoring row with index: " + std::to_string(changedItem->row()) + " from table widget: " + tableWidget->objectName().toStdString());
			std::vector<QString*> rowToRestore = tableWidgetItemsCopy.at(changedItem->row());

			tableWidget->blockSignals(true);
			for (int column = 0; column < tableWidget->columnCount(); column++)
			{
				if (column == changedItem->column())
				{
					changedItem->text() = QString::fromStdString(rowToRestore.at(column)->toStdString());
				}
				TableWidgetHelper::addItemToTableWidget(tableWidget, rowToRestore.at(column)->toStdString(), changedItem->row(), column);
			}
			tableWidget->blockSignals(false);
		}

		TableWidgetHelper::writeTableWidgetToCSVfile(ConfigHandler::GetInstance()->GetAppConfig()->consumationCSVPath, ui->tableWidget_consume);
	}

	TableWidgetHelper::generateConsumeTableHeader(ui->tableWidget_consume, ui->tableWidget_member, ui->tableWidget_items);

	TableWidgetHelper::writeTableWidgetToCSVfile(ConfigHandler::GetInstance()->GetAppConfig()->itemCSVPath, ui->tableWidget_items);
}

void MainWindow::onItemTableWidgetMemberChanged(QTableWidgetItem* changedItem)
{
	QObject* obj = sender();
	QTableWidget* tableWidget = qobject_cast<QTableWidget*>(obj);

	spdlog::info("Item: " + changedItem->text().toStdString() + " changed in table widget: " + tableWidget->objectName().toStdString());

	QTableWidgetItem* memberAliasItem = tableWidget->item(changedItem->row(), 0);
	QTableWidgetItem* memberFirstNameItem = tableWidget->item(changedItem->row(), 1);
	QTableWidgetItem* memberLastNameItem = tableWidget->item(changedItem->row(), 2);

	QString memberAlias = QString::fromStdString("");
	QString memberFirstName = QString::fromStdString("");
	QString memberLastName = QString::fromStdString("");

	if (memberAliasItem != nullptr)
	{
		memberAlias = tableWidget->item(changedItem->row(), 0)->text();
	}
	if (memberFirstNameItem != nullptr)
	{
		memberFirstName = tableWidget->item(changedItem->row(), 1)->text();
	}
	if (memberLastNameItem != nullptr)
	{
		memberLastName = tableWidget->item(changedItem->row(), 2)->text();
	}

	if (memberAlias.isEmpty() && memberFirstName.isEmpty() && memberLastName.isEmpty())
	{
		spdlog::info("Member will be deleted, because name and alias is empty.");

		TableWidgetHelper::deleteEmptyMemberOfTableWidget(this, tableWidget, ui->tableWidget_consume, ui->tableWidget_member);
	}

	TableWidgetHelper::updateMemberInMemberAndConsumeTable(changedItem, ui->tableWidget_consume, ui->tableWidget_member);
	TableWidgetHelper::updateConsumeTableVerticalHeader(ui->tableWidget_consume, ui->tableWidget_consume->horizontalScrollBar()->value());
	TableWidgetHelper::writeTableWidgetToCSVfile(ConfigHandler::GetInstance()->GetAppConfig()->memberCSVPath, ui->tableWidget_member);
	TableWidgetHelper::writeTableWidgetToCSVfile(ConfigHandler::GetInstance()->GetAppConfig()->consumationCSVPath, ui->tableWidget_consume);
}

void MainWindow::onItemTableWidgetConsumeChanged(QTableWidgetItem* changedItem)
{
	QObject* obj = sender();
	QTableWidget* tableWidget = qobject_cast<QTableWidget*>(obj);

	spdlog::info("Item: " + changedItem->text().toStdString() + " changed in table widget: " + tableWidget->objectName().toStdString());

	if (changedItem->column() < 3) //first 3 columns are member data
	{
		spdlog::info("Item changed in member data.");

		QTableWidgetItem* memberAliasItem = tableWidget->item(changedItem->row(), 0);
		QTableWidgetItem* memberFirstNameItem = tableWidget->item(changedItem->row(), 1);
		QTableWidgetItem* memberLastNameItem = tableWidget->item(changedItem->row(), 2);

		QString memberAlias = QString::fromStdString("");
		QString memberFirstName = QString::fromStdString("");
		QString memberLastName = QString::fromStdString("");

		if (memberAliasItem != nullptr)
		{
			memberAlias = tableWidget->item(changedItem->row(), 0)->text();
		}
		if (memberFirstNameItem != nullptr)
		{
			memberFirstName = tableWidget->item(changedItem->row(), 1)->text();
		}
		if (memberLastNameItem != nullptr)
		{
			memberLastName = tableWidget->item(changedItem->row(), 2)->text();
		}

		if (memberAlias.isEmpty() && memberFirstName.isEmpty() && memberLastName.isEmpty())
		{
			spdlog::info("Member will be deleted, because name and alias is empty.");

			TableWidgetHelper::deleteEmptyMemberOfTableWidget(this, tableWidget, ui->tableWidget_consume, ui->tableWidget_member);
		}
		TableWidgetHelper::updateMemberInMemberAndConsumeTable(changedItem, ui->tableWidget_consume, ui->tableWidget_member);
		TableWidgetHelper::updateConsumeTableVerticalHeader(ui->tableWidget_consume, ui->tableWidget_consume->horizontalScrollBar()->value());
	}
	else
	{
		spdlog::info("Item changed in calculation data.");

		int columnCarryover;
		int columnDeposits;
		int columnTurnover;
		int columnDebt;
		int columnCredit;

		int columnItemsStart;
		int columnItemsEnd;

		TableWidgetHelper::findRelevantColumnIndexes(tableWidget, ui->tableWidget_items, columnCarryover, columnDeposits, columnTurnover, columnDebt, columnCredit, columnItemsStart, columnItemsEnd);

		if (changedItem->column() == columnCarryover)
		{
			tableWidget->blockSignals(true);
			StringHelper::checkDoubleDigitItem(changedItem);
			tableWidget->blockSignals(false);
		}
		else if (changedItem->column() == columnDeposits)
		{
			tableWidget->blockSignals(true);
			StringHelper::checkDoubleDigitItem(changedItem);
			CalculationHelper::calculateAndUpdateConsumeTable(tableWidget, ui->tableWidget_items, columnCarryover, columnDeposits, columnTurnover, columnDebt, columnCredit, columnItemsStart, columnItemsEnd);
			tableWidget->blockSignals(false);
		}
		else if (changedItem->column() == columnTurnover)
		{
			tableWidget->blockSignals(true);
			StringHelper::checkDoubleDigitItem(changedItem);
			tableWidget->blockSignals(false);
		}
		else if (changedItem->column() == columnDebt)
		{
			tableWidget->blockSignals(true);
			StringHelper::checkDoubleDigitItem(changedItem);
			tableWidget->blockSignals(false);
		}
		else if (changedItem->column() == columnCredit)
		{
			tableWidget->blockSignals(true);
			StringHelper::checkDoubleDigitItem(changedItem);
			tableWidget->blockSignals(false);
		}
		else if (changedItem->column() >= columnItemsStart && changedItem->column() <= columnItemsEnd)
		{
			tableWidget->blockSignals(true);
			StringHelper::checkIntDigitItem(changedItem);
			CalculationHelper::calculateAndUpdateConsumeTable(tableWidget, ui->tableWidget_items, columnCarryover, columnDeposits, columnTurnover, columnDebt, columnCredit, columnItemsStart, columnItemsEnd);
			tableWidget->blockSignals(false);
		}
	}

	TableWidgetHelper::writeTableWidgetToCSVfile(ConfigHandler::GetInstance()->GetAppConfig()->memberCSVPath, ui->tableWidget_member);
	TableWidgetHelper::writeTableWidgetToCSVfile(ConfigHandler::GetInstance()->GetAppConfig()->consumationCSVPath, ui->tableWidget_consume);
}

void MainWindow::onItemTableWidgetConsumeDoubleClicked(QTableWidgetItem* item)
{
	QObject* obj = sender();
	QTableWidget* tableWidget = qobject_cast<QTableWidget*>(obj);

	spdlog::info("Item: " + item->text().toStdString() + " double clicked.");

	int columnCarryover = TableWidgetHelper::findColumnInTableHeader(tableWidget, "Uebertrag");
	int columnTurnover = TableWidgetHelper::findColumnInTableHeader(tableWidget, "Umsatz");
	int columnDebt = TableWidgetHelper::findColumnInTableHeader(tableWidget, "Schulden");
	int columnCredit = TableWidgetHelper::findColumnInTableHeader(tableWidget, "Guthaben");

	if (item->column() == columnCarryover || item->column() == columnDebt || item->column() == columnCredit || item->column() == columnTurnover)
	{
		spdlog::warn("Item can not be edited");
		item->setFlags(item->flags() & ~Qt::ItemIsEditable);
	}
}

void MainWindow::onTableWidgetItemsRowInserted()
{

}

void MainWindow::onTableWidgetItemsRowDeleted()
{

}

void MainWindow::onTableWidgetConsumeHorizontalScroll()
{	
	QObject* obj = sender();
	QScrollBar* scrollBar = qobject_cast<QScrollBar*>(obj);

	QTableWidget* tableWidget = qobject_cast<QTableWidget*>(scrollBar->parent()->parent());

	spdlog::info("Horizontal scroll in table widget: " + tableWidget->objectName().toStdString()+". Scroll value: "+std::to_string(scrollBar->value()));

	TableWidgetHelper::updateConsumeTableVerticalHeader(tableWidget, scrollBar->value());

	spdlog::info("Horizontal scroll finished");
}

void MainWindow::shortcutCtrlF()
{
	spdlog::info("Shortcut STRG + F pressed.");

	QStringList list = SearchMemberDialog::getStrings(this);

	QTableWidgetItem* alias;

	if (TableWidgetHelper::findMemberByAliasInConsumeTable(ui->tableWidget_consume, list[0], alias))
	{
		spdlog::info("Trying to focus row of member: " + alias->text().toStdString());

		int columnToFocus = -1;
		columnToFocus = ui->tableWidget_consume->currentColumn();
		if (columnToFocus == -1)
		{
			columnToFocus == alias->column();
		}

		ui->tableWidget_consume->scrollToItem(alias);
		ui->tableWidget_consume->setCurrentCell(alias->row(), columnToFocus);

		spdlog::info("Set focus to cell in row: " + std::to_string(alias->row()) + " and column: " + std::to_string(columnToFocus));
	}
	else
	{
		spdlog::info("Could not find member in consume table.");
	}

	spdlog::info("Finished STRG + F");
}

bool MainWindow::fillItemList()
{
	spdlog::info("Trying to fill item list");

	ui->tableWidget_items->setColumnCount(2);

	QTableWidgetItem* headerItem = new QTableWidgetItem("Artikel");
	headerItem->setTextAlignment(Qt::AlignCenter);
	ui->tableWidget_items->setHorizontalHeaderItem(0, headerItem);

	headerItem = new QTableWidgetItem("Preis (Euro)");
	headerItem->setTextAlignment(Qt::AlignCenter);
	ui->tableWidget_items->setHorizontalHeaderItem(1, headerItem);

	TableWidgetHelper::readCSVAndAddToTableWidget(ConfigHandler::GetInstance()->GetAppConfig()->itemCSVPath, ui->tableWidget_items);

	spdlog::info("Successfully filled item list");

	return true;
}

bool MainWindow::fillMemberList()
{
	spdlog::info("Trying to fill member list");

	ui->tableWidget_member->setColumnCount(3);

	QTableWidgetItem* headerItem = new QTableWidgetItem("Coleurname");
	headerItem->setTextAlignment(Qt::AlignCenter);
	ui->tableWidget_member->setHorizontalHeaderItem(0, headerItem);

	headerItem = new QTableWidgetItem("Vorname");
	headerItem->setTextAlignment(Qt::AlignCenter);
	ui->tableWidget_member->setHorizontalHeaderItem(1, headerItem);

	headerItem = new QTableWidgetItem("Nachname");
	headerItem->setTextAlignment(Qt::AlignCenter);
	ui->tableWidget_member->setHorizontalHeaderItem(2, headerItem);

	TableWidgetHelper::readCSVAndAddToTableWidget(ConfigHandler::GetInstance()->GetAppConfig()->memberCSVPath, ui->tableWidget_member);

	spdlog::info("Successfully filled member list");

	return true;
}

bool MainWindow::fillConsumationList()
{
	if (!TableWidgetHelper::generateConsumeTableHeader(ui->tableWidget_consume, ui->tableWidget_member, ui->tableWidget_items))
	{
		return false;
	}

	std::vector<CSVRow> rows;

	FileHelper::readCSVFile(ConfigHandler::GetInstance()->GetAppConfig()->consumationCSVPath, rows);

	if (!rows.empty())
	{
		for (CSVRow row : rows)
		{
			TableWidgetHelper::addRowToTableWidget(ui->tableWidget_consume, row);
		}
	}
	else
	{
		spdlog::info("Consumation csv file: " + ConfigHandler::GetInstance()->GetAppConfig()->consumationCSVPath + " is empty. Trying to fill table widget: " + ui->tableWidget_consume->objectName().toStdString() + " with members.");

		ui->tableWidget_consume->setRowCount(ui->tableWidget_member->rowCount()); //equalize row count
		for (int row = 0; row < ui->tableWidget_member->rowCount(); row++)
		{
			for (int column = 0; column < ui->tableWidget_member->columnCount(); column++)
			{
				TableWidgetHelper::addItemToTableWidget(ui->tableWidget_consume, ui->tableWidget_member->item(row, column)->text(), row, column);
			}
		}
	}

	spdlog::info("Successfully filled consumation list");

	return true;
}

void MainWindow::saveCopyOfTablewidgetItems()
{
	QTableWidget* tableWidget = ui->tableWidget_items;

	for (int row = 0; row < tableWidget->rowCount(); row++)
	{
		std::vector<QString*> rowList;
		for (int column = 0; column < tableWidget->columnCount(); column++)
		{
			rowList.push_back(new QString(tableWidget->item(row, column)->text()));
		}
		tableWidgetItemsCopy.push_back(rowList);
	}
}

