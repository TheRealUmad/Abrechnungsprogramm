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
    bool addItemToTableWidget(QTableWidget* tableWidget, std::string item, int row, int column);
    bool writeTableWidgetToCSVfile(std::string csvFilePath, QTableWidget* tableWidget);
    bool isDigit(std::string str);
    
    bool findMemberByNameAndAlias(QTableWidget* tableWidget, std::string name, std::string alias, QTableWidgetItem* outName, QTableWidgetItem* outAlias);
    bool findItemInHeader(QTableWidget* tableWidget, std::string itemName, QTableWidgetItem* outItemName);

public slots:
    void onButtonAddItemClick();
    void onButtonAddMemberClick();
    void onItemTableWidgetItemsChanged();
    void onItemTableWidgetMemberChanged();
    void onItemTableWidgetConsumeChanged();
    void onTableWidgetItemsRowInserted();
    void onTableWidgetItemsRowDeleted();
private:
    Ui::MainWindow *ui;
    std::filesystem::path itemCSVPath = std::filesystem::current_path() / "Artikel.csv";
    std::filesystem::path memberCSVPath = std::filesystem::current_path() / "Mitglieder.csv";
    std::filesystem::path consumationCSVPath = std::filesystem::current_path() / "Abrechnung.csv";

};

#endif // MAINWINDOW_H
