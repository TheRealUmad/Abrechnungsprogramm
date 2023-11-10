#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <fstream>
#include <QTableWidget>
#include <AddItemDialog.h>
#include <AddMemberDialog.h>
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

MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{

	try
	{
	    ui->setupUi(this);

		ErrorHandler::CreateInstance(this);

		ConfigHandler::CreateInstance();
		initLogger();

		ErrorHandler::CreateInstance(this);

		if (!fillItemList())
		{
			std::string message = "Could not fill item list!";
			spdlog::error(message);
			throw std::exception(message.c_str());
		}

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
	console_sink->set_level(spdlog::level::warn);

	auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(ConfigHandler::GetInstance()->GetAppConfig()->logFileDir + "/logfile", 23, 59);
	file_sink->set_level(spdlog::level::debug);

	spdlog::sinks_init_list sink_list = { file_sink, console_sink };

	spdlog::logger logger("logger", sink_list.begin(), sink_list.end());
	logger.set_level(spdlog::level::debug);

	spdlog::set_default_logger(std::make_shared<spdlog::logger>(logger));

	spdlog::info("Logger started");
}

void MainWindow::onButtonPrintToPdfClick()
{
	spdlog::info("Button print to PDF clicked.");

	QTableWidget* consumeTable = ui->tableWidget_consume;

	PDFHelper::printCalculationToPdf(consumeTable, ConfigHandler::GetInstance()->GetAppConfig()->pdfFilePath);

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

		//consumeTable->update();
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

	/*ui->tableWidget_member->blockSignals(true);
	ui->tableWidget_member->blockSignals(true);*/
	TableWidgetHelper::addRowToTableWidget(ui->tableWidget_member, newRow);
	//addRowToTableWidget(ui->tableWidget_consume, newRow);
	/*ui->tableWidget_member->blockSignals(false);
	ui->tableWidget_member->blockSignals(false);*/
}

void MainWindow::onItemTableWidgetItemsChanged(QTableWidgetItem* changedItem)
{
	QObject* obj = sender();
	QTableWidget* tableWidget = qobject_cast<QTableWidget*>(obj);

	spdlog::info("Item: " + changedItem->text().toStdString() + " changed in table widget: " + tableWidget->objectName().toStdString());

	bool rowIsEmpty = true;
	for (int column = 0; column < tableWidget->columnCount(); column++)
	{
		QTableWidgetItem* item = tableWidget->item(changedItem->row(), column);
		if (item == nullptr || item->text().isEmpty())
		{
			rowIsEmpty = true;
		}
		else
		{
			rowIsEmpty = false;
			break;
		}
	}

	TableWidgetHelper::generateConsumeTableHeader(ui->tableWidget_consume, ui->tableWidget_member, ui->tableWidget_items);

	if (rowIsEmpty)
	{
		TableWidgetHelper::deleteEmptyColumnOfTableWidget(ui->tableWidget_consume); //remove header from consume
		TableWidgetHelper::writeTableWidgetToCSVfile(ConfigHandler::GetInstance()->GetAppConfig()->consumationCSVPath, ui->tableWidget_consume);
		ConfigHandler::GetInstance()->GetAppConfig()->itemsInConsumeTableIndex--; //so next new item will be inserted correctly

		spdlog::info("Removing empty row with index: " + std::to_string(tableWidget->currentRow()) + " from table widget: " + tableWidget->objectName().toStdString());
		tableWidget->removeRow(changedItem->row());
	}

	//deleteEmptyRowsOfTableWidget(tableWidget);
	TableWidgetHelper::writeTableWidgetToCSVfile(ConfigHandler::GetInstance()->GetAppConfig()->itemCSVPath, tableWidget);
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

	/*QTableWidgetItem* memberAliasInConsumeTable;
	QTableWidgetItem* memberFirstNameInConsumeTable;
	QTableWidgetItem* memberLastNameInConsumeTable;*/

	/*if (!findMemberByNameAndAlias(tableWidget, memberFirstName, memberLastName, memberAlias, memberFirstNameInConsumeTable, memberLastNameInConsumeTable, memberAliasInConsumeTable))
	{
		std::cout << "Member: " << memberFirstName.toStdString() << " " << memberLastName.toStdString() << " v/o " << memberAlias.toStdString() << " was not found in table widget: " << tableWidget->objectName().toStdString() << std::endl;*/

	if (memberAlias.isEmpty() && memberFirstName.isEmpty() && memberLastName.isEmpty())
	{
		spdlog::info("Member will be deleted, because name and alias is empty.");

		deleteEmptyMemberFromTable(tableWidget);
	}
	//}

	//deleteEmptyRowsOfTableWidget(tableWidget);
	updateMemberInMemberAndConsumeTable(changedItem);
	TableWidgetHelper::writeTableWidgetToCSVfile(ConfigHandler::GetInstance()->GetAppConfig()->memberCSVPath, tableWidget);
	TableWidgetHelper::writeTableWidgetToCSVfile(ConfigHandler::GetInstance()->GetAppConfig()->consumationCSVPath, tableWidget);
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

			deleteEmptyMemberFromTable(tableWidget);
		}
		updateMemberInMemberAndConsumeTable(changedItem);
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

	//deleteEmptyRowsOfTableWidget(tableWidget);
	TableWidgetHelper::writeTableWidgetToCSVfile(ConfigHandler::GetInstance()->GetAppConfig()->memberCSVPath, tableWidget);
	TableWidgetHelper::writeTableWidgetToCSVfile(ConfigHandler::GetInstance()->GetAppConfig()->consumationCSVPath, tableWidget);
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

bool MainWindow::deleteEmptyMemberFromTable(QTableWidget* tableWidget)
{
	spdlog::info("Trying to delete empty members from table widget: " + tableWidget->objectName().toStdString());

	for (int row = 0; row < tableWidget->rowCount(); row++)
	{
		QTableWidgetItem* memberAlias = tableWidget->item(row, 0);
		QTableWidgetItem* memberFirstName = tableWidget->item(row, 1);
		QTableWidgetItem* memberLastName = tableWidget->item(row, 2);

		if (memberAlias->text().isEmpty() && memberFirstName->text().isEmpty() && memberLastName->text().isEmpty())
		{
			spdlog::info("Found a row with empty member data in table: " + tableWidget->objectName().toStdString());

			double debt = 0.0;
			double credit = 0.0;
			if (memberHasDebtOrCredit(row, debt, credit))
			{
				std::string message = "Member still has " + std::to_string(debt) + " Euro debt and " + std::to_string(credit) + " Euro credit.";
				spdlog::warn(message);

				message = "Mitglied hat noch " + std::to_string(debt) + " Euro Schulden und " + std::to_string(credit) + " Euro Guthaben. \n Willst du das Mitglied wirklich loeschen?";

				auto reply = QMessageBox::question(this, QString::fromStdString("Warnung"), QString::fromStdString(message), QMessageBox::Ok | QMessageBox::Cancel);
				if (reply == QMessageBox::Ok)
				{
					spdlog::info("User decided that row with empty member should be deleted.");
					deleteRowFromMemberAndConsumeTable(row);
					spdlog::info("Successfully deleted empty member from table widget: " + tableWidget->objectName().toStdString());
					return true;
				}
				else
				{
					spdlog::info("User decided row with empty member should not be deleted.");
					tableWidget->blockSignals(true);
					restoreRowFromMemberOrConsumeTable(tableWidget, row);
					tableWidget->blockSignals(false);
					return false;
				}
			}
			else
			{
				spdlog::info("Member can be deleted because he has no credit or debt.");
				deleteRowFromMemberAndConsumeTable(row);
				spdlog::info("Successfully deleted empty member from table widget: " + tableWidget->objectName().toStdString());
			}
		}
	}
}

bool MainWindow::restoreRowFromMemberOrConsumeTable(QTableWidget* tableWidgetToRestore, int rowToRestore)
{
	spdlog::info("Trying to restore row: " + std::to_string(rowToRestore) + " of table widget: " + tableWidgetToRestore->objectName().toStdString());

	if (tableWidgetToRestore->objectName() == ui->tableWidget_consume->objectName())
	{
		for (int i = 0; i < 3; i++)
		{
			if (!TableWidgetHelper::addItemToTableWidget(tableWidgetToRestore, ui->tableWidget_member->item(rowToRestore, i), rowToRestore, i, true))
			{
				return false;
			}
		}
	}
	else if (tableWidgetToRestore->objectName() == ui->tableWidget_member->objectName())
	{
		for (int i = 0; i < 3; i++)
		{
			if (!TableWidgetHelper::addItemToTableWidget(tableWidgetToRestore, ui->tableWidget_consume->item(rowToRestore, i), rowToRestore, i, true))
			{
				return false;
			}
		}
	}
	else
	{
		spdlog::warn("Unknown table widget: " + tableWidgetToRestore->objectName().toStdString());
		return false;
	}

	spdlog::info("Successfully restored row: " + std::to_string(rowToRestore) + " of table widget: " + tableWidgetToRestore->objectName().toStdString());
	return true;
}

bool MainWindow::deleteRowFromMemberAndConsumeTable(int row)
{
	spdlog::info("Trying to remove row: " + std::to_string(row) + " from consume table and member table");

	QTableWidget* consumeTable = ui->tableWidget_consume;
	QTableWidget* memberTable = ui->tableWidget_member;

	consumeTable->blockSignals(true);
	memberTable->blockSignals(true);

	consumeTable->removeRow(row);
	memberTable->removeRow(row);

	consumeTable->blockSignals(false);
	memberTable->blockSignals(false);

	spdlog::info("Successfully removed row: " + std::to_string(row) + " from consume and member table");
	return true;
}

bool MainWindow::updateMemberInMemberAndConsumeTable(QTableWidgetItem* changedItem)
{
	spdlog::info("Trying to update member in member and consume table");

	if (changedItem != nullptr && changedItem->tableWidget() == ui->tableWidget_consume)
	{
		if (!changedItem->text().isEmpty())
		{
			QTableWidgetItem* itemInMemberTable = ui->tableWidget_member->item(changedItem->row(), changedItem->column());
			if (itemInMemberTable != nullptr && changedItem->text() == itemInMemberTable->text())
			{
				spdlog::info("Not inserting item from consume table into member table because the text is the same");
				return true;
			}

			spdlog::info("Trying to insert item from consume table into member table.");
			//ui->tableWidget_member->blockSignals(true);
			TableWidgetHelper::addItemToTableWidget(ui->tableWidget_member, changedItem->text(), changedItem->row(), changedItem->column());
			//ui->tableWidget_member->blockSignals(false);
		}
		else
		{
			spdlog::info("Not inserting item from consume table into member table because the text is empty");
		}
	}
	else if (changedItem != nullptr && changedItem->tableWidget() == ui->tableWidget_member)
	{
		if (!changedItem->text().isEmpty())
		{
			QTableWidgetItem* itemInConsumeTable = ui->tableWidget_consume->item(changedItem->row(), changedItem->column());
			if (itemInConsumeTable != nullptr && changedItem->text() == itemInConsumeTable->text())
			{
				spdlog::info("Not inserting item from member table into consume table because the text is the same");
				return true;
			}

			spdlog::info("Trying to insert item from member table into consume table.");
			//ui->tableWidget_consume->blockSignals(true);
			TableWidgetHelper::addItemToTableWidget(ui->tableWidget_consume, changedItem->text(), changedItem->row(), changedItem->column());
			//ui->tableWidget_consume->blockSignals(true);
		}
		else
		{
			spdlog::info("Not inserting item from member table into consume table because the text is empty");
		}
	}
	else
	{
		spdlog::warn("Changed item was null or table widget was not recognized");
		return false;
	}

	return true;
}

bool MainWindow::memberHasDebtOrCredit(int rowInConsumeTable, double& outDebt, double& outCredit)
{
	spdlog::info("Trying to find out if member has debt or credit");

	QTableWidget* tableWidget = ui->tableWidget_consume;

	int columnDebt = TableWidgetHelper::findColumnInTableHeader(tableWidget, QString::fromStdString("Schulden"));
	int columnCredit = TableWidgetHelper::findColumnInTableHeader(tableWidget, QString::fromStdString("Guthaben"));

	if (columnDebt != -1 && columnCredit != 1)
	{
		QTableWidgetItem* debtItem = tableWidget->item(rowInConsumeTable, columnDebt);
		QTableWidgetItem* creditItem = tableWidget->item(rowInConsumeTable, columnCredit);

		if (debtItem != nullptr)
		{
			outDebt = debtItem->text().toDouble();
		}
		else
		{
			outDebt = 0.0;
		}

		if (creditItem != nullptr)
		{
			outCredit = creditItem->text().toDouble();
		}
		else
		{
			outCredit = 0.0;
		}

		spdlog::info("Member has " + std::to_string(outDebt) + " debt and " + std::to_string(outCredit) + " credit.");
		if (outDebt == 0.0 && outCredit == 0.0)
		{
			spdlog::info("Member has no debt or credit.");
			return false;
		}
		else
		{
			spdlog::info("Member has debt or credit.");
			return true;
		}
	}
	else
	{
		std::string message = "Could not find column for debt or credit in table widget: " + tableWidget->objectName().toStdString();
		spdlog::error(message);
		throw std::exception(message.c_str());
	}

	return true;

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
	/*std::vector<CSVRow> rows;

	if (!readCSVFile(memberCSVPath, rows))
	{
		std::cerr << "Error reading csv file!" << std::endl;
		return false;
	}

	for (CSVRow row : rows)
	{
		if (!addRowToTableWidget(ui->tableWidget_member, row))
		{
			std::cerr << "Error adding row to table!" << std::endl;
			return false;
		}
	}*/

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

