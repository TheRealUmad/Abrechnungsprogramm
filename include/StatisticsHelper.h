#pragma once
#include <QTableWidget>

struct DebugStatistics {
	int totalNumberBeer;
	DebugStatistics()
	{

	}
	~DebugStatistics();
};

struct Statistics {
	std::map<std::pair<std::string, int>,int> itemSumMap;
	std::vector<std::pair<int, int>> highestBeerRows;
	std::vector<std::pair<int, double>> highestTurnoverRows;
	//std::pair<int, int> highestBeerRow;
	//std::pair<int, double> highestTurnoverRow;
	std::vector<std::pair<int, double>> highestDebtRows;
	DebugStatistics* debugStatistics;
	Statistics()
	{
		debugStatistics = new DebugStatistics();
	}
	~Statistics();
};

class StatisticsHelper
{
public:
	static void gatherStatistics(QTableWidget* tableWidgetConsume, QTableWidget* tableWidgetItems);
	static bool writeStatisticsToFile(QTableWidget* tableWidgetConsume, Statistics* &statistics);
	static bool readStatisticsFromFile(Statistics* &statistics);
	static void extractStatisticsOfConsumeTable(QTableWidget* consumeTableWidget, QTableWidget* itemsTableWidget, Statistics* &statistics);
};

