#include "ConfigHandler.h"
#include <spdlog/spdlog.h>
#include <filesystem>
#include <FileHelper.h>
#include <exception>
#include <chrono>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif // _WIN32


ConfigHandler* ConfigHandler::_instance = nullptr;

void ConfigHandler::CreateInstance()
{
	if (_instance == nullptr)
	{
		spdlog::info("Creating ConfigHandler instance");
		_instance = new ConfigHandler();
		_instance->Init();
	}
}

void ConfigHandler::DeleteInstance()
{
	if (_instance)
	{
		delete _instance;
		_instance = nullptr;
	}
}

ConfigHandler* ConfigHandler::GetInstance(void)
{
	if (_instance == nullptr)
		ConfigHandler::CreateInstance();

	return _instance;
}

AppConfig* ConfigHandler::GetAppConfig()
{
	return mConfig;
}

std::string ConfigHandler::GetCurrentDateStr()
{
	// Get the current time point
	auto currentTime = std::chrono::system_clock::now();

	// Convert the time point to a time_t object
	std::time_t currentTime_t = std::chrono::system_clock::to_time_t(currentTime);

	// Convert the time_t to a tm structure
	std::tm* now = std::localtime(&currentTime_t);

	// Format the date as a string
	std::stringstream ss;
	ss << std::put_time(now, "%d.%m.%Y");
	return ss.str();
}

ConfigHandler::ConfigHandler(void)
{

}

ConfigHandler::~ConfigHandler(void)
{
	delete mConfig;
}



void ConfigHandler::Init()
{
	spdlog::info("Init config handler");

	mConfig = new AppConfig();

	char* pt;

#ifdef _WIN32
	char szFilePath[MAX_PATH + 1] = { 0 };
	GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
	pt = szFilePath;
#else
	char szFilePath[PATH_MAX];
	ssize_t len = readlink("/proc/self/exe", szFilePath, sizeof(szFilePath) - 1);
	if (len != -1)
	{
		szFilePath[len] = '\0';
	}
	pt = szFilePath;
#endif // _WIN32

	(strrchr(pt, '\\'))[0] = 0;

	mConfig->exePath = pt;
	mConfig->consumationCSVPath = mConfig->exePath + "/Abrechnung.csv";
	mConfig->memberCSVPath = mConfig->exePath + "/Mitglieder.csv";
	mConfig->itemCSVPath = mConfig->exePath + "/Artikel.csv";
	mConfig->saveFileDir = mConfig->exePath + "/Sicherungskopien";
	mConfig->saveFilePath = mConfig->saveFileDir + "/Artikel_SicherungsKopie.csv";
	mConfig->logFileDir = mConfig->exePath + "/Logs";
	mConfig->pdfFileDir = mConfig->exePath + "/PDFs";

	mConfig->pdfFilePath = mConfig->pdfFileDir + "/Abrechnung-"+GetCurrentDateStr() + ".pdf";

	bool checkOk = true;

	checkOk = checkOk && FileHelper::checkFile(mConfig->consumationCSVPath);
	checkOk = checkOk && FileHelper::checkFile(mConfig->memberCSVPath);
	checkOk = checkOk && FileHelper::checkFile(mConfig->itemCSVPath);

	checkOk = checkOk && FileHelper::checkDir(mConfig->saveFileDir);
	checkOk = checkOk && FileHelper::checkFile(mConfig->saveFilePath);

	checkOk = checkOk && FileHelper::checkDir(mConfig->logFileDir);

	checkOk = checkOk && FileHelper::checkDir(mConfig->pdfFileDir);
	checkOk = checkOk && FileHelper::checkFile(mConfig->pdfFilePath);

	if (checkOk)
		spdlog::info("All dirs and files are ok.");
	else
	{
		std::string message = "Check on dirs and/or files failed!";
		spdlog::error(message);
		throw std::exception(message.c_str());
	}

}
