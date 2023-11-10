#pragma once
#include <string>

struct AppConfig
{
	std::string exePath;
	std::string itemCSVPath;
	std::string memberCSVPath;
	std::string consumationCSVPath;
	std::string saveFileDir;
	std::string saveFilePath;
	std::string pdfFileDir;
	std::string pdfFilePath;
	std::string logFileDir;

	int itemsInConsumeTableIndex;
};

class ConfigHandler
{
public:
	static void CreateInstance();
	static void DeleteInstance();
	static ConfigHandler* GetInstance(void);
	AppConfig* GetAppConfig();
	static std::string GetCurrentDateStr();
private:
	AppConfig* mConfig;
	static ConfigHandler* _instance;
	ConfigHandler(void);
	~ConfigHandler(void);
	void Init();
};


