#include "StringHelper.h"
#include <spdlog/spdlog.h>

bool StringHelper::isDigit(std::string str)
{
	spdlog::info("Trying to check if string: " + str + " is a number.");

	if (str.empty()||std::all_of(str.begin(), str.end(), isspace))
	{
		spdlog::info("String: " + str + " is not a number");
	}

	if (std::all_of(str.begin(), str.end(), ::isdigit))
	{
		spdlog::info("String: " + str + " is a number.");
		return true;
	}
	spdlog::info("String: " + str + " is not a number.");
	return false;
}

bool StringHelper::checkDoubleDigitString(std::string& digitString)
{
	spdlog::info("Trying to check double digit string: " + digitString);

	std::replace(digitString.begin(), digitString.end(), ',', '.'); //replace all , with .

	size_t posPoint = digitString.find('.');

	if (posPoint == std::string::npos) //see if . was found
	{
		if (checkIntDigitString(digitString))
		{
			spdlog::info("Digit string: " + digitString + " is ok.");
			return true;
		}
		else
		{
			spdlog::warn("Digit string " + digitString + " is not a digit!");
			return false;
		}
	}
	else
	{
		std::string::difference_type n = std::count(digitString.begin(), digitString.end(), '.'); //count number of .
		if (n == 1)
		{
			if (digitString.length() > posPoint + 3)
			{
				digitString.erase(posPoint + 3); //erase after two digits after '.'
			}

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

			spdlog::info("First digits: " + firstDigits + " last digits: " + lastDigits);

			if (isDigit(firstDigitsCleaned) && isDigit(lastDigits))
			{
				spdlog::info("Digit string: " + digitString + " is ok.");
				return true;
			}
			else
			{
				spdlog::warn("Digit string: " + digitString + " is not a digit!");
				return false;
			}
		}
		else
		{
			spdlog::warn("Digit string has more than one '.'");
			return false;
		}
	}
}

bool StringHelper::checkIntDigitString(std::string& digitString)
{
	spdlog::info("Trying to check int digit string: " + digitString);

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
			spdlog::info("Digit string: " + digitCleaned + " is ok.");
			return true;
		}
		else
		{
			spdlog::warn("Digit string: " + digitString + " is not a digit!");
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool StringHelper::checkDoubleDigitItem(QTableWidgetItem* &item)
{
	spdlog::info("Trying to check double digit item.");

	std::string itemText = item->text().toStdString();

	if (checkDoubleDigitString(itemText))
	{
		spdlog::info("Trying to set the corrected value: "+itemText);
		item->setText(QString::fromStdString(itemText));
		return true;
	}
	else
	{
		spdlog::warn("Digit item was not correct. Setting it blank.");
		item->setText(QString::fromStdString(""));
		return false;
	}
}

bool StringHelper::checkIntDigitItem(QTableWidgetItem* &item)
{
	spdlog::info("Trying to check digit item.");

	std::string itemText = item->text().toStdString();

	if (checkIntDigitString(itemText))
	{
		spdlog::info("Trying to set the corrected value: " + itemText);
		item->setText(QString::fromStdString(itemText));
		return true;
	}
	else
	{
		spdlog::warn("Digit item was not correct. Setting it blank.");
		item->setText(QString::fromStdString(""));
		return false;
	}
}
