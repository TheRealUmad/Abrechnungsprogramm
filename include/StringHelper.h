#pragma once
#include <string>
#include <QTableWidgetItem>

class StringHelper
{
public:
	static bool isDigit(std::string str);
	static bool checkDoubleDigitString(std::string& digitString);
	static bool checkIntDigitString(std::string& digitString);
	static bool checkDoubleDigitItem(QTableWidgetItem* item);
	static bool checkIntDigitItem(QTableWidgetItem* item);
};

