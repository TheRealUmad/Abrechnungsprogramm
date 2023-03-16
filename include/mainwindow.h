#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once

#include <QMainWindow>
#include <filesystem>
#include <QTableWidget>
#include <CSVParser.h>
#include <Windows.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool fillItemList();
    bool fillMemberList();
    bool fillConsumationList();
    bool readCSVFile(std::string filename, std::vector<CSVRow>& outRows);

    inline bool fileExists(const std::string& name) {
        struct stat buffer;
        return (stat(name.c_str(), &buffer) == 0);
    }

    bool addRowToTableWidget(QTableWidget* tableWidget, std::vector<std::string> row);
    bool addRowToTableWidget(QTableWidget* tableWidget, CSVRow row);
    bool addRowToTableWidget(QTableWidget* tableWidget, CSVRow csvRow, std::vector<std::string> stringRow);
    bool deleteEmptyRowsOfTableWidget(QTableWidget* tableWidget);
    bool deleteEmptyColumnOfTableWidget(QTableWidget* tableWidget);
    bool addItemToTableWidget(QTableWidget* tableWidget, std::string itemText, int row, int column);
    bool addItemToTableWidget(QTableWidget* tableWidget, QTableWidgetItem* item, int row, int column);
    bool addItemToTableWidget(QTableWidget* tableWidget, QTableWidgetItem* item, int row, int column, bool copyItem);
    bool addItemToTableWidget(QTableWidget* tableWidget, QString itemText, int row, int column);
    bool createFile(std::string filePath);
    bool createDir(std::string dirPath);
    bool copyFile(std::string fromPath, std::string toPath);
    bool deleteFile(std::string filePath);
    //bool deleteItemFromTableWidget(QTableWidget* tableWidget, int row, int column);
    bool writeTableWidgetToCSVfile(std::string csvFilePath, QTableWidget* tableWidget);
    bool isDigit(std::string str);
    bool checkDoubleDigitString(std::string &digitString);
    bool checkIntDigitString(std::string& digitString);
    bool checkDoubleDigitItem(QTableWidgetItem* item);
    bool checkIntDigitItem(QTableWidgetItem* item);

    bool addItemToConsumeTableHeader(std::string itemName);
    bool updateConsumeTableHeader();
    bool generateConsumeTableHeader();

    bool memberHasDebtOrCredit(int rowInConsumeTable, double &outDebt, double &outCredit);
    
    bool findMemberByNameAndAlias(QTableWidget* tableWidget, QString firstName, QString lastName, QString alias, QTableWidgetItem* outFirstName, QTableWidgetItem* outLastName, QTableWidgetItem* outAlias);
    int findColumnInTableHeader(QTableWidget* tableWidget, QString headerText);
    bool deleteEmptyMemberFromTable(QTableWidget* tableWidget);
    bool deleteRowFromMemberAndConsumeTable(int row);
    bool restoreRowFromMemberOrConsumeTable(QTableWidget* tableWidgetToRestore, int rowToRestore);
    bool updateMemberInMemberAndConsumeTable(QTableWidgetItem* changedItem);

    bool calculateAndUpdateConsumeTable(QTableWidget* consumeTable,QTableWidget* itemTable, int columnCarryover, int columnDeposits, int columnTurnover, int columnDebt, int columnCredit, int columnItemsStart, int columnItemsEnd);
    void generateItemValueMap(QTableWidget* itemTable, QTableWidget* consumeTable, int columnItemsStart, int columnItemsEnd, std::map<int,double> &outItemValueMap);

    bool clearColumnOfTable(QTableWidget* tableWidget, int column);
    bool copyColumnOfTable(QTableWidget* tableWidget, int columnFrom, int columnTo);

public slots:
    void onButtonAddItemClick();
    void onButtonAddMemberClick();
    void onButtonNewCalculationClick();
    void onItemTableWidgetItemsChanged(QTableWidgetItem*);
    void onItemTableWidgetMemberChanged(QTableWidgetItem*);
    void onItemTableWidgetConsumeChanged(QTableWidgetItem*);
    void onItemTableWidgetConsumeDoubleClicked(QTableWidgetItem*);
    void onTableWidgetItemsRowInserted();
    void onTableWidgetItemsRowDeleted();
private:
    Ui::MainWindow *ui;

    std::string exePath;
    std::string itemCSVPath;
    std::string memberCSVPath;
    std::string consumationCSVPath;
    std::string saveFileDir;
    std::string saveFilePath;

    /*std::filesystem::path itemCSVPath = std::filesystem::current_path() / "Artikel.csv";
    std::filesystem::path memberCSVPath = std::filesystem::current_path() / "Mitglieder.csv";
    std::filesystem::path consumationCSVPath = std::filesystem::current_path() / "Abrechnung.csv";*/

    int itemsInConsumeTableIndex;
};

#endif // MAINWINDOW_H
