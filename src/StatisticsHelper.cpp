#include "StatisticsHelper.h"
#include <spdlog/spdlog.h>
#include <TableWidgetHelper.h>
#include <ConfigHandler.h>
#include <nlohmann/json.hpp>
#include <StringHelper.h>
#include <FileHelper.h>

void StatisticsHelper::gatherStatistics(QTableWidget* tableWidgetConsume, QTableWidget* tableWidgetItems)
{
	spdlog::info("Trying to gather statistics.");

	Statistics* statistics = new Statistics();

	readStatisticsFromFile(statistics);

	extractStatisticsOfConsumeTable(tableWidgetConsume, tableWidgetItems, statistics);

	writeStatisticsToFile(tableWidgetConsume, statistics);

	spdlog::info("Finished gathering statistics.");
}

bool StatisticsHelper::writeStatisticsToFile(QTableWidget* tableWidgetConsume, Statistics* &statistics)
{
	spdlog::info("Trying to write statistics to file: " + ConfigHandler::GetInstance()->GetAppConfig()->statisticsPath);

	FileHelper::checkFile(ConfigHandler::GetInstance()->GetAppConfig()->statisticsPath, false);

	try
	{
		std::ofstream file(ConfigHandler::GetInstance()->GetAppConfig()->statisticsPath);

		file << "Statistik für Budenwart:" << std::endl;
		file << std::endl;

		file << "Meisten Bierkonsum:" << std::endl;
		int lastBeer = -1;
		for (auto highestBeerRow : statistics->highestBeerRows)
		{
			if (lastBeer == -1 || lastBeer == highestBeerRow.second)
			{
				lastBeer = highestBeerRow.second;
				std::string aliasBeer = tableWidgetConsume->item(highestBeerRow.first, 0)->text().toStdString();
				std::string nameBeer = tableWidgetConsume->item(highestBeerRow.first, 1)->text().toStdString() + " " + tableWidgetConsume->item(highestBeerRow.first, 2)->text().toStdString();

				file << nameBeer << " v/o " << aliasBeer << " mit " << highestBeerRow.second << " Bier" << std::endl;
			}
			else
			{
				break;
			}
		}
		file << std::endl;

		file << "Meisten Umsatz:" << std::endl;
		double lastTurnover = -1.0;
		for (auto highestTurnoverRow : statistics->highestTurnoverRows)
		{
			if (lastTurnover == -1.0 || lastTurnover == highestTurnoverRow.second)
			{
				lastTurnover = highestTurnoverRow.second;
				std::string aliasTurnover = tableWidgetConsume->item(highestTurnoverRow.first, 0)->text().toStdString();
				std::string nameTurnover = tableWidgetConsume->item(highestTurnoverRow.first, 1)->text().toStdString() + " " + tableWidgetConsume->item(highestTurnoverRow.first, 2)->text().toStdString();

				file << nameTurnover << " v/o " << aliasTurnover << " mit " << highestTurnoverRow.second << "€ Umsatz" << std::endl;
			}
			else
			{
				break;
			}
		}
		file << std::endl;

		file << "Großschuldner über " << ConfigHandler::GetInstance()->GetAppConfig()->debtThreshhold << "€:" << std::endl;

		for (auto debtRow : statistics->highestDebtRows)
		{
			std::string aliasDebt = tableWidgetConsume->item(debtRow.first, 0)->text().toStdString();
			std::string nameDebt = tableWidgetConsume->item(debtRow.first, 1)->text().toStdString() + " " + tableWidgetConsume->item(debtRow.first, 2)->text().toStdString();
			file << nameDebt << " v/o " << aliasDebt << " mit " << debtRow.second << "€ Schulden" << std::endl;
		}
		file << std::endl;

		file << "Summe der konsumierten Artikel in dieser Abrechnung:" << std::endl;
		for (auto itemSum : statistics->itemSumMap)
		{
			std::string itemName = itemSum.first.first;
			std::string itemAmount = std::to_string(itemSum.second);

			file << itemName << ": " << itemAmount << std::endl;
		}

		file.close();
	}
	catch (std::exception& ex)
	{
		std::string message = "Could not write statistics to file: " + ConfigHandler::GetInstance()->GetAppConfig()->statisticsPath + "! Exception: " + ex.what();
		spdlog::error(message);
		throw std::exception(message.c_str());
	}

	spdlog::info("Successfully wrote statistics to file: " + ConfigHandler::GetInstance()->GetAppConfig()->statisticsPath);

	return true;
}

bool StatisticsHelper::readStatisticsFromFile(Statistics* &statistics)
{
	spdlog::info("Trying to read statistics from file.");

	spdlog::info("Finished reading statistics from file.");
	return true;
}

void StatisticsHelper::extractStatisticsOfConsumeTable(QTableWidget* consumeTableWidget, QTableWidget* itemsTableWidget, Statistics* &statistics)
{
	spdlog::info("Trying to extract statistics of consume table.");

	int columnCarryover;
	int columnDeposits;
	int columnTurnover;
	int columnDebt;
	int columnCredit;
	int columnItemsStart;
	int columnItemsEnd;

	if (!TableWidgetHelper::findRelevantColumnIndexes(consumeTableWidget, itemsTableWidget, columnCarryover, columnDeposits, columnTurnover, columnDebt, columnCredit, columnItemsStart, columnItemsEnd))
	{
		return;
	}

	int columnBeer = TableWidgetHelper::findColumnInTableHeader(consumeTableWidget, QString("Bier"));

	for (int row = 0; row < consumeTableWidget->rowCount(); row++)
	{
		for (int column = 0; column < consumeTableWidget->columnCount(); column++)
		{
			auto item = consumeTableWidget->item(row, column);

			if (column < columnItemsEnd && column >= columnItemsStart)
			{
				std::pair<std::string, int> itemIndexPair;
				itemIndexPair.first = consumeTableWidget->horizontalHeaderItem(column)->text().toStdString();
				itemIndexPair.second = column;

				auto it = statistics->itemSumMap.find(itemIndexPair);

				StringHelper::checkIntDigitItem(item);

				std::string itemText = item->text().toStdString();

				if (column == columnBeer)
				{
					if (!itemText.empty() || !all_of(itemText.begin(), itemText.end(), isspace))
					{
						int amount = std::stod(itemText);
						if (statistics->highestBeerRows.empty() || amount > statistics->highestBeerRows.back().second)
						{
							spdlog::info("New highest beer amount: " + std::to_string(amount) + " in row: " + std::to_string(row));
							std::pair<int, int> newBeerRow = std::pair<int, int>(row, amount);
							statistics->highestBeerRows.push_back(newBeerRow);
						}
					}
				}

				if (it != statistics->itemSumMap.end())
				{		
					if (!itemText.empty() || !all_of(itemText.begin(), itemText.end(), isspace))
					{
						spdlog::info("Adding value: " + itemText + " to sum of item: " + itemIndexPair.first);
						it->second += std::stod(itemText);
						spdlog::info("New sum of item: " + itemIndexPair.first + " is: " + std::to_string(it->second));
					}
				}
				else
				{
					if (!itemText.empty() || !all_of(itemText.begin(), itemText.end(), isspace))
					{
						spdlog::info("Adding new item: " + itemIndexPair.first + " with initial sum value: " + itemText);
						statistics->itemSumMap.insert(std::pair<std::pair<std::string, int>, double>(itemIndexPair, std::stod(itemText)));
					}
				}
			}
			else if (column == columnTurnover)
			{
				StringHelper::checkDoubleDigitItem(item);

				std::string itemText = item->text().toStdString();

				if (!itemText.empty() || !all_of(itemText.begin(), itemText.end(), isspace))
				{
					double amount = std::stod(itemText);

					if (statistics->highestTurnoverRows.empty() || amount >= statistics->highestTurnoverRows.back().second)
					{
						spdlog::info("New highest turnover amount: " + std::to_string(amount) + " in row: " + std::to_string(row));
						std::pair<int, double> newTurnoverRow = std::pair<int, double>(row, amount);
						statistics->highestTurnoverRows.push_back(newTurnoverRow);
					}
				}
			}
			else if (column == columnDebt)
			{
				StringHelper::checkDoubleDigitItem(item);

				std::string itemText = item->text().toStdString();

				if (!itemText.empty() || !all_of(itemText.begin(), itemText.end(), isspace))
				{
					double amount = std::stod(itemText);

					if ((-1.0) * amount >= ConfigHandler::GetInstance()->GetAppConfig()->debtThreshhold)
					{
						spdlog::info("New debt amount: " + std::to_string(amount) + " over threshhold: " + std::to_string(ConfigHandler::GetInstance()->GetAppConfig()->debtThreshhold) + " in row: " + std::to_string(row));
						std::pair<int, double> newDebtRow = std::pair<int, double>(row, amount);
						statistics->highestDebtRows.push_back(newDebtRow);
					}
				}
			}

		}
	}

	std::sort(statistics->highestBeerRows.begin(), statistics->highestBeerRows.end(), [](auto& left, auto& right) {
		return left.second > right.second;
	});

	std::sort(statistics->highestTurnoverRows.begin(), statistics->highestTurnoverRows.end(), [](auto& left, auto& right) {
		return left.second > right.second;
	});

	std::sort(statistics->highestDebtRows.begin(), statistics->highestDebtRows.end(), [](auto& left, auto& right) {
		return left.second < right.second;
	});

	spdlog::info("Finished extracting statistics of consume table.");
}

