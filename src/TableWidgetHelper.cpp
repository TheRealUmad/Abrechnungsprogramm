#include "TableWidgetHelper.h"
#include <spdlog/spdlog.h>
#include <QString>
#include <FileHelper.h>
#include <ConfigHandler.h>
#include <StringHelper.h>

bool TableWidgetHelper::addRowToTableWidget(QTableWidget* tableWidget, std::vector<std::string> row)
{
	CSVRow csvRow;
	return addRowToTableWidget(tableWidget, csvRow, row);
}

bool TableWidgetHelper::addRowToTableWidget(QTableWidget* tableWidget, CSVRow row)
{
	std::vector<std::string> stringRow;
	return addRowToTableWidget(tableWidget, row, stringRow);
}

bool TableWidgetHelper::addRowToTableWidget(QTableWidget* tableWidget, CSVRow csvRow, std::vector<std::string> stringRow)
{
	spdlog::info("Trying to add row to table widget: " + tableWidget->objectName().toStdString());

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
		std::string message = "Row to insert into table widget: "+tableWidget->objectName().toStdString()+" is empty!";
		spdlog::error(message);
		throw std::exception(message.c_str());
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

			if (!addItemToTableWidget(tableWidget, item, tableWidget->rowCount() - 1, column))
			{
				std::string message = "Could not insert item: " + item + " into table widget: " + tableWidget->objectName().toStdString();
				spdlog::error(message);
				throw std::exception(message.c_str());
			}
		}
	}
	catch (const std::exception& ex)
	{
		std::string message = "Could not insert new row in table widget! Exception: " + (std::string)ex.what();
		spdlog::error(message);
		throw std::exception(message.c_str());
	}

	return true;
}

bool TableWidgetHelper::deleteEmptyRowsOfTableWidget(QTableWidget* tableWidget)
{
	spdlog::info("Trying to delete empty rows of table widget: " + tableWidget->objectName().toStdString());
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
				spdlog::info("Removing empty row with index: " + std::to_string(row) + " from table: " + tableWidget->objectName().toStdString());
				tableWidget->removeRow(row);
			}
		}
	}
	catch (std::exception ex)
	{
		std::string message = "Could not remove empty rows of table widget: " + tableWidget->objectName().toStdString()+"! Exception: "+ex.what();
		spdlog::error(message);
		throw std::exception(message.c_str());
	}

	return true;
}

bool TableWidgetHelper::deleteEmptyColumnOfTableWidget(QTableWidget* tableWidget)
{
	spdlog::info("Trying to remove empty rows of table widget: " + tableWidget->objectName().toStdString());
	try
	{
		for (int column = 0; column < tableWidget->columnCount(); column++)
		{
			QTableWidgetItem* item = tableWidget->horizontalHeaderItem(column);
			if (item == nullptr || item->text().isEmpty())
			{
				spdlog::info("Removing empty column with index: " + std::to_string(column) + " from table widget: " + tableWidget->objectName().toStdString());
				tableWidget->removeColumn(column);
			}
		}
	}
	catch (std::exception ex)
	{
		std::string message = "Could not remove empty columns of table widget: " + tableWidget->objectName().toStdString()+"! Exception: "+ex.what();
		spdlog::error(message);
		throw std::exception(message.c_str());
	}

	return true;
}

bool TableWidgetHelper::addItemToTableWidget(QTableWidget* tableWidget, std::string itemText, int row, int column)
{
	QString text = QString::fromStdString(itemText);
	return addItemToTableWidget(tableWidget, text, row, column);
}

bool TableWidgetHelper::addItemToTableWidget(QTableWidget* tableWidget, QTableWidgetItem* item, int row, int column)
{
	while (tableWidget->rowCount() - 1 < row)
	{
		tableWidget->insertRow(row);
		spdlog::debug("Increased row count of table widget: " + tableWidget->objectName().toStdString()+" by one.");
	}

	tableWidget->setItem(row, column, item);

	spdlog::debug("Added item: " + item->text().toStdString() + " to table widget: " + tableWidget->objectName().toStdString());
	return true;
}

bool TableWidgetHelper::addItemToTableWidget(QTableWidget* tableWidget, QTableWidgetItem* item, int row, int column, bool copyItem)
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

bool TableWidgetHelper::addItemToTableWidget(QTableWidget* tableWidget, QString itemText, int row, int column)
{
	QTableWidgetItem* newItem = new QTableWidgetItem(itemText);
	return addItemToTableWidget(tableWidget, newItem, row, column);
}

bool TableWidgetHelper::writeTableWidgetToCSVfile(std::string csvFilePath, QTableWidget* tableWidget)
{
	spdlog::info("Trying to write table widget: " + tableWidget->objectName().toStdString() + " to csv file: " + csvFilePath);

	FileHelper::checkFile(csvFilePath, false);
	
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
		std::string message = "Could not write table widget: " + tableWidget->objectName().toStdString() + " to csv file: " + csvFilePath + "! Exception: " + ex.what();
		spdlog::error(message);
		throw std::exception(message.c_str());
	}
	
	spdlog::info("Successfully wrote table widget " + tableWidget->objectName().toStdString() + " to csv file: " + csvFilePath);

	return true;
}

bool TableWidgetHelper::readCSVAndAddToTableWidget(std::string csvFilePath, QTableWidget* tableWidget)
{
	spdlog::info("Trying to read csv file: " + csvFilePath + " and add it to table widget: " + tableWidget->objectName().toStdString());

	std::vector<CSVRow> rows;

	FileHelper::readCSVFile(csvFilePath, rows);

	for (CSVRow row : rows)
	{
		TableWidgetHelper::addRowToTableWidget(tableWidget, row);
	}
}

bool TableWidgetHelper::generateConsumeTableHeader(QTableWidget* consumeTableWidget, QTableWidget* memberTableWidget, QTableWidget* itemsTableWidget)
{
	spdlog::info("Trying to generate consume table header");
	try
	{
		int nrOfItems = itemsTableWidget->rowCount();
		int nrOfMemberHeaders = memberTableWidget->columnCount();

		consumeTableWidget->setColumnCount(nrOfItems + nrOfMemberHeaders + 5); //columncount needs to be set first

		int columnCount = 0;

		for (int column = 0; column < memberTableWidget->columnCount(); column++)
		{
			QTableWidgetItem* item = new QTableWidgetItem(memberTableWidget->horizontalHeaderItem(column)->text());

			consumeTableWidget->setHorizontalHeaderItem(columnCount, item);
			columnCount++;
		}

		QTableWidgetItem* headerItem = new QTableWidgetItem("Uebertrag");
		headerItem->setTextAlignment(Qt::AlignCenter);
		consumeTableWidget->setHorizontalHeaderItem(columnCount, headerItem);

		columnCount++;

		for (int row = 0; row < itemsTableWidget->rowCount(); row++)
		{
			QTableWidgetItem* item = new QTableWidgetItem(itemsTableWidget->item(row, 0)->text());

			consumeTableWidget->setHorizontalHeaderItem(columnCount, item);
			columnCount++;
		}

		ConfigHandler::GetInstance()->GetAppConfig()->itemsInConsumeTableIndex = columnCount; //for new items to insert

		headerItem = new QTableWidgetItem("Umsatz");
		headerItem->setTextAlignment(Qt::AlignCenter);
		consumeTableWidget->setHorizontalHeaderItem(columnCount, headerItem);

		columnCount++;

		headerItem = new QTableWidgetItem("Einzahlungen");
		headerItem->setTextAlignment(Qt::AlignCenter);
		consumeTableWidget->setHorizontalHeaderItem(columnCount, headerItem);

		columnCount++;

		headerItem = new QTableWidgetItem("Schulden");
		headerItem->setTextAlignment(Qt::AlignCenter);
		consumeTableWidget->setHorizontalHeaderItem(columnCount, headerItem);

		columnCount++;

		headerItem = new QTableWidgetItem("Guthaben");
		headerItem->setTextAlignment(Qt::AlignCenter);
		consumeTableWidget->setHorizontalHeaderItem(columnCount, headerItem);

		columnCount++;
	}
	catch (std::exception ex)
	{
		std::string message = "Could not generate table header for table widget: " + consumeTableWidget->objectName().toStdString() + "! Exception: "+ ex.what();
		spdlog::error(message);
		throw std::exception(message.c_str());
	}

	return true;
}

bool TableWidgetHelper::addItemToConsumeTableHeader(QTableWidget* consumeTableWidget, std::string itemName)
{
	spdlog::info("Trying to add item name: " + itemName + " to header of table widget: " + consumeTableWidget->objectName().toStdString());

	int column = ConfigHandler::GetInstance()->GetAppConfig()->itemsInConsumeTableIndex;
	consumeTableWidget->insertColumn(ConfigHandler::GetInstance()->GetAppConfig()->itemsInConsumeTableIndex);
	QTableWidgetItem* newItem = new QTableWidgetItem(itemName.c_str());
	consumeTableWidget->setHorizontalHeaderItem(ConfigHandler::GetInstance()->GetAppConfig()->itemsInConsumeTableIndex, newItem);

	ConfigHandler::GetInstance()->GetAppConfig()->itemsInConsumeTableIndex++; //so next new item will be inserted afterwards

	spdlog::info("Successfully added item name: " + itemName + " to header of table widget: " + consumeTableWidget->objectName().toStdString());
	return true;
}

bool TableWidgetHelper::findMemberByNameAndAlias(QTableWidget* tableWidget, QString firstName, QString lastName, QString alias, QTableWidgetItem* &outFirstName, QTableWidgetItem* &outLastName, QTableWidgetItem* &outAlias)
{
	spdlog::info("Trying to find member: " + firstName.toStdString() + " " + lastName.toStdString() + " v/o " + alias.toStdString() + " in table widget: " + tableWidget->objectName().toStdString());

	QTableWidgetItem* soutAlias;
	QTableWidgetItem* soutFirstName;
	QTableWidgetItem* soutLastName;

	int matches = 0;

	for (int row = 0; row < tableWidget->rowCount()-1; row++)
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
			spdlog::debug(matchCount + " critearia match was found.");
			
			soutAlias = aliasToCompare;
			soutFirstName = firstNameToCompare;
			soutLastName = lastNameToCompare;
			matches++;
		}
		else if (matchCount == 2)
		{
			spdlog::debug(matchCount + " critearia match was found.");

			spdlog::debug("Trying to partially match the missing criteria.");

			bool partiallyMatched = false;

			if (!alias.isEmpty() || !firstName.isEmpty() || !lastName.isEmpty() || !aliasToCompare->text().isEmpty() || !firstNameToCompare->text().isEmpty() || !lastNameToCompare->text().isEmpty())
			{
				if (!alias.count() < 4 || !firstName.count() < 4 || !lastName.count() < 4)
				{
					if (!aliasMatch)
					{
						if (aliasToCompare->text().contains(alias))
						{
							spdlog::debug("Partially matched alias");
							partiallyMatched = true;
						}
					}
					else if (!firstNameMatch)
					{
						if (firstNameToCompare->text().contains(firstName))
						{
							spdlog::debug("Partially matched first name");
							partiallyMatched = true;
						}
					}
					else if (!lastNameMatch)
					{
						if (lastNameToCompare->text().contains(lastName))
						{
							spdlog::debug("Partially matched last name");
							partiallyMatched = true;
						}
					}
				}
				else
				{
					spdlog::warn("Cannot partially match strings with less than 4 characters");
				}
			}
			else
			{
				spdlog::error("Cannot partially match empty strings");
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
				spdlog::warn("No partial match was found");
				soutAlias = aliasToCompare;
				soutFirstName = firstNameToCompare;
				soutLastName = lastNameToCompare;
				matches++;
			}
		}
		else
		{
			spdlog::warn("Too few criterias matched (" + std::to_string(matchCount) + ").");
		}
	}

	if (matches > 1)
	{
		spdlog::info("Found more than one match. Returning last found match.");
		outAlias = soutAlias;
		outFirstName = soutFirstName;
		outLastName = soutLastName;
		return false;
	}
	else if (matches == 1)
	{
		spdlog::info("Found exactly one match.");
		outAlias = soutAlias;
		outFirstName = soutFirstName;
		outLastName = soutLastName;
		return true;
	}

	spdlog::warn("Found no match.");
	return false;
}

int TableWidgetHelper::findColumnInTableHeader(QTableWidget* tableWidget, QString headerText)
{
	spdlog::info("Trying to find column: " + headerText.toStdString() + " in table widget: " + tableWidget->objectName().toStdString());

	for (int column = 0; column < tableWidget->columnCount(); column++)
	{
		QTableWidgetItem* item = tableWidget->horizontalHeaderItem(column);
		if (item->text() == headerText)
		{
			spdlog::info("Found column: " + headerText.toStdString() + " in table widget: " + tableWidget->objectName().toStdString() + " at index: " + std::to_string(column));
			return column;
		}
	}

	spdlog::warn("Could not find column: " + headerText.toStdString() + " in table widget: " + tableWidget->objectName().toStdString());
	return -1;
}

bool TableWidgetHelper::findRelevantColumnIndexes(QTableWidget* consumeTable, QTableWidget* itemTable, int& columnCarryover, int& columnDeposits, int& columnTurnover, int& columnDebt, int& columnCredit, int& columnItemsStart, int& columnItemsEnd)
{
	spdlog::info("Trying to find relevant column indexes.");

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
		std::string message = "Could not find the needed table headers in consume table!";
		spdlog::error(message);
		throw std::exception(message.c_str());
	}

	spdlog::info("Successfully found relevant column indexes.");
	return true;
}

bool TableWidgetHelper::findMemberByAliasInConsumeTable(QTableWidget* tableWidget, QString alias, QTableWidgetItem* &outAlias)
{
	spdlog::info("Trying to find member with alias: " + alias.toStdString() + " in table widget: " + tableWidget->objectName().toStdString());

	if (alias.isEmpty())
	{
		spdlog::warn("Alias is empty!");
		return false;
	}

	int matches = 0;
	int partialMatchesCs = 0;
	int partialMatchesCis = 0;

	QTableWidgetItem* lastMatch;
	QTableWidgetItem* lastPartialMatchCs;
	QTableWidgetItem* lastPartialMatchCis;

	for (int row = 0; row < tableWidget->rowCount(); row++)
	{
		QTableWidgetItem* aliasToCompare = tableWidget->item(row, 0);
		
		if (aliasToCompare!=nullptr && aliasToCompare->text() == alias)
		{
			spdlog::info("Found perfect match.");
			matches++;
			//lastMatch = std::pair<int,int>(aliasToCompare->row(), aliasToCompare->column());
			lastMatch = aliasToCompare;
		}
		else if(aliasToCompare!=nullptr && alias.count()>=4)
		{
			spdlog::debug("Trying to find partial match.");

			if (aliasToCompare->text().contains(alias, Qt::CaseSensitive))
			{
				spdlog::info("Found partial match:" + aliasToCompare->text().toStdString() + " (case sensitive).");
				partialMatchesCs++;
				//lastPartialMatchCs = std::pair<int,int>(aliasToCompare->row(), aliasToCompare->column());
				lastPartialMatchCs = aliasToCompare;
			}
			else if(aliasToCompare->text().contains(alias, Qt::CaseInsensitive))
			{
				spdlog::info("Found partial match: " + aliasToCompare->text().toStdString() + " (case insensitive).");
				partialMatchesCis++;
				//lastPartialMatchCis = std::pair<int,int>(aliasToCompare->row(), aliasToCompare->column());
				lastPartialMatchCis = aliasToCompare;
			}
			else
			{
				spdlog::debug("Found no partial match.");
			}
		}
		else if (aliasToCompare != nullptr)
		{
			spdlog::warn("Cannot partially match: " + alias.toStdString() + ", because there are to few characters in the string! (minimum 4 characters needed)");
		}
		else
		{
			spdlog::warn("Alias in table widget was empty!");
		}
	}

	if (matches >= 1)
	{
		if (matches == 1)
			spdlog::info("Found exactly one perfect match.");
		else
			spdlog::info("Found: " + std::to_string(matches) + " perfect matches. Returning the last one.");

		//outAlias = tableWidget->item(lastMatch.first, lastMatch.second);
		outAlias = lastMatch;
		return true;
	}
	else if(partialMatchesCs >= 1)
	{
		if (partialMatchesCs == 1)
			spdlog::info("Found exactly one partial case sensitive match.");
		else
			spdlog::info("Found: " + std::to_string(partialMatchesCs) + " partial case sensitive matches. Returning the last one.");

		//outAlias = tableWidget->item(lastPartialMatchCs.first, lastPartialMatchCs.second);
		outAlias = lastPartialMatchCs;
		return true;
	}
	else if (partialMatchesCis >= 1)
	{
		if (partialMatchesCis == 1)
			spdlog::info("Found exactly one partial case insensitive match.");
		else
			spdlog::info("Found: " + std::to_string(partialMatchesCis) + " partial case insensitive matches. Returning the last one.");

		//outAlias = tableWidget->item(lastPartialMatchCis.first, lastPartialMatchCis.second);
		outAlias = lastPartialMatchCis;
		return true;
	}
	else
	{
		spdlog::info("Found no match.");
	}

	return false;
}

bool TableWidgetHelper::clearColumnOfTable(QTableWidget* tableWidget, int column)
{
	spdlog::info("Trying to clear column: " + std::to_string(column) + " of table widget: " + tableWidget->objectName().toStdString());

	for (int row = 0; row < tableWidget->rowCount(); row++)
	{
		QTableWidgetItem* item = tableWidget->item(row, column);

		if (item != nullptr && !item->text().isEmpty())
		{
			spdlog::info("Clearing text of item: " + item->text().toStdString() + " in row: " + std::to_string(row));
			item->setText("");
		}
		else
		{
			spdlog::info("Nothing to clear. Item is already empty.");
		}
	}

	spdlog::info("Successfully cleared column: " + std::to_string(column) + " of table widget: " + tableWidget->objectName().toStdString());
	return true;
}

bool TableWidgetHelper::copyColumnOfTable(QTableWidget* tableWidget, int columnFrom, int columnTo)
{
	spdlog::info("Trying to copy column: " + std::to_string(columnFrom) + " to column: " + std::to_string(columnTo) + " in table widget: " + tableWidget->objectName().toStdString());

	for (int row = 0; row < tableWidget->rowCount(); row++)
	{
		QTableWidgetItem* itemFrom = tableWidget->item(row, columnFrom);

		if (itemFrom != nullptr && !itemFrom->text().isEmpty())
		{
			spdlog::info("Copying test of item: " + itemFrom->text().toStdString() + " in row: " + std::to_string(row));
			TableWidgetHelper::addItemToTableWidget(tableWidget, itemFrom, row, columnTo, true);
		}
		else
		{
			spdlog::warn("Nothing to copy. Item is empty.");
		}
	}

	spdlog::info("Successfully copied column: " + std::to_string(columnFrom) + " to column: " + std::to_string(columnTo) + " in table widget: " + tableWidget->objectName().toStdString());
	return true;
}
