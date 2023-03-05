#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <fstream>
//#include <CSVParser.h>
#include <QTableWidget>
#include <AddItemDialog.h>
#include <AddMemberDialog.h>

MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	if (!fillItemList())
	{
		std::cerr << "Error! Table widget items could not be filled correctly!" << std::endl;
	}

	if (!fillMemberList())
	{
		std::cerr << "Error! Table widget member could not be filled correctly!" << std::endl;
	}

	if (!fillConsumationList())
	{
		std::cerr << "Error! Table widget consumation could not be filled correctly!" << std::endl;
	}

	QObject::connect(ui->btn_addItem, &QPushButton::clicked, this, &MainWindow::onButtonAddItemClick);
	QObject::connect(ui->btn_addMember, &QPushButton::clicked, this, &MainWindow::onButtonAddMemberClick);
	QObject::connect(ui->tableWidget_items, &QTableWidget::itemChanged, this, &MainWindow::onItemTableWidgetItemsChanged);
	QObject::connect(ui->tableWidget_member, &QTableWidget::itemChanged, this, &MainWindow::onItemTableWidgetMemberChanged);
	QObject::connect(ui->tableWidget_consume, &QTableWidget::itemChanged, this, &MainWindow::onItemTableWidgetConsumeChanged);
	QObject::connect(ui->tableWidget_items->model(), &QAbstractTableModel::rowsInserted, this, &MainWindow::onTableWidgetItemsRowInserted);
	QObject::connect(ui->tableWidget_items->model(), &QAbstractTableModel::rowsRemoved, this, &MainWindow::onTableWidgetItemsRowDeleted);

}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::onButtonAddItemClick()
{
	std::cout << "Button add item clicked" << std::endl;

	std::vector<std::string> newRow;

	QStringList list = AddItemDialog::getStrings(this);
	if (!list.isEmpty())
	{
		std::string itemName = list.first().toStdString();
		if (!itemName.empty())
		{
			std::cout << "New item name: " << itemName << std::endl;
			newRow.push_back(itemName);
		}
		else
		{
			ui->statusbar->showMessage("Artikelname darf nicht leer sein!");
			return;
		}
		std::string itemValue = list.last().toStdString();
		if (!itemValue.empty())
		{
			std::cout << "New item value: " << itemValue << std::endl;

			std::replace(itemValue.begin(), itemValue.end(), ',', '.'); //replace all , with .

			if (itemValue.find('.') == std::string::npos) //see if . was found
			{
				if (isDigit(itemValue))
				{
					newRow.push_back(itemValue);
				}
				else
				{
					ui->statusbar->showMessage("Artikelpreis ist keine Nummer!");
					return;
				}
			}
			else
			{
				std::string::difference_type n = std::count(itemValue.begin(), itemValue.end(), '.'); //count number of .
				if (n == 1)
				{
					std::string firstDigits = itemValue.substr(0, itemValue.find('.'));
					std::string lastDigits = itemValue.substr(itemValue.find('.') + 1, itemValue.length());

					std::cout << "first digits: " << firstDigits << " last digits: " << lastDigits << std::endl;

					if (isDigit(firstDigits) && isDigit(lastDigits))
					{
						newRow.push_back(itemValue);
					}
					else
					{
						ui->statusbar->showMessage("Artikelpreis ist keine Nummer!");
						return;
					}
				}
				else
				{
					ui->statusbar->showMessage("Artikelpreis darf nur ein Komma ',' oder einen Punkt '.' enthalten!");
					return;
				}
			}
		}
		else
		{
			ui->statusbar->showMessage("Artikelpreis darf nicht leer sein!");
			return;
		}

	}
	else
	{
		ui->statusbar->showMessage("Es wurde kein Preis und/oder Artikelname eingegeben!");
		return;
	}

	addRowToTableWidget(ui->tableWidget_items, newRow);

}

void MainWindow::onButtonAddMemberClick()
{
	std::cout << "Button add member clicked" << std::endl;

	std::vector<std::string> newRow;

	QStringList list = AddMemberDialog::getStrings(this);

	if (!list.isEmpty())
	{
		std::string memberAlias = list.first().toStdString();
		if (!memberAlias.empty())
		{
			std::cout << "New member alias: " << memberAlias << std::endl;
			newRow.push_back(memberAlias);
		}
		else
		{
			ui->statusbar->showMessage("Coleurname darf nicht leer sein!");
			return;
		}

		std::string memberName = list.last().toStdString();
		if (!memberName.empty())
		{
			std::cout << "New member name: " << memberName << std::endl;
			newRow.push_back(memberName);
		}
		else
		{
			ui->statusbar->showMessage("Name darf nicht leer sein!");
			return;
		}
	}
	else
	{
		ui->statusbar->showMessage("Es wurde kein Coleurname und/oder Name eingegeben!");
		return;
	}

	addRowToTableWidget(ui->tableWidget_member, newRow);
}

void MainWindow::onItemTableWidgetItemsChanged()
{
	QObject* obj = sender();
	QTableWidget* tableWidget = qobject_cast<QTableWidget*>(obj);

	std::cout << "Item changed in table " << tableWidget->objectName().toStdString() << std::endl;

	deleteEmptyRowsOfTableWidget(tableWidget);
	writeTableWidgetToCSVfile(itemCSVPath.string(), tableWidget);
}

void MainWindow::onItemTableWidgetMemberChanged()
{
	QObject* obj = sender();
	QTableWidget* tableWidget = qobject_cast<QTableWidget*>(obj);

	std::cout << "Item changed in table " << tableWidget->objectName().toStdString() << std::endl;

	deleteEmptyRowsOfTableWidget(tableWidget);
	writeTableWidgetToCSVfile(memberCSVPath.string(), tableWidget);
}

void MainWindow::onItemTableWidgetConsumeChanged()
{
	QObject* obj = sender();
	QTableWidget* tableWidget = qobject_cast<QTableWidget*>(obj);

	std::cout << "Item changed in table " << tableWidget->objectName().toStdString() << std::endl;

	deleteEmptyRowsOfTableWidget(tableWidget);
	writeTableWidgetToCSVfile(consumationCSVPath.string(), tableWidget);
}

void MainWindow::onTableWidgetItemsRowInserted()
{
	
}

void MainWindow::onTableWidgetItemsRowDeleted()
{

}

bool MainWindow::deleteEmptyRowsOfTableWidget(QTableWidget* tableWidget)
{
	try
	{
		for (int row = 0; row < tableWidget->rowCount(); row++)
		{
			bool isEmpty = false;
			for (int column = 0; column < tableWidget->columnCount(); column++)
			{
				QTableWidgetItem* item = tableWidget->item(row, column);
				if (item == nullptr)
				{
					isEmpty = true;
				}
				else
				{
					std::string data = item->text().toStdString();
					if (data.empty())
					{
						isEmpty = true;
					}
					else
					{
						isEmpty = false;
						break;
					}
				}
			}
			if (isEmpty)
			{
				std::cout << "Removing empty row with index: " << row << " from table: " << tableWidget->objectName().toStdString() << std::endl;
				tableWidget->removeRow(row);
			}
		}
	}
	catch (std::exception ex)
	{
		std::cerr << "Error during removing empty rows from table! Exception: " << ex.what() << std::endl;
		return false;
	}

	return true;
}

bool MainWindow::isDigit(std::string str)
{
	if (std::all_of(str.begin(), str.end(), ::isdigit))
	{
		return true;
	}
	return false;
}

bool MainWindow::fillItemList()
{
	std::cout << "Trying to fill item list" << std::endl;

	ui->tableWidget_items->setColumnCount(2);

	QTableWidgetItem* headerItem = new QTableWidgetItem("Artikel");
	headerItem->setTextAlignment(Qt::AlignCenter);
	ui->tableWidget_items->setHorizontalHeaderItem(0, headerItem);

	headerItem = new QTableWidgetItem("Preis (Euro)");
	headerItem->setTextAlignment(Qt::AlignCenter);
	ui->tableWidget_items->setHorizontalHeaderItem(1, headerItem);

	std::vector<CSVRow> rows;
	if (!readCSVFile(itemCSVPath.string(), rows))
	{
		std::cerr << "Error reading csv file!" << std::endl;
		return false;
	}

	for (CSVRow row : rows)
	{
		if (!addRowToTableWidget(ui->tableWidget_items, row))
		{
			std::cerr << "Error adding row to table!" << std::endl;
			return false;
		}
	}

	return true;
}

bool MainWindow::fillMemberList()
{
	std::cout << "Trying to fill member list" << std::endl;

	ui->tableWidget_member->setColumnCount(2);

	QTableWidgetItem* headerItem = new QTableWidgetItem("Coleurname");
	headerItem->setTextAlignment(Qt::AlignCenter);
	ui->tableWidget_member->setHorizontalHeaderItem(0, headerItem);

	headerItem = new QTableWidgetItem("Name");
	headerItem->setTextAlignment(Qt::AlignCenter);
	ui->tableWidget_member->setHorizontalHeaderItem(1, headerItem);

	std::vector<CSVRow> rows;

	if (!readCSVFile(memberCSVPath.string(), rows))
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
	}

	return true;
}

bool MainWindow::fillConsumationList()
{
	try
	{
		int nrOfItems = ui->tableWidget_items->rowCount();

		ui->tableWidget_consume->setColumnCount(nrOfItems + 7); //columncount needs to be set first

		int columnCount = 0;

		QTableWidgetItem* headerItem = new QTableWidgetItem("Coleurname");
		headerItem->setTextAlignment(Qt::AlignCenter);
		ui->tableWidget_consume->setHorizontalHeaderItem(columnCount, headerItem);

		columnCount++;

		headerItem = new QTableWidgetItem("Name");
		headerItem->setTextAlignment(Qt::AlignCenter);
		ui->tableWidget_consume->setHorizontalHeaderItem(columnCount, headerItem);

		columnCount++;

		headerItem = new QTableWidgetItem("Uebertrag");
		headerItem->setTextAlignment(Qt::AlignCenter);
		ui->tableWidget_consume->setHorizontalHeaderItem(columnCount, headerItem);

		columnCount++;

		for (int row = 0; row < ui->tableWidget_items->rowCount(); row++)
		{
			QTableWidgetItem* item = ui->tableWidget_items->item(row, 0);
			//std::string itemName = item->text().toStdString();

			ui->tableWidget_consume->setHorizontalHeaderItem(columnCount, item);
			columnCount++;
		}

		headerItem = new QTableWidgetItem("Umsatz");
		headerItem->setTextAlignment(Qt::AlignCenter);
		ui->tableWidget_consume->setHorizontalHeaderItem(columnCount, headerItem);

		columnCount++;

		headerItem = new QTableWidgetItem("Einzahlungen");
		headerItem->setTextAlignment(Qt::AlignCenter);
		ui->tableWidget_consume->setHorizontalHeaderItem(columnCount, headerItem);

		columnCount++;

		headerItem = new QTableWidgetItem("Schulden");
		headerItem->setTextAlignment(Qt::AlignCenter);
		ui->tableWidget_consume->setHorizontalHeaderItem(columnCount, headerItem);

		columnCount++;

		headerItem = new QTableWidgetItem("Forderungen");
		headerItem->setTextAlignment(Qt::AlignCenter);
		ui->tableWidget_consume->setHorizontalHeaderItem(columnCount, headerItem);

		columnCount++;
	}
	catch (std::exception ex)
	{
		std::cerr << "Error while filling table widget " << ui->tableWidget_consume->objectName().toStdString() << "! Exception: " << ex.what() << std::endl;
		return false;
	}

	std::vector<CSVRow> rows;

	if (!readCSVFile(consumationCSVPath.string(), rows))
	{
		std::cerr << "Error reading csv file!" << std::endl;
		return false;
	}

	for (CSVRow row : rows)
	{
		if (!addRowToTableWidget(ui->tableWidget_consume, row))
		{
			std::cerr << "Error adding row to table!" << std::endl;
			return false;
		}
	}

	return true;
}

bool MainWindow::readCSVFile(std::string filename, std::vector<CSVRow>& outRows)
{
	std::cout << "Trying to read csv file: " << filename << std::endl;


	try
	{
		if (fileExists(filename))
		{
			std::cout << "File " << filename << " exists" << std::endl;

			std::ifstream file(filename);

			for (CSVRow csvRow : CSVRange(file))
			{
				outRows.push_back(csvRow);
			}

			file.close();
		}
		else
		{
			std::cout << "File " << filename << " does not exist! Trying to create it" << std::endl;

			std::ofstream{ filename };
		}
	}
	catch (const std::exception& ex)
	{
		std::cerr << "Error reading csv file " << filename << "! Exception: " << ex.what() << std::endl;
		return false;
	}


	return true;
}

bool MainWindow::addRowToTableWidget(QTableWidget* tableWidget, CSVRow row)
{
	std::vector<std::string> stringRow;
	return addRowToTableWidget(tableWidget, row, stringRow);
}

bool MainWindow::addRowToTableWidget(QTableWidget* tableWidget, std::vector<std::string> row)
{
	CSVRow csvRow;
	return addRowToTableWidget(tableWidget, csvRow, row);
}

bool MainWindow::addRowToTableWidget(QTableWidget* tableWidget, CSVRow csvRow, std::vector<std::string> stringRow)
{
	int size;
	bool isCSV = false;

	if (csvRow.size() == -1 && stringRow.size() != 0)
	{
		size = stringRow.size();
	}
	else if (csvRow.size() != -1 && stringRow.size() == 0)
	{
		size = csvRow.size();
		isCSV = true;
	}
	else
	{
		return false;
	}


	try
	{
		tableWidget->insertRow(tableWidget->rowCount()); //add new row

		for (int column = 0; column < size; column++)
		{
			std::string item;
			if (isCSV)
			{
				item = csvRow[column];
			}
			else
			{
				item = stringRow[column];
			}

			if (addItemToTableWidget(tableWidget, item, tableWidget->rowCount() - 1, column))
			{
				ui->statusbar->showMessage("Artikel wurde erfolgreich hinzugefuegt.");
			}
			else
			{
				std::cerr << "Error while inserting item in table widget!" << std::endl;
				return false;
			}


		}
	}
	catch (std::exception& ex)
	{
		std::cerr << "Error while inserting new row in table widget! Exception: " << ex.what() << std::endl;
		return false;
	}

	return true;
}

bool MainWindow::addItemToTableWidget(QTableWidget* tableWidget, std::string item, int row, int column)
{
	QTableWidgetItem* newItem = new QTableWidgetItem(item.c_str());
	tableWidget->setItem(row, column, newItem);

	std::cout << "Added item " << item << " to table widget " << tableWidget->objectName().toStdString() << std::endl;
	return true;
}

bool MainWindow::writeTableWidgetToCSVfile(std::string csvFilePath, QTableWidget* tableWidget)
{
	if (fileExists(csvFilePath))
	{
		try
		{
			std::ofstream file(csvFilePath);

			for (int row = 0; row < tableWidget->rowCount(); row++)
			{
				for (int column = 0; column < tableWidget->columnCount(); column++)
				{
					QTableWidgetItem* item = tableWidget->item(row, column);
					if (item != nullptr)
					{
						std::string data = item->text().toStdString();
						file << data.c_str();
					}

					if (column != tableWidget->columnCount() - 1)
					{
						file << ";";
					}
				}

				if (row != tableWidget->rowCount() - 1)
				{
					file << "\n";
				}
			}
			file.close();
		}
		catch (std::exception& ex)
		{
			std::cerr << "Error during writing table: " << tableWidget->objectName().toStdString() << " to csv file: " << csvFilePath << "! Exception: " << ex.what() << std::endl;
			return false;
		}
	}
	else
	{
		std::cerr << "Error file " << csvFilePath << " does not exist!" << std::endl;
		return false;
	}

	std::cout << "CSV file " << csvFilePath << " successfully written" << std::endl;

	return true;
}

bool MainWindow::findMemberByNameAndAlias(QTableWidget* tableWidget, std::string name, std::string alias, QTableWidgetItem* outName, QTableWidgetItem* outAlias)
{
	std::cout << "Trying to find member with name " << name << " and alias " << alias<<" in table "<<tableWidget->objectName().toStdString() << std::endl;

	for (int row = 0; row < tableWidget->rowCount(); row++)
	{
		QTableWidgetItem* nameToCompare = tableWidget->item(row, 0);
		QTableWidgetItem* aliasToCompare = tableWidget->item(row, 1);

		if (nameToCompare->text().toStdString() == name && aliasToCompare->text().toStdString() == alias)
		{
			outName = nameToCompare;
			outAlias = aliasToCompare;
			std::cout << "Found a match" << std::endl;
			return true;
		}
	}

	std::cout << "Didn't find a match" << std::endl;
	return false;
}

bool findItemInHeader(QTableWidget* tableWidget, std::string itemName, QTableWidgetItem* outItemName)
{
	std::cout << "Trying to find item " << itemName << " in table " << tableWidget->objectName().toStdString()<<" header" << std::endl;

	for (int column = 0; column < tableWidget->columnCount(); column++)
	{
		QTableWidgetItem* itemNameToCompare = tableWidget->horizontalHeaderItem(column);

		if (itemNameToCompare->text().toStdString() == itemName)
		{
			outItemName = itemNameToCompare;
			std::cout << "Found a match!" << std::endl;
			return true;
		}
	}

	std::cout << "Didn't find a match" << std::endl;
	return false;
}
