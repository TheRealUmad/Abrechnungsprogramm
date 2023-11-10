#include "ErrorHandler.h"
#include <QMessageBox>
#include <iostream>
#include <spdlog/spdlog.h>

ErrorHandler* ErrorHandler::_instance = nullptr;
QWidget* ErrorHandler::_mainWindow = nullptr;

ErrorHandler::ErrorHandler()
{
}

ErrorHandler::~ErrorHandler()
{
}

void ErrorHandler::ShowErrorMessageDialog(std::string message)
{
	auto ret = QMessageBox::critical(_mainWindow, "Error", QString::fromStdString(message), QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Ignore);
	if (ret == QMessageBox::Ok)
	{
		std::string message = "Error message aknowleged by user! Aborting program!";
		spdlog::critical(message);
		throw std::exception(message.c_str());
	}
	else if (ret == QMessageBox::Ignore)
	{
		spdlog::warn("Error message ignored by user! Continuing program.");
	}
}

ErrorHandler* ErrorHandler::GetInstance(void)
{
	if (_instance != nullptr)
		return _instance;
	else
	{
		std::string message = "No instance of ErrorHandler was created!";
		spdlog::error(message);
		throw std::exception(message.c_str());
	}
}

void ErrorHandler::DeleteInstance(void)
{
	if (_instance)
	{
		delete _instance;
		_instance = nullptr;
		_mainWindow = nullptr;
	}
}

void ErrorHandler::CreateInstance(QWidget* parent)
{
	if (_instance == nullptr)
	{
		_instance = new ErrorHandler();
		_instance->Init(parent);
	}
}

void ErrorHandler::Init(QWidget* parent)
{
	_mainWindow = parent;
}
