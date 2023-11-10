#include "CalculationHelper.h"
#include "TableWidgetHelper.h"
#include <spdlog/spdlog.h>

bool CalculationHelper::calculateAndUpdateConsumeTable(QTableWidget* consumeTable, QTableWidget* itemTable, int columnCarryover, int columnDeposits, int columnTurnover, int columnDebt, int columnCredit, int columnItemsStart, int columnItemsEnd)
{
	spdlog::info("Trying to calculate and update consume table");

	std::map<int, double> itemValueMap;

	generateItemValueMap(itemTable, consumeTable, columnItemsStart, columnItemsEnd, itemValueMap);

	if (itemValueMap.empty())
	{
		std::string message = "Cannot do calculation because item value map was empty!";
		spdlog::error(message);
		throw std::exception(message.c_str());
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

				turnoverValue += itemQuantity * itemValue;

				spdlog::info("Added: " + std::to_string(turnoverValue) + " to turnover value");
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
			spdlog::info("Member in row: " + std::to_string(row) + " has: " + std::to_string(debtValue) + " euro debt.");
		}
		else if (debtOrCreditValue > 0)
		{
			creditValue = debtOrCreditValue;
			spdlog::info("Member in row: " + std::to_string(row) + " has: " + std::to_string(creditValue) + " euro debt.");
		}
		else
		{
			spdlog::info("Member in row: " + std::to_string(row) + " has: " + std::to_string(debtOrCreditValue) + " euro credit/debt.");
		}

		if (turnoverValue != 0.0)
		{
			spdlog::info("Setting turnover value to: " + std::to_string(turnoverValue));
			TableWidgetHelper::addItemToTableWidget(consumeTable, std::to_string(turnoverValue), row, columnTurnover);
		}
		else
		{
			spdlog::info("Turnover was 0. Not setting it.");
			TableWidgetHelper::addItemToTableWidget(consumeTable, QString::fromStdString(""), row, columnTurnover);
		}

		if (debtValue != 0.0)
		{
			spdlog::info("Setting debt value to: " + std::to_string(debtValue));
			TableWidgetHelper::addItemToTableWidget(consumeTable, std::to_string(debtValue), row, columnDebt);
		}
		else
		{
			spdlog::info("Debt was 0. Not setting it.");
			TableWidgetHelper::addItemToTableWidget(consumeTable, QString::fromStdString(""), row, columnDebt);
		}

		if (creditValue != 0.0)
		{
			spdlog::info("Setting debt value to: " + std::to_string(creditValue));
			TableWidgetHelper::addItemToTableWidget(consumeTable, std::to_string(creditValue), row, columnCredit);
		}
		else
		{
			spdlog::info("Credit was 0. Not setting it.");
			TableWidgetHelper::addItemToTableWidget(consumeTable, QString::fromStdString(""), row, columnCredit);
		}

		spdlog::info("Calculation and update of row: " + std::to_string(row) + " finished");
	}

	spdlog::info("Calcualtion and update for consume table finished");
	return true;
}

void CalculationHelper::generateItemValueMap(QTableWidget* itemTable, QTableWidget* consumeTable, int columnItemsStart, int columnItemsEnd, std::map<int, double>& outItemValueMap)
{
	spdlog::info("Trying to generate item value map");

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

						spdlog::info("Item: " + itemName.toStdString() + " in column: " + std::to_string(columnItems) + " has value: " + std::to_string(itemValue));
						outItemValueMap.insert(std::pair<int, double>(columnItems, itemValue));
					}
				}
			}
		}
	}

	spdlog::info("Successfully generated item value map");
}
