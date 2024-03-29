#pragma once

#include <QTableWidget>
#include <CSVParser.h>

class TableWidgetHelper
{
public:
    static bool addRowToTableWidget(QTableWidget* tableWidget, std::vector<std::string> row);
	static bool addRowToTableWidget(QTableWidget* tableWidget, CSVRow row);
	static bool addRowToTableWidget(QTableWidget* tableWidget, CSVRow csvRow, std::vector<std::string> stringRow);

	static bool deleteEmptyRowsOfTableWidget(QTableWidget* tableWidget);
	static bool deleteEmptyColumnOfTableWidget(QWidget* parent, QTableWidget* tableWidget);
	static bool deleteEmptyMemberOfTableWidget(QWidget* parent, QTableWidget* tableWidget, QTableWidget* consumeTableWidget, QTableWidget* memberTableWidget);
	static bool deleteRowOfTableWidget(QTableWidget* tableWidget, int row, bool blockSignals = false);

	static bool restoreRowFromMemberOrConsumeTable(QTableWidget* tableWidgetToRestore, QTableWidget* consumeTableWidget, QTableWidget* memberTableWidget, int rowToRestore);

	static bool addItemToTableWidget(QTableWidget* tableWidget, std::string itemText, int row, int column);
	static bool addItemToTableWidget(QTableWidget* tableWidget, QTableWidgetItem* item, int row, int column);
	static bool addItemToTableWidget(QTableWidget* tableWidget, QTableWidgetItem* item, int row, int column, bool copyItem);
	static bool addItemToTableWidget(QTableWidget* tableWidget, QString itemText, int row, int column);

	static bool writeTableWidgetToCSVfile(std::string csvFilePath, QTableWidget* tableWidget);
	static bool readCSVAndAddToTableWidget(std::string csvFilePath, QTableWidget* tableWidget);

	static bool generateConsumeTableHeader(QTableWidget* consumeTableWidget, QTableWidget* memberTableWidget, QTableWidget* itemsTableWidget);
	static bool addItemToConsumeTableHeader(QTableWidget* consumeTableWidget, std::string itemName);

	static bool findMemberByNameAndAlias(QTableWidget* tableWidget, QString firstName, QString lastName, QString alias, QTableWidgetItem*& outFirstName, QTableWidgetItem*& outLastName, QTableWidgetItem*& outAlias);
	static int findColumnInTableHeader(QTableWidget* tableWidget, QString headerText);
	static bool findRelevantColumnIndexes(QTableWidget* consumeTable, QTableWidget* itemTable, int& columnCarryover, int& columnDeposits, int& columnTurnover, int& columnDebt, int& columnCredit, int& columnItemsStart, int& columnItemsEnd);
	static bool findMemberByAliasInConsumeTable(QTableWidget* tableWidget, QString alias, QTableWidgetItem* &outAlias);

	static bool clearColumnOfTable(QTableWidget* tableWidget, int column);
	static bool copyColumnOfTable(QTableWidget* tableWidget, int columnFrom, int columnTo);

	static bool memberHasDebtOrCredit(QTableWidget* consumeTableWidget, int rowInConsumeTable, double& outDebt, double& outCredit);

	static bool updateMemberInMemberAndConsumeTable(QTableWidgetItem* changedItem, QTableWidget* consumeTableWidget, QTableWidget* memberTableWidget);

	static bool updateConsumeTableVerticalHeader(QTableWidget* consumeTableWidget, int scrollValue);

	static bool checkColumnIsEmpty(QTableWidget* tableWidget, int column, QWidget* parent);
};

