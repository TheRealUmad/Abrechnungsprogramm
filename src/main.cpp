#include <QtWidgets/QApplication>
#include <mainwindow.h>
#include <spdlog/spdlog.h>

int main(int argc, char* argv[])
{
	::ShowWindow(::GetConsoleWindow(), SW_HIDE);
	spdlog::info("Application started.");

	QApplication app(argc, argv);

	MainWindow mainWindow;

	int res;
	try
	{
	    mainWindow.show();
		res = app.exec();

		spdlog::info("Application closed.");

		return res;
	}
	catch (const std::exception& ex)
	{
		spdlog::critical("Program failed! Exception: "+(std::string)ex.what());
		return res;
	}
	catch (const std::string& ex)
	{
		spdlog::critical("Program failed! Exception: " + ex);
	}
	catch (...)
	{
		spdlog::critical("Program failed!");
	}
}