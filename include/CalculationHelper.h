#pragma once
#include <QTableWidget>

class CalculationHelper
{
public:
	static bool calculateAndUpdateConsumeTable(QTableWidget* consumeTable, QTableWidget* itemTable, int columnCarryover, int columnDeposits, int columnTurnover, int columnDebt, int columnCredit, int columnItemsStart, int columnItemsEnd);
	static void generateItemValueMap(QTableWidget* itemTable, QTableWidget* consumeTable, int columnItemsStart, int columnItemsEnd, std::map<int, double>& outItemValueMap);
};

