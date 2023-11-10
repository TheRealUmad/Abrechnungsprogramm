#pragma once
#include <string>
#include <QWidget>

class ErrorHandler
{
private:
	ErrorHandler();
	~ErrorHandler();
public:
	static void ShowErrorMessageDialog(std::string message);
	static ErrorHandler* GetInstance(void);
	static void DeleteInstance(void);
	static void CreateInstance(QWidget* parent);
private:
	static QWidget* _mainWindow;
	static ErrorHandler* _instance;
	void Init(QWidget* parent);
};

