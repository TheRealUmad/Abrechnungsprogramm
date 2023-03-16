#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <fstream>
//#include <CSVParser.h>
#include <QTableWidget>
#include <AddItemDialog.h>
#include <AddMemberDialog.h>
#include <qmessagebox.h>
#include <qpdfwriter.h>
#include <qtexttable.h>
#include <qprinter.h>

MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	char szFilePath[MAX_PATH + 1] = { 0 };
	GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
	/*
	strrchr: function function: find the position of the last occurrence of a character c in another string str (that is, find the position of the first occurrence of the character c from the right side of str),
	 And return the address of this location. If the specified character cannot be found, the function returns NULL.
	 Use this address to return the string from the last character c to the end of str.
	*/
	(strrchr(szFilePath, '\\'))[0] = 0; // Delete the file name, only get the path string //
	exePath = szFilePath;

	consumationCSVPath = exePath + "/Abrechnung.csv";
	memberCSVPath = exePath + "/Mitglieder.csv";
	itemCSVPath = exePath + "/Artikel.csv";
	saveFileDir = exePath + "/Sicherungskopien";
	saveFilePath = saveFileDir + "/Artikel_SicherungsKopie.csv";

	pdfFileDir = exePath + "/PDFs";
	if (!createDir(pdfFileDir))
	{
		std::cerr << "Error! Cannot create dir" << std::endl;
	}
	pdfFilePath = pdfFileDir + "/Abrechnung.pdf";

	if (!createDir(saveFileDir))
	{
		std::cerr << "Error! Cannot create dir" << std::endl;
	}

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
	QObject::connect(ui->btn_newCalculation, &QPushButton::clicked, this, &MainWindow::onButtonNewCalculationClick);
	QObject::connect(ui->btn_saveAsPdf, &QPushButton::clicked, this, &MainWindow::onButtonPrintToPdfClick);
	QObject::connect(ui->tableWidget_items, &QTableWidget::itemChanged, this, &MainWindow::onItemTableWidgetItemsChanged);
	QObject::connect(ui->tableWidget_member, &QTableWidget::itemChanged, this, &MainWindow::onItemTableWidgetMemberChanged);
	QObject::connect(ui->tableWidget_consume, &QTableWidget::itemChanged, this, &MainWindow::onItemTableWidgetConsumeChanged);
	QObject::connect(ui->tableWidget_consume, &QTableWidget::itemDoubleClicked, this, &MainWindow::onItemTableWidgetConsumeDoubleClicked);
	QObject::connect(ui->tableWidget_items->model(), &QAbstractTableModel::rowsInserted, this, &MainWindow::onTableWidgetItemsRowInserted);
	QObject::connect(ui->tableWidget_items->model(), &QAbstractTableModel::rowsRemoved, this, &MainWindow::onTableWidgetItemsRowDeleted);

}

bool MainWindow::createFile(std::string filePath)
{
	if (std::filesystem::exists(filePath))
	{
		std::cout << "File " << filePath << " already exists." << std::endl;
	}
	else
	{
		std::cout << "File " << filePath << " does not exist. Trying to create it" << std::endl;
		std::ofstream{ filePath };
	}
	return true;
}
bool MainWindow::createDir(std::string dirPath)
{
	if (std::filesystem::is_directory(dirPath))
	{
		std::cout << "Directory " << dirPath << " already exists." << std::endl;
	}
	else
	{
		std::cout << "Directory " << dirPath << " does not exist. Trying to create it" << std::endl;
		return std::filesystem::create_directories(dirPath);
	}
	return true;
}
bool MainWindow::copyFile(std::string fromPath, std::string toPath)
{
	std::cout << "Trying to copy file " << fromPath << " to " << toPath << std::endl;
	if (std::filesystem::exists(fromPath))
	{
		//createFile(toPath);
		std::filesystem::copy_file(fromPath, toPath);
	}
	else
	{
		std::cout << "File " << fromPath << " does not exist. Nothing to copy." << std::endl;
		return false;
	}
	return true;
}
bool MainWindow::deleteFile(std::string filePath)
{
	std::cout << "Trying to delete file " << filePath << std::endl;
	if (std::filesystem::exists(filePath))
	{
		return std::filesystem::remove(filePath);
	}
	else
	{
		std::cout << "File " << filePath << " does not exist. Nothing to delete." << std::endl;
	}
	return true;
}

MainWindow::~MainWindow()
{
	try
	{
		delete ui;
	}
	catch (std::exception ex)
	{
		std::cerr << "Error during deletion of ui! Exception: " << ex.what() << std::endl;
	}
}

void MainWindow::onButtonPrintToPdfClick()
{
	std::cout << "Button print to PDF clicked" << std::endl;

	QTableWidget* consumeTable = ui->tableWidget_consume;

	int columnCount = 0;

	QString headerTextAlias = "Coleurname";
	QString headerTextCarryover = "Uebertrag";
	QString headerTextDeposits = "Einzahlungen";
	QString headerTextTurnover = "Umsatz";
	QString headerTextDebt = "Schulden";
	QString headerTextCredit = "Guthaben";

	QList<QString> headerTexts;
	headerTexts.push_back(headerTextAlias);
	headerTexts.push_back(headerTextCarryover);
	headerTexts.push_back(headerTextDeposits);
	headerTexts.push_back(headerTextTurnover);
	headerTexts.push_back(headerTextDebt);
	headerTexts.push_back(headerTextCredit);

	std::map<int, int> columnTranslatorMap;

	int columnAlias = findColumnInTableHeader(consumeTable, headerTextAlias);
	columnTranslatorMap.insert(std::pair<int, int>(columnCount, columnAlias));
	columnCount++;
	int columnCarryover = findColumnInTableHeader(consumeTable, headerTextCarryover);
	columnTranslatorMap.insert(std::pair<int, int>(columnCount, columnCarryover));
	columnCount++;
	int columnDeposits = findColumnInTableHeader(consumeTable, headerTextDeposits);
	columnTranslatorMap.insert(std::pair<int, int>(columnCount, columnDeposits));
	columnCount++;
	int columnTurnover = findColumnInTableHeader(consumeTable, headerTextTurnover);
	columnTranslatorMap.insert(std::pair<int, int>(columnCount, columnTurnover));
	columnCount++;
	int columnDebt = findColumnInTableHeader(consumeTable, headerTextDebt);
	columnTranslatorMap.insert(std::pair<int, int>(columnCount, columnDebt));
	columnCount++;
	int columnCredit = findColumnInTableHeader(consumeTable, headerTextCredit);
	columnTranslatorMap.insert(std::pair<int, int>(columnCount, columnCredit));
	columnCount++;

	if (columnAlias == -1 || columnCarryover == -1 || columnDeposits == -1 || columnTurnover == -1 || columnDebt == -1 || columnCredit == -1)
	{
		std::cout << "Error! Could not find column indexes" << std::endl;
		return;
	}

	//relevant columns for pdf: alias, carryover, deposits, turnover, debt, credit = 6

	QTextDocument* doc = new QTextDocument;
	//doc->setDocumentMargin(10);
	//doc->setPageSize(QPageSize::A4);
	QTextCursor cursor(doc);

	QTextCharFormat boldFormat;
	boldFormat.setFontWeight(QFont::Bold);

	QTextCharFormat textFormat;
	textFormat.setBackground(Qt::white);

	QTextCharFormat redFormat;
	redFormat.setBackground(Qt::red);

	QTextCharFormat alternateCellFormat;
	alternateCellFormat.setBackground(Qt::lightGray);

	QTextTableFormat tableFormat;
	tableFormat.setAlignment(Qt::AlignCenter);
	tableFormat.setBorderCollapse(true);
	QTextLength tableWidth = QTextLength(QTextLength::PercentageLength, 80);
	tableFormat.setWidth(tableWidth);
	tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);

	cursor.movePosition(QTextCursor::Start);

	QTextTable* table = cursor.insertTable(consumeTable->rowCount() + 1, columnCount, tableFormat); //+1 because table header

	std::cout << "Generating document table header" << std::endl;
	for (int column = 0; column < columnCount; column++)
	{
		QTextTableCell headerCell = table->cellAt(0, column);
		QTextCursor headerCellCursor = headerCell.firstCursorPosition();
		headerCellCursor.insertText(headerTexts.at(column), boldFormat);
	}

	std::cout << "Generating document table items" << std::endl;
	for (int row = 0; row < consumeTable->rowCount(); row++)
	{
		QTextCharFormat cellFormat = row % 2 == 0 ? textFormat : alternateCellFormat;
		for (int column = 0; column < columnCount; column++)
		{

			QTableWidgetItem* consumeTableItem = consumeTable->item(row, columnTranslatorMap.at(column));

			if (consumeTableItem != nullptr && !consumeTableItem->text().isEmpty())
			{
				QTextTableCell cell = table->cellAt(row + 1, column);
				QTextCursor cellCursor = cell.firstCursorPosition();

				if (consumeTableItem->column() == columnDebt)
				{
					cell.setFormat(redFormat);
				}
				else
				{
					cell.setFormat(cellFormat);
				}

				std::string cellText = consumeTableItem->text().toStdString();
				checkDoubleDigitString(cellText);

				std::cout << "Inserting text " << cellText << " from consume table into document" << std::endl;
				cellCursor.insertText(QString::fromStdString(cellText));
			}
		}
	}

	cursor.movePosition(QTextCursor::End);
	cursor.insertBlock();

	std::cout << "Trying to print document to pdf" << std::endl;

	QPrinter printer(QPrinter::HighResolution);
	printer.setPageOrientation(QPageLayout::Portrait);
	printer.setPageSize(QPageSize::A4);
	printer.setFullPage(true);
	printer.setOutputFileName(QString::fromStdString(pdfFilePath));
	doc->print(&printer);

}

void MainWindow::onButtonNewCalculationClick()
{
	std::cout << "Button new calculation clicked" << std::endl;

	std::string message = "Alle Konsumationen und Einzahlungen werden auf 0 gesetzt. Aktuelle Guthaben bzw. Schulden werden in die Spalte Uebertrag gesetzt. \n Willst du wirklich eine neue Abrechnung beginnen?";

	auto reply = QMessageBox::question(this, QString::fromStdString("Warnung"), QString::fromStdString(message), QMessageBox::Ok | QMessageBox::Cancel);

	if (reply == QMessageBox::Ok)
	{
		std::cout << "New calculation shall begin!" << std::endl;

		QTableWidget* consumeTable = ui->tableWidget_consume;
		QTableWidget* itemsTable = ui->tableWidget_items;

		std::cout << "Trying to get relevant column indexes" << std::endl;

		int columnCarryover;
		int columnDeposits;
		int columnTurnover;
		int columnDebt;
		int columnCredit;
		int columnItemsStart;
		int columnItemsEnd;

		if (!findRelevantColumnIndexes(consumeTable, itemsTable, columnCarryover, columnDeposits, columnTurnover, columnDebt, columnCredit, columnItemsStart, columnItemsEnd))
		{
			std::cout << "Error! Could not find column indexes" << std::endl;
			return;
		}

		std::cout << "Trying to create a save file" << std::endl;

		deleteFile(saveFilePath); //delete old save file
		copyFile(consumationCSVPath, saveFilePath); //create save file

		std::cout << "Trying to prepare consume table for new calculation" << std::endl;

		consumeTable->blockSignals(true);

		copyColumnOfTable(consumeTable, columnDebt, columnCarryover); //copy debt and credit values into carryover
		copyColumnOfTable(consumeTable, columnCredit, columnCarryover);

		for (int column = columnItemsStart; column <= columnItemsEnd; column++)
		{
			clearColumnOfTable(consumeTable, column); //delete all item values
		}

		clearColumnOfTable(consumeTable, columnTurnover); //delete turnover values

		clearColumnOfTable(consumeTable, columnDeposits); //delete deposit values

		clearColumnOfTable(consumeTable, columnDebt); //delete credit and debt values
		clearColumnOfTable(consumeTable, columnCredit);

		calculateAndUpdateConsumeTable(consumeTable, itemsTable, columnCarryover, columnDeposits, columnTurnover, columnDebt, columnCredit, columnItemsStart, columnItemsEnd);
		consumeTable->blockSignals(false);

		//consumeTable->update();
		writeTableWidgetToCSVfile(consumationCSVPath, consumeTable);

		std::cout << "Ready for new calculation" << std::endl;
	}
}

bool MainWindow::findRelevantColumnIndexes(QTableWidget* consumeTable, QTableWidget* itemTable, int& columnCarryover, int& columnDeposits, int& columnTurnover, int& columnDebt, int& columnCredit, int& columnItemsStart, int& columnItemsEnd)
{
	std::cout << "Trying to get relevant column indexes" << std::endl;

	columnCarryover = findColumnInTableHeader(consumeTable, "Uebertrag");
	columnDeposits = findColumnInTableHeader(consumeTable, "Einzahlungen");
	columnTurnover = findColumnInTableHeader(consumeTable, "Umsatz");
	columnDebt = findColumnInTableHeader(consumeTable, "Schulden");
	columnCredit = findColumnInTableHeader(consumeTable, "Guthaben");

	columnItemsStart = -1;
	columnItemsEnd = -1;

	QTableWidgetItem* firstItem = itemTable->item(0, 0);

	if (firstItem != nullptr && !firstItem->text().isEmpty())
	{
		columnItemsStart = findColumnInTableHeader(consumeTable, firstItem->text());
	}

	QTableWidgetItem* lastItem = itemTable->item(itemTable->rowCount() - 1, 0);

	if (lastItem != nullptr && !lastItem->text().isEmpty())
	{
		columnItemsEnd = findColumnInTableHeader(consumeTable, lastItem->text());
	}

	if (columnCarryover == -1 || columnDeposits == -1 || columnTurnover == -1 || columnDebt == -1 || columnCredit == -1 || columnItemsStart == -1 || columnItemsEnd == -1)
	{
		std::cout << "Could not find the needed table headers in consume table" << std::endl;
		return false;
	}

	return true;
}

void MainWindow::onButtonAddItemClick()
{
	std::cout << "Button add item clicked" << std::endl;

	std::vector<std::string> newRow;
	std::string itemName;

	QStringList list = AddItemDialog::getStrings(this);
	if (!list.isEmpty())
	{
		itemName = list.first().toStdString();
		if (!itemName.empty())
		{
			std::cout << "New item name: " << itemName << std::endl;
			newRow.push_back(itemName);
		}
		else
		{
			std::cout << "Error! Item name is empty!" << std::endl;
			return;
		}
		std::string itemValue = list.last().toStdString();
		if (!itemValue.empty())
		{
			std::cout << "New item value: " << itemValue << std::endl;

			if (!checkDoubleDigitString(itemValue))
			{
				std::cout << "Error! Item value was not correct!" << std::endl;
				return;
			}
			else
			{
				newRow.push_back(itemValue);
			}
		}
		else
		{
			std::cout << "Error! Item name is empty!" << std::endl;
			return;
		}

	}
	else
	{
		std::cout << "Error! Item name and value is empty!" << std::endl;
		return;
	}

	ui->tableWidget_items->blockSignals(true);
	ui->tableWidget_consume->blockSignals(true);

	addRowToTableWidget(ui->tableWidget_items, newRow);
	addItemToConsumeTableHeader(itemName);
	writeTableWidgetToCSVfile(itemCSVPath, ui->tableWidget_items);
	writeTableWidgetToCSVfile(consumationCSVPath, ui->tableWidget_consume);

	ui->tableWidget_consume->blockSignals(false);
	ui->tableWidget_items->blockSignals(false);

}

bool MainWindow::checkIntDigitString(std::string& digitString)
{
	std::cout << "Trying to check int digit string: " << digitString << std::endl;

	std::replace(digitString.begin(), digitString.end(), ',', '.'); //replace all , with .

	if (digitString.find('.') == std::string::npos) //see if . was found
	{
		std::string digitCleaned;
		if (digitString.find('-') != std::string::npos)
		{
			digitCleaned = digitString.substr(digitString.find('-') + 1, digitString.length());
		}
		else
		{
			digitCleaned = digitString;
		}
		if (isDigit(digitCleaned))
		{
			std::cout << "Digit string: " << digitString << " is ok" << std::endl;
			return true;
		}
		else
		{
			std::cout << "Digit string " << digitString << " is not a digit!" << std::endl;
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool MainWindow::checkDoubleDigitString(std::string& digitString)
{
	std::cout << "Trying to check double digit string: " << digitString << std::endl;

	std::replace(digitString.begin(), digitString.end(), ',', '.'); //replace all , with .

	size_t posPoint = digitString.find('.');

	if (posPoint == std::string::npos) //see if . was found
	{
		if (checkIntDigitString(digitString))
		{
			std::cout << "Digit string: " << digitString << " is ok" << std::endl;
			return true;
		}
		else
		{
			std::cout << "Digit string " << digitString << " is not a digit!" << std::endl;
			return false;
		}
	}
	else
	{
		std::string::difference_type n = std::count(digitString.begin(), digitString.end(), '.'); //count number of .
		if (n == 1)
		{
			digitString.erase(posPoint + 3); //erase after two digits after '.'

			std::string firstDigitsCleaned;

			std::string firstDigits = digitString.substr(0, digitString.find('.'));
			if (digitString.find('-') != std::string::npos)
			{
				firstDigitsCleaned = firstDigits.substr(firstDigits.find('-') + 1, firstDigits.length());
			}
			else
			{
				firstDigitsCleaned = firstDigits;
			}
			std::string lastDigits = digitString.substr(digitString.find('.') + 1, digitString.length());

			std::cout << "first digits: " << firstDigits << " last digits: " << lastDigits << std::endl;

			if (isDigit(firstDigitsCleaned) && isDigit(lastDigits))
			{
				std::cout << "Digit string: " << digitString << " is ok" << std::endl;
				return true;
			}
			else
			{
				std::cout << "Digit string " << digitString << " is not a digit!" << std::endl;
				return false;
			}
		}
		else
		{
			std::cout << "Digit string as more than one '.'" << std::endl;
			return false;
		}
	}
}

bool MainWindow::checkDoubleDigitItem(QTableWidgetItem* item)
{
	std::cout << "Trying to check double digit item" << std::endl;

	std::string itemText = item->text().toStdString();

	if (checkDoubleDigitString(itemText))
	{
		std::cout << "Trying to set the corrected value" << std::endl;
		item->setText(QString::fromStdString(itemText));
		return true;
	}
	else
	{
		std::cout << "Digit item was not correct. Setting it blank." << std::endl;
		item->setText(QString::fromStdString(""));
		return false;
	}
}

bool MainWindow::checkIntDigitItem(QTableWidgetItem* item)
{
	std::cout << "Trying to check digit item" << std::endl;

	std::string itemText = item->text().toStdString();

	if (checkIntDigitString(itemText))
	{
		std::cout << "Trying to set the corrected value" << std::endl;
		item->setText(QString::fromStdString(itemText));
		return true;
	}
	else
	{
		std::cout << "Digit item was not correct. Setting it blank." << std::endl;
		item->setText(QString::fromStdString(""));
		return false;
	}
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

		std::string memberFirstName = list.at(1).toStdString();
		std::string memberLastName = list.last().toStdString();
		if (!memberFirstName.empty() && !memberLastName.empty())
		{
			std::cout << "New member name: " << memberFirstName << " " << memberLastName << std::endl;
			newRow.push_back(memberFirstName);
			newRow.push_back(memberLastName);
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

	/*ui->tableWidget_member->blockSignals(true);
	ui->tableWidget_member->blockSignals(true);*/
	addRowToTableWidget(ui->tableWidget_member, newRow);
	//addRowToTableWidget(ui->tableWidget_consume, newRow);
	/*ui->tableWidget_member->blockSignals(false);
	ui->tableWidget_member->blockSignals(false);*/
}

void MainWindow::onItemTableWidgetItemsChanged(QTableWidgetItem* changedItem)
{
	QObject* obj = sender();
	QTableWidget* tableWidget = qobject_cast<QTableWidget*>(obj);

	std::cout << "Item: " << changedItem->text().toStdString() << " changed in table " << tableWidget->objectName().toStdString() << std::endl;

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

	updateConsumeTableHeader();

	if (rowIsEmpty)
	{
		deleteEmptyColumnOfTableWidget(ui->tableWidget_consume); //remove header from consume
		writeTableWidgetToCSVfile(consumationCSVPath, ui->tableWidget_consume);
		itemsInConsumeTableIndex--; //so next new item will be inserted correctly

		std::cout << "Removing empty row with index: " << tableWidget->currentRow() << " from table: " << tableWidget->objectName().toStdString() << std::endl;
		tableWidget->removeRow(changedItem->row());
	}

	//deleteEmptyRowsOfTableWidget(tableWidget);
	writeTableWidgetToCSVfile(itemCSVPath, tableWidget);
}

void MainWindow::onItemTableWidgetMemberChanged(QTableWidgetItem* changedItem)
{
	QObject* obj = sender();
	QTableWidget* tableWidget = qobject_cast<QTableWidget*>(obj);

	std::cout << "Item: " << changedItem->text().toStdString() << " changed in table " << tableWidget->objectName().toStdString() << std::endl;

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
		std::cout << "Member should be deleted." << std::endl;

		deleteEmptyMemberFromTable(tableWidget);
	}
	//}

	//deleteEmptyRowsOfTableWidget(tableWidget);
	updateMemberInMemberAndConsumeTable(changedItem);
	writeTableWidgetToCSVfile(memberCSVPath, tableWidget);
	writeTableWidgetToCSVfile(consumationCSVPath, tableWidget);
}

void MainWindow::onItemTableWidgetConsumeChanged(QTableWidgetItem* changedItem)
{
	QObject* obj = sender();
	QTableWidget* tableWidget = qobject_cast<QTableWidget*>(obj);

	std::cout << "Item: " << changedItem->text().toStdString() << " changed in table " << tableWidget->objectName().toStdString() << std::endl;

	if (changedItem->column() < 3) //first 3 columns are member data
	{
		std::cout << "Item changed in member data" << std::endl;

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
			std::cout << "Member should be deleted." << std::endl;

			deleteEmptyMemberFromTable(tableWidget);
		}
		updateMemberInMemberAndConsumeTable(changedItem);
	}
	else
	{
		int columnCarryover;
		int columnDeposits;
		int columnTurnover;
		int columnDebt;
		int columnCredit;

		int columnItemsStart;
		int columnItemsEnd;

		if (!findRelevantColumnIndexes(tableWidget, ui->tableWidget_items, columnCarryover, columnDeposits, columnTurnover, columnDebt, columnCredit, columnItemsStart, columnItemsEnd))
		{
			std::cout << "Error! Could not find column indexes" << std::endl;
			return;
		}

		if (changedItem->column() == columnCarryover)
		{
			tableWidget->blockSignals(true);
			checkDoubleDigitItem(changedItem);
			tableWidget->blockSignals(false);
		}
		else if (changedItem->column() == columnDeposits)
		{
			tableWidget->blockSignals(true);
			checkDoubleDigitItem(changedItem);
			calculateAndUpdateConsumeTable(tableWidget, ui->tableWidget_items, columnCarryover, columnDeposits, columnTurnover, columnDebt, columnCredit, columnItemsStart, columnItemsEnd);
			tableWidget->blockSignals(false);
		}
		else if (changedItem->column() == columnTurnover)
		{
			tableWidget->blockSignals(true);
			checkDoubleDigitItem(changedItem);
			tableWidget->blockSignals(false);
		}
		else if (changedItem->column() == columnDebt)
		{
			tableWidget->blockSignals(true);
			checkDoubleDigitItem(changedItem);
			tableWidget->blockSignals(false);
		}
		else if (changedItem->column() == columnCredit)
		{
			tableWidget->blockSignals(true);
			checkDoubleDigitItem(changedItem);
			tableWidget->blockSignals(false);
		}
		else if (changedItem->column() >= columnItemsStart && changedItem->column() <= columnItemsEnd)
		{
			tableWidget->blockSignals(true);
			checkIntDigitItem(changedItem);
			calculateAndUpdateConsumeTable(tableWidget, ui->tableWidget_items, columnCarryover, columnDeposits, columnTurnover, columnDebt, columnCredit, columnItemsStart, columnItemsEnd);
			tableWidget->blockSignals(false);
		}
	}

	//deleteEmptyRowsOfTableWidget(tableWidget);
	writeTableWidgetToCSVfile(memberCSVPath, tableWidget);
	writeTableWidgetToCSVfile(consumationCSVPath, tableWidget);
}

void MainWindow::generateItemValueMap(QTableWidget* itemTable, QTableWidget* consumeTable, int columnItemsStart, int columnItemsEnd, std::map<int, double>& outItemValueMap)
{
	std::cout << "Trying to generate item value map" << std::endl;

	for (int columnItems = columnItemsStart; columnItems <= columnItemsEnd; columnItems++)
	{
		QTableWidgetItem* itemNameItem = consumeTable->horizontalHeaderItem(columnItems);

		if (itemNameItem != nullptr && !itemNameItem->text().isEmpty())
		{
			QString itemName = itemNameItem->text();

			QList<QTableWidgetItem*> itemsInItemTable = itemTable->findItems(itemName, Qt::MatchFlag::MatchExactly);

			if (!itemsInItemTable.isEmpty() && itemsInItemTable.size() == 1)
			{
				QTableWidgetItem* itemInItemTable = itemsInItemTable.first();

				if (itemInItemTable != nullptr && !itemInItemTable->text().isEmpty())
				{
					QTableWidgetItem* itemValueInItemTable = itemTable->item(itemInItemTable->row(), 1);

					if (itemValueInItemTable != nullptr && !itemValueInItemTable->text().isEmpty())
					{
						double itemValue = itemValueInItemTable->text().toDouble();

						std::cout << "Item " << itemName.toStdString() << " in column " << columnItems << " has value " << itemValue << std::endl;
						outItemValueMap.insert(std::pair<int, double>(columnItems, itemValue));
					}
				}
			}
		}
	}

	std::cout << "Generating item value map complete" << std::endl;
}

bool MainWindow::calculateAndUpdateConsumeTable(QTableWidget* consumeTable, QTableWidget* itemTable, int columnCarryover, int columnDeposits, int columnTurnover, int columnDebt, int columnCredit, int columnItemsStart, int columnItemsEnd)
{
	std::cout << "Trying to calculate and update consume table" << std::endl;

	std::map<int, double> itemValueMap;

	generateItemValueMap(itemTable, consumeTable, columnItemsStart, columnItemsEnd, itemValueMap);

	if (itemValueMap.empty())
	{
		std::cout << "Item value map was empty. Cannot do calculation." << std::endl;
		return false;
	}

	for (int row = 0; row < consumeTable->rowCount(); row++)
	{
		QTableWidgetItem* carryoverItem = consumeTable->item(row, columnCarryover);
		QTableWidgetItem* depositItem = consumeTable->item(row, columnDeposits);
		QTableWidgetItem* turnoverItem = consumeTable->item(row, columnTurnover);
		QTableWidgetItem* debtItem = consumeTable->item(row, columnDebt);
		QTableWidgetItem* creditItem = consumeTable->item(row, columnCredit);

		double turnoverValue = 0.0;
		for (int columnItems = columnItemsStart; columnItems <= columnItemsEnd; columnItems++)
		{
			QTableWidgetItem* itemItem = consumeTable->item(row, columnItems);

			if (itemItem != nullptr && !itemItem->text().isEmpty())
			{
				int itemQuantity = 0;
				itemQuantity = itemItem->text().toInt();
				double itemValue = 0.0;
				itemValue = itemValueMap.at(columnItems);

				std::cout << "Added " << itemQuantity << " * " << itemValue << " to turnover value" << std::endl;

				turnoverValue += itemQuantity * itemValue;
			}
		}

		double carryoverValue = 0.0;
		double depositValue = 0.0;
		double debtValue = 0.0;
		double creditValue = 0.0;

		if (carryoverItem != nullptr && !carryoverItem->text().isEmpty())
		{
			carryoverValue = carryoverItem->text().toDouble();
		}

		if (depositItem != nullptr && !depositItem->text().isEmpty())
		{
			depositValue = depositItem->text().toDouble();
		}

		double debtOrCreditValue = carryoverValue + depositValue - turnoverValue;

		if (debtOrCreditValue < 0)
		{
			debtValue = debtOrCreditValue;
			std::cout << "Member in row " << row << " has " << debtValue << " euro debt" << std::endl;
		}
		else if (debtOrCreditValue > 0)
		{
			creditValue = debtOrCreditValue;
			std::cout << "Member in row " << row << " has " << creditValue << " euro credit" << std::endl;
		}
		else
		{
			std::cout << "Member in row " << row << " has " << debtOrCreditValue << " euro credit/debt" << std::endl;
		}

		if (turnoverValue != 0.0)
		{
			std::cout << "Setting turnover value to " << turnoverValue << std::endl;
			addItemToTableWidget(consumeTable, std::to_string(turnoverValue), row, columnTurnover);
		}
		else
		{
			std::cout << "Turnover was 0. Not setting it." << std::endl;
			addItemToTableWidget(consumeTable, QString::fromStdString(""), row, columnTurnover);
		}

		if (debtValue != 0.0)
		{
			std::cout << "Setting debt value to " << debtValue << std::endl;
			addItemToTableWidget(consumeTable, std::to_string(debtValue), row, columnDebt);
		}
		else
		{
			std::cout << "Debt was 0. Not setting it." << std::endl;
			addItemToTableWidget(consumeTable, QString::fromStdString(""), row, columnDebt);
		}

		if (creditValue != 0.0)
		{
			std::cout << "Setting credit value to " << creditValue << std::endl;
			addItemToTableWidget(consumeTable, std::to_string(creditValue), row, columnCredit);
		}
		else
		{
			std::cout << "Credit was 0. Not setting it." << std::endl;
			addItemToTableWidget(consumeTable, QString::fromStdString(""), row, columnCredit);
		}

		std::cout << "Calculation and update of row " << row << " finished" << std::endl;
	}

	std::cout << "Calculation and update for consume table finished" << std::endl;
	return true;
}

void MainWindow::onItemTableWidgetConsumeDoubleClicked(QTableWidgetItem* item)
{
	QObject* obj = sender();
	QTableWidget* tableWidget = qobject_cast<QTableWidget*>(obj);

	std::cout << "Item " << item << " double clicked" << std::endl;

	int columnCarryover = findColumnInTableHeader(tableWidget, "Uebertrag");
	int columnTurnover = findColumnInTableHeader(tableWidget, "Umsatz");
	int columnDebt = findColumnInTableHeader(tableWidget, "Schulden");
	int columnCredit = findColumnInTableHeader(tableWidget, "Guthaben");

	if (item->column() == columnCarryover || item->column() == columnDebt || item->column() == columnCredit || item->column() == columnTurnover)
	{
		std::cout << "Item can not be edited" << std::endl;
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
	for (int row = 0; row < tableWidget->rowCount(); row++)
	{
		QTableWidgetItem* memberAlias = tableWidget->item(row, 0);
		QTableWidgetItem* memberFirstName = tableWidget->item(row, 1);
		QTableWidgetItem* memberLastName = tableWidget->item(row, 2);

		if (memberAlias->text().isEmpty() && memberFirstName->text().isEmpty() && memberLastName->text().isEmpty())
		{
			std::cout << "Found a row with empty member data in table: " << tableWidget->objectName().toStdString() << std::endl;

			double debt = 0.0;
			double credit = 0.0;
			if (memberHasDebtOrCredit(row, debt, credit))
			{
				std::string message = "Member still has " + std::to_string(debt) + " Euro debt and " + std::to_string(credit) + " Euro credit.";
				std::cout << message << std::endl;

				message = "Mitglied hat noch " + std::to_string(debt) + " Euro Schulden und " + std::to_string(credit) + " Euro Guthaben. \n Willst du das Mitglied wirklich loeschen?";

				auto reply = QMessageBox::question(this, QString::fromStdString("Warnung"), QString::fromStdString(message), QMessageBox::Ok | QMessageBox::Cancel);
				if (reply == QMessageBox::Ok)
				{
					deleteRowFromMemberAndConsumeTable(row);
					std::cout << "Row with empty member deleted." << std::endl;
					return true;
				}
				else
				{
					std::cout << "Row with empty member should not be deleted." << std::endl;
					tableWidget->blockSignals(true);
					restoreRowFromMemberOrConsumeTable(tableWidget, row);
					tableWidget->blockSignals(false);
					return false;
				}
			}
			else
			{
				std::cout << "Member can be deleted because he has no credit or debt" << std::endl;
				deleteRowFromMemberAndConsumeTable(row);
			}
		}
	}
}

bool MainWindow::clearColumnOfTable(QTableWidget* tableWidget, int column)
{
	std::cout << "Trying to clear column " << column << " of table: " << tableWidget->objectName().toStdString() << std::endl;

	for (int row = 0; row < tableWidget->rowCount(); row++)
	{
		QTableWidgetItem* item = tableWidget->item(row, column);

		if (item != nullptr && !item->text().isEmpty())
		{
			std::cout << "Clearing text of item " << item->text().toStdString() << " in row " << row << std::endl;
			item->setText("");
		}
		else
		{
			std::cout << "Nothing to clear. Item is already empty." << std::endl;
		}
	}
	return true;

}

bool MainWindow::copyColumnOfTable(QTableWidget* tableWidget, int columnFrom, int columnTo)
{
	std::cout << "Trying to copy column " << columnFrom << " to column " << columnTo << " in table: " << tableWidget->objectName().toStdString() << std::endl;

	for (int row = 0; row < tableWidget->rowCount(); row++)
	{
		QTableWidgetItem* itemFrom = tableWidget->item(row, columnFrom);

		if (itemFrom != nullptr && !itemFrom->text().isEmpty())
		{
			std::cout << "Copying text of item " << itemFrom->text().toStdString() << " in row " << row << std::endl;
			addItemToTableWidget(tableWidget, itemFrom, row, columnTo, true);
		}
		else
		{
			std::cout << "Nothing to copy. Item is empty." << std::endl;
		}
	}
	return true;
}

bool MainWindow::restoreRowFromMemberOrConsumeTable(QTableWidget* tableWidgetToRestore, int rowToRestore)
{
	std::cout << "Trying to restore row " << rowToRestore << " of table widget " << tableWidgetToRestore->objectName().toStdString() << std::endl;

	if (tableWidgetToRestore->objectName() == ui->tableWidget_consume->objectName())
	{
		for (int i = 0; i < 3; i++)
		{
			if (!addItemToTableWidget(tableWidgetToRestore, ui->tableWidget_member->item(rowToRestore, i), rowToRestore, i, true))
			{
				return false;
			}
		}
	}
	else if (tableWidgetToRestore->objectName() == ui->tableWidget_member->objectName())
	{
		for (int i = 0; i < 3; i++)
		{
			if (!addItemToTableWidget(tableWidgetToRestore, ui->tableWidget_consume->item(rowToRestore, i), rowToRestore, i, true))
			{
				return false;
			}
		}
	}
	else
	{
		std::cout << "Unknown table widget" << std::endl;
		return false;
	}

	return true;
}

bool MainWindow::deleteRowFromMemberAndConsumeTable(int row)
{
	std::cout << "Trying to remove row " << row << " from consume and member table" << std::endl;

	QTableWidget* consumeTable = ui->tableWidget_consume;
	QTableWidget* memberTable = ui->tableWidget_member;

	consumeTable->blockSignals(true);
	memberTable->blockSignals(true);

	consumeTable->removeRow(row);
	memberTable->removeRow(row);

	consumeTable->blockSignals(false);
	memberTable->blockSignals(false);

	std::cout << "Removed row " << row << " from consume and member table" << std::endl;
	return true;
}

bool MainWindow::updateMemberInMemberAndConsumeTable(QTableWidgetItem* changedItem)
{
	std::cout << "Trying to update member in member and consume table" << std::endl;

	if (changedItem != nullptr && changedItem->tableWidget() == ui->tableWidget_consume)
	{
		if (!changedItem->text().isEmpty())
		{
			QTableWidgetItem* itemInMemberTable = ui->tableWidget_member->item(changedItem->row(), changedItem->column());
			if (itemInMemberTable != nullptr && changedItem->text() == itemInMemberTable->text())
			{
				std::cout << "Not inserting item from consume table into member table because the text is the same" << std::endl;
				return true;
			}

			std::cout << "Trying to insert item from consume table into member table." << std::endl;

			//ui->tableWidget_member->blockSignals(true);
			if (!addItemToTableWidget(ui->tableWidget_member, changedItem->text(), changedItem->row(), changedItem->column()))
			{
				//ui->tableWidget_member->blockSignals(false);
				std::cout << "Could not insert item!" << std::endl;
				return false;
			}
			//ui->tableWidget_member->blockSignals(false);
		}
		else
		{
			std::cout << "Not inserting item from consume table into member table because the text is empty" << std::endl;
		}
	}
	else if (changedItem != nullptr && changedItem->tableWidget() == ui->tableWidget_member)
	{
		if (!changedItem->text().isEmpty())
		{
			QTableWidgetItem* itemInConsumeTable = ui->tableWidget_consume->item(changedItem->row(), changedItem->column());
			if (itemInConsumeTable != nullptr && changedItem->text() == itemInConsumeTable->text())
			{
				std::cout << "Not inserting item from member table into consume table because the text is the same" << std::endl;
				return true;
			}

			std::cout << "Trying to insert item from member table into consume table." << std::endl;

			//ui->tableWidget_consume->blockSignals(true);
			if (!addItemToTableWidget(ui->tableWidget_consume, changedItem->text(), changedItem->row(), changedItem->column()))
			{
				//ui->tableWidget_consume->blockSignals(false);
				std::cout << "Could not insert item!" << std::endl;
				return false;
			}
			//ui->tableWidget_consume->blockSignals(true);
		}
		else
		{
			std::cout << "Not inserting item from member table into consume table because the text is empty" << std::endl;
		}
	}
	else
	{
		std::cout << "Changed item was null or table widget was not recognized" << std::endl;
	}

	return true;
}

bool MainWindow::memberHasDebtOrCredit(int rowInConsumeTable, double& outDebt, double& outCredit)
{
	QTableWidget* tableWidget = ui->tableWidget_consume;

	int columnDebt = findColumnInTableHeader(tableWidget, QString::fromStdString("Schulden"));
	int columnCredit = findColumnInTableHeader(tableWidget, QString::fromStdString("Guthaben"));

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

		std::cout << "Member has " << outDebt << " debt and " << outCredit << " credit." << std::endl;
		if (outDebt == 0.0 && outCredit == 0.0)
		{
			std::cout << "Member has no debt or credit." << std::endl;
			return false;
		}
		else
		{
			return true;
		}
	}

	std::cout << "Column for debt or credit was not found in table: " << tableWidget->objectName().toStdString() << std::endl;
	return true;

}

int MainWindow::findColumnInTableHeader(QTableWidget* tableWidget, QString headerText)
{
	for (int column = 0; column < tableWidget->columnCount(); column++)
	{
		QTableWidgetItem* item = tableWidget->horizontalHeaderItem(column);
		if (item->text() == headerText)
		{
			return column;
		}
	}

	return -1;
}

bool MainWindow::updateConsumeTableHeader()
{
	return generateConsumeTableHeader();
}

bool MainWindow::deleteEmptyColumnOfTableWidget(QTableWidget* tableWidget)
{
	try
	{
		for (int column = 0; column < tableWidget->columnCount(); column++)
		{
			QTableWidgetItem* item = tableWidget->horizontalHeaderItem(column);
			if (item == nullptr || item->text().isEmpty())
			{
				std::cout << "Removing empty column with index: " << column << " from table: " << tableWidget->objectName().toStdString() << std::endl;
				tableWidget->removeColumn(column);
			}
		}
	}
	catch (std::exception ex)
	{
		std::cerr << "Error during removing empty columns from table! Exception: " << ex.what() << std::endl;
		return false;
	}

	return true;
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
	if (!readCSVFile(itemCSVPath, rows))
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

	std::vector<CSVRow> rows;

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
	}

	return true;
}

bool MainWindow::fillConsumationList()
{
	if (!generateConsumeTableHeader())
	{
		return false;
	}

	std::vector<CSVRow> rows;

	if (!readCSVFile(consumationCSVPath, rows))
	{
		std::cerr << "Error reading csv file!" << std::endl;
		return false;
	}

	if (!rows.empty())
	{
		for (CSVRow row : rows)
		{
			if (!addRowToTableWidget(ui->tableWidget_consume, row))
			{
				std::cerr << "Error adding row to table!" << std::endl;
				return false;
			}
		}
	}
	else
	{
		std::cout << "Consumation csv file: " << consumationCSVPath << " is empty. Trying to fill table widget " << ui->tableWidget_consume->objectName().toStdString() << " with members." << std::endl;
		ui->tableWidget_consume->setRowCount(ui->tableWidget_member->rowCount()); //equalize row count
		for (int row = 0; row < ui->tableWidget_member->rowCount(); row++)
		{
			for (int column = 0; column < ui->tableWidget_member->columnCount(); column++)
			{
				addItemToTableWidget(ui->tableWidget_consume, ui->tableWidget_member->item(row, column)->text(), row, column);
			}
		}
	}

	return true;
}

bool MainWindow::generateConsumeTableHeader()
{
	try
	{
		int nrOfItems = ui->tableWidget_items->rowCount();
		int nrOfMemberHeaders = ui->tableWidget_member->columnCount();

		ui->tableWidget_consume->setColumnCount(nrOfItems + nrOfMemberHeaders + 5); //columncount needs to be set first

		int columnCount = 0;

		/*QTableWidgetItem* headerItem = new QTableWidgetItem("Coleurname");
		headerItem->setTextAlignment(Qt::AlignCenter);
		ui->tableWidget_consume->setHorizontalHeaderItem(columnCount, headerItem);*/

		/*columnCount++;*/

		for (int column = 0; column < ui->tableWidget_member->columnCount(); column++)
		{
			QTableWidgetItem* item = new QTableWidgetItem(ui->tableWidget_member->horizontalHeaderItem(column)->text());

			ui->tableWidget_consume->setHorizontalHeaderItem(columnCount, item);
			columnCount++;
		}

		/*headerItem = new QTableWidgetItem("Name");
		headerItem->setTextAlignment(Qt::AlignCenter);
		ui->tableWidget_consume->setHorizontalHeaderItem(columnCount, headerItem);

		columnCount++;*/

		QTableWidgetItem* headerItem = new QTableWidgetItem("Uebertrag");
		headerItem->setTextAlignment(Qt::AlignCenter);
		ui->tableWidget_consume->setHorizontalHeaderItem(columnCount, headerItem);

		columnCount++;

		for (int row = 0; row < ui->tableWidget_items->rowCount(); row++)
		{
			QTableWidgetItem* item = new QTableWidgetItem(ui->tableWidget_items->item(row, 0)->text());
			//std::string itemName = item->text().toStdString();

			ui->tableWidget_consume->setHorizontalHeaderItem(columnCount, item);
			columnCount++;
		}

		itemsInConsumeTableIndex = columnCount; //for new items to insert

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

		headerItem = new QTableWidgetItem("Guthaben");
		headerItem->setTextAlignment(Qt::AlignCenter);
		ui->tableWidget_consume->setHorizontalHeaderItem(columnCount, headerItem);

		columnCount++;
	}
	catch (std::exception ex)
	{
		std::cerr << "Error while filling table widget " << ui->tableWidget_consume->objectName().toStdString() << "! Exception: " << ex.what() << std::endl;
		return false;
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
				ui->statusbar->showMessage("Item wurde erfolgreich hinzugefuegt.");
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

bool MainWindow::addItemToConsumeTableHeader(std::string itemName)
{
	int column = itemsInConsumeTableIndex;
	ui->tableWidget_consume->insertColumn(column);
	QTableWidgetItem* newItem = new QTableWidgetItem(itemName.c_str());
	ui->tableWidget_consume->setHorizontalHeaderItem(column, newItem);

	itemsInConsumeTableIndex++; //so next new item will be inserted afterwards

	std::cout << "Added item name " << itemName << " to header of table widget " << ui->tableWidget_consume->objectName().toStdString() << std::endl;
	return true;
}

bool MainWindow::addItemToTableWidget(QTableWidget* tableWidget, std::string itemText, int row, int column)
{
	QString text = QString::fromStdString(itemText);
	return addItemToTableWidget(tableWidget, text, row, column);
}

bool MainWindow::addItemToTableWidget(QTableWidget* tableWidget, QString itemText, int row, int column)
{
	QTableWidgetItem* newItem = new QTableWidgetItem(itemText);
	return addItemToTableWidget(tableWidget, newItem, row, column);
}

bool MainWindow::addItemToTableWidget(QTableWidget* tableWidget, QTableWidgetItem* item, int row, int column, bool copyItem)
{
	if (copyItem)
	{
		return addItemToTableWidget(tableWidget, new QTableWidgetItem(item->text()), row, column);
	}
	else
	{
		return addItemToTableWidget(tableWidget, item, row, column);
	}
}

bool MainWindow::addItemToTableWidget(QTableWidget* tableWidget, QTableWidgetItem* item, int row, int column)
{
	while (tableWidget->rowCount() - 1 < row)
	{
		std::cout << "Row count of table widget " << tableWidget->objectName().toStdString() << " is too low, increasing it." << std::endl;
		tableWidget->insertRow(row);
	}

	tableWidget->setItem(row, column, item);

	std::cout << "Added item " << item->text().toStdString() << " to table widget " << tableWidget->objectName().toStdString() << std::endl;
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

bool MainWindow::findMemberByNameAndAlias(QTableWidget* tableWidget, QString firstName, QString lastName, QString alias, QTableWidgetItem* outFirstName, QTableWidgetItem* outLastName, QTableWidgetItem* outAlias)
{
	std::cout << "Trying to find member with name " << firstName.toStdString() << " " << lastName.toStdString() << " and alias " << alias.toStdString() << " in table " << tableWidget->objectName().toStdString() << std::endl;

	QTableWidgetItem* soutAlias;
	QTableWidgetItem* soutFirstName;
	QTableWidgetItem* soutLastName;

	int matches = 0;

	for (int row = 0; row < tableWidget->rowCount(); row++)
	{

		QTableWidgetItem* aliasToCompare = tableWidget->item(row, 0);
		QTableWidgetItem* firstNameToCompare = tableWidget->item(row, 1);
		QTableWidgetItem* lastNameToCompare = tableWidget->item(row, 2);

		bool firstNameMatch = false, lastNameMatch = false, aliasMatch = false;
		int matchCount = 0;

		if (!alias.isEmpty() && aliasToCompare->text() == alias)
		{
			aliasMatch = true;
			matchCount++;
		}
		if (!firstName.isEmpty() && firstNameToCompare->text() == firstName)
		{
			firstNameMatch = true;
			matchCount++;
		}
		if (!lastName.isEmpty() && lastNameToCompare->text() == lastName)
		{
			lastNameMatch = true;
			matchCount++;
		}

		if (matchCount == 3)
		{
			std::cout << matchCount << " criteria match was found." << std::endl;
			soutAlias = aliasToCompare;
			soutFirstName = firstNameToCompare;
			soutLastName = lastNameToCompare;
			matches++;
		}
		else if (matchCount == 2)
		{
			std::cout << matchCount << " criteria match was found." << std::endl;

			std::cout << "Trying to partially match the missing criteria." << std::endl;

			bool partiallyMatched = false;

			if (!alias.isEmpty() || !firstName.isEmpty() || !lastName.isEmpty() || !aliasToCompare->text().isEmpty() || !firstNameToCompare->text().isEmpty() || !lastNameToCompare->text().isEmpty())
			{
				if (!alias.count() < 4 || !firstName.count() < 4 || !lastName.count() < 4)
				{
					if (!aliasMatch)
					{
						if (aliasToCompare->text().contains(alias))
						{
							std::cout << "Partially matched alias" << std::endl;
							partiallyMatched = true;
						}
					}
					else if (!firstNameMatch)
					{
						if (firstNameToCompare->text().contains(firstName))
						{
							std::cout << "Partially matched first name" << std::endl;
							partiallyMatched = true;
						}
					}
					else if (!lastNameMatch)
					{
						if (lastNameToCompare->text().contains(lastName))
						{
							std::cout << "Partially matched last name" << std::endl;
							partiallyMatched = true;
						}
					}
				}
				else
				{
					std::cout << "Cannot partially match strings with less than 4 characters" << std::endl;
				}
			}
			else
			{
				std::cout << "Cannot partially match empty strings" << std::endl;
			}

			if (partiallyMatched)
			{
				soutAlias = aliasToCompare;
				soutFirstName = firstNameToCompare;
				soutLastName = lastNameToCompare;
				matches++;
			}
			else
			{
				std::cout << "No partial match was found" << std::endl;
				soutAlias = aliasToCompare;
				soutFirstName = firstNameToCompare;
				soutLastName = lastNameToCompare;
				matches++;
			}
		}
		else
		{
			std::cout << "Too few criterias matched (" << matchCount << ")." << std::endl;
		}
	}

	if (matches > 1)
	{
		std::cout << "Found more than one match. Returning last found match." << std::endl;
		outAlias = soutAlias;
		outFirstName = soutFirstName;
		outLastName = soutLastName;
		return false;
	}
	else if (matches == 1)
	{
		std::cout << "Found exactly one match." << std::endl;
		outAlias = soutAlias;
		outFirstName = soutFirstName;
		outLastName = soutLastName;
		return true;
	}

	std::cout << "Found no match." << std::endl;
	return false;

}

//bool findItemInHeader(QTableWidget* tableWidget, std::string itemName, QTableWidgetItem* outItemName)
//{
//	std::cout << "Trying to find item " << itemName << " in table " << tableWidget->objectName().toStdString() << " header" << std::endl;
//
//	for (int column = 0; column < tableWidget->columnCount(); column++)
//	{
//		QTableWidgetItem* itemNameToCompare = tableWidget->horizontalHeaderItem(column);
//
//		if (itemNameToCompare->text().toStdString() == itemName)
//		{
//			outItemName = itemNameToCompare;
//			std::cout << "Found a match!" << std::endl;
//			return true;
//		}
//	}
//
//	std::cout << "Didn't find a match" << std::endl;
//	return false;
//}
