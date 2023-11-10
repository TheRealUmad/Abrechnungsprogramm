#include "FileHelper.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <spdlog/spdlog.h>

FileHelper::FileHelper()
{
}

FileHelper::~FileHelper()
{
}

bool FileHelper::createFile(std::string filePath)
{
	spdlog::info("Trying to create file: " + filePath);

	try
	{
		if (std::filesystem::is_regular_file(filePath))
		{
			spdlog::warn("File " + filePath + " already exists.");
			return true;
		}
		else
		{
			std::ofstream{ filePath };

			if (std::filesystem::is_regular_file(filePath))
			{
				spdlog::info("Successfully created file: " + filePath);
				return true;
			}
			else
			{
				std::string message = "Could not create file: " + filePath;
				spdlog::error(message);
				throw std::exception(message.c_str());
			}
		}
	}
	catch (const std::exception& ex)
	{
		std::string message = "Could not create file: " + filePath + "! Exception: " + ex.what();
		spdlog::error(message);
		throw std::exception(message.c_str());
	}
}

bool FileHelper::createDir(std::string dirPath)
{
	spdlog::info("Trying to create dir: " + dirPath);

	try
	{
		if (std::filesystem::is_directory(dirPath))
		{
			spdlog::warn("Directory: " + dirPath + " already exists.");
			return true;
		}
		else
		{
			if (std::filesystem::create_directories(dirPath))
			{
				spdlog::info("Successfully created directory: " + dirPath);
				return true;
			}
			else
			{
				std::string message = "Could not create directory: " + dirPath;
				spdlog::error(message);
				throw std::exception(message.c_str());
			}
		}
	}
	catch (const std::exception& ex)
	{
		std::string message = "Could not create dir: " + dirPath + "! Exception: " + ex.what();
		spdlog::error(message);
		throw std::exception(message.c_str());
	}
}

bool FileHelper::copyFile(std::string fromPath, std::string toPath, bool softFail)
{
	spdlog::info("Trying to copy file: " + fromPath + " to: " + toPath);

	try
	{
		if (std::filesystem::is_regular_file(fromPath))
		{
			if (std::filesystem::copy_file(fromPath, toPath))
			{
				spdlog::info("Successfully copied file: " + fromPath + " to: " + toPath);
				return true;
			}
			else
			{
				std::string message = "Could not copy file: " + fromPath + " to: " + toPath;
				spdlog::error(message);
				throw std::exception(message.c_str());
			}
		}
		else
		{
			std::string message = "File: " + fromPath + " does not exist! Could not copy file!";
			if (softFail)
			{
				spdlog::warn(message);
				return true;
			}
			else
			{
				spdlog::error(message);
				throw std::exception(message.c_str());
			}
		}
	}
	catch (const std::exception& ex)
	{
		std::string message = "Could not copy file: " + fromPath +" to: "+ toPath+ "! Exception: " + ex.what();
		spdlog::error(message);
		throw std::exception(message.c_str());
	}
}

bool FileHelper::deleteFile(std::string filePath, bool softFail)
{
	spdlog::info("Trying to delete file: " + filePath);

	try
	{
		if (std::filesystem::is_regular_file(filePath))
		{
			if (std::filesystem::remove(filePath))
			{
				spdlog::info("Successfully deleted file: " + filePath);
				return true;
			}
			else
			{
				std::string message = "Could not delete file: " + filePath;
				spdlog::error(message);
				throw std::exception(message.c_str());
			}
		}
		else
		{
			std::string message = "File: " + filePath + " does not exist! Could not delete file!";
			if (softFail)
			{
				spdlog::warn(message);
				return true;
			}
			else
			{
				spdlog::error(message);
				throw std::exception(message.c_str());
			}
		}
	}
	catch (const std::exception& ex)
	{
		std::string message = "Could not delete file: " + filePath + "! Exception: " + ex.what();
		spdlog::error(message);
		throw std::exception(message.c_str());
	}
}

bool FileHelper::checkDir(std::string dirPath, bool createIfNotExists)
{
	spdlog::info("Checking dir: " + dirPath);

	try
	{
		if (std::filesystem::is_directory(dirPath))
		{
			spdlog::info("Dir: " + dirPath + " exists.");
			return true;
		}
		else if (std::filesystem::is_regular_file(dirPath))
		{
			std::string message = "Path: " + dirPath + " is not a dir but a file!";
			spdlog::error(message);
			throw std::exception(message.c_str());
		}
		else if (createIfNotExists)
		{
			spdlog::info("Dir: " + dirPath + " does not exist yet. Trying to create it.");
			return createDir(dirPath);
		}
		else
		{
			std::string message = "Dir: " + dirPath + " does not exist!";
			spdlog::error(message);
			throw std::exception(message.c_str());
		}
	}
	catch (const std::exception& ex)
	{
		std::string message = "Could not check dir: " + dirPath + "! Exception: " + ex.what();
		spdlog::error(message);
		throw std::exception(message.c_str());
	}
}

bool FileHelper::checkFile(std::string filePath, bool createIfNotExists)
{
	spdlog::info("Checking file: " + filePath);

	try
	{
		if (std::filesystem::is_regular_file(filePath))
		{
			spdlog::info("File: " + filePath + " exists.");
			return true;
		}
		else if (std::filesystem::is_directory(filePath))
		{
			std::string message = "Path: " + filePath + " is not a file but a dir!";
			spdlog::error(message);
			throw std::exception(message.c_str());
		}
		else if (createIfNotExists)
		{
			spdlog::info("File: " + filePath + " does not exist yet. Trying to create it.");
			return createFile(filePath);
		}
		else
		{
			std::string message = "File: " + filePath + " does not exist!";
			spdlog::error(message);
			throw std::exception(message.c_str());
		}
	}
	catch (const std::exception& ex)
	{
		std::string message = "Could not check file: " + filePath + "! Exception: " + ex.what();
		spdlog::error(message);
		throw std::exception(message.c_str());
	}
}

void FileHelper::readCSVFile(std::string filename, std::vector<CSVRow>& outRows)
{
	spdlog::info("Trying to read csv file: " + filename);

	try
	{
		if (checkFile(filename))
		{
			std::ifstream file(filename);

			for (CSVRow csvRow : CSVRange(file))
			{
				outRows.push_back(csvRow);
			}

			file.close();
		}
	}
	catch (const std::exception& ex)
	{
		std::string message = "Could not read csv file: " + filename + "! Exception: " + ex.what();
		spdlog::error(message);
		throw std::exception(message.c_str());
	}
}
