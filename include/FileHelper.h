#pragma once
#include <string>
#include <CSVParser.h>

class FileHelper
{
public:
	FileHelper();
	~FileHelper();
	static bool createFile(std::string filePath);
	static bool createDir(std::string dirPath);
	static bool copyFile(std::string fromPath, std::string toPath, bool softFail = false);
	static bool deleteFile(std::string filePath, bool softFail = false);
	//inline static bool fileExists(const std::string& name) {
	//	struct stat buffer;
	//	return (stat(name.c_str(), &buffer) == 0);
	//}
	static bool checkDir(std::string dirPath, bool createIfNotExists = true);
	static bool checkFile(std::string filePath, bool createIfNotExists = true);
	static void readCSVFile(std::string filename, std::vector<CSVRow>& outRows);
};

