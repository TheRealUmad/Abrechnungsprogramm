#include "PDFHelper.h"
#include <TableWidgetHelper.h>
#include <StringHelper.h>
#include <QTextDocument>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextTable>
#include <QPrinter>
#include <spdlog/spdlog.h>

void PDFHelper::printCalculationToPdf(QTableWidget* consumeTableWidget, std::string pdfFilePath)
{
	spdlog::info("Trying to print calculation to pdf: " + pdfFilePath);

	int columnCount = 0;

	QString headerTextAlias = "Coleurname";
	QString headerTextCarryover = "Uebertrag";
	QString headerTextDeposits = "Einzahlungen";
	QString headerTextTurnover = "Umsatz";
	QString headerTextDebt = "Schulden";
	QString headerTextCredit = "Guthaben";

	QList<QString> headerTexts;
	headerTexts.push_back(headerTextAlias);
	headerTexts.push_back(headerTextCarryover);
	headerTexts.push_back(headerTextDeposits);
	headerTexts.push_back(headerTextTurnover);
	headerTexts.push_back(headerTextDebt);
	headerTexts.push_back(headerTextCredit);

	int columnCarryoverPdf = 0;
	int columnDepositsPdf = 0;
	int columnTurnoverPdf = 0;
	int columnDebtPdf = 0;
	int columnCreditPdf = 0;

	std::map<int, int> columnTranslatorMap;

	int columnAlias = TableWidgetHelper::findColumnInTableHeader(consumeTableWidget, headerTextAlias);
	columnTranslatorMap.insert(std::pair<int, int>(columnCount, columnAlias));
	columnCount++;

	int columnCarryover = TableWidgetHelper::findColumnInTableHeader(consumeTableWidget, headerTextCarryover);
	columnCarryoverPdf = columnCount;
	columnTranslatorMap.insert(std::pair<int, int>(columnCount, columnCarryover));
	columnCount++;

	int columnDeposits = TableWidgetHelper::findColumnInTableHeader(consumeTableWidget, headerTextDeposits);
	columnDepositsPdf = columnCount;
	columnTranslatorMap.insert(std::pair<int, int>(columnCount, columnDeposits));
	columnCount++;

	int columnTurnover = TableWidgetHelper::findColumnInTableHeader(consumeTableWidget, headerTextTurnover);
	columnTurnoverPdf = columnCount;
	columnTranslatorMap.insert(std::pair<int, int>(columnCount, columnTurnover));
	columnCount++;

	int columnDebt = TableWidgetHelper::findColumnInTableHeader(consumeTableWidget, headerTextDebt);
	columnDebtPdf = columnCount;
	columnTranslatorMap.insert(std::pair<int, int>(columnCount, columnDebt));
	columnCount++;

	int columnCredit = TableWidgetHelper::findColumnInTableHeader(consumeTableWidget, headerTextCredit);
	columnCreditPdf = columnCount;
	columnTranslatorMap.insert(std::pair<int, int>(columnCount, columnCredit));
	columnCount++;

	if (columnAlias == -1 || columnCarryover == -1 || columnDeposits == -1 || columnTurnover == -1 || columnDebt == -1 || columnCredit == -1)
	{
		std::string message = "Could not find column indexes!";
		spdlog::error(message);
		throw std::exception(message.c_str());
	}

	//relevant columns for pdf: alias, carryover, deposits, turnover, debt, credit = 6

	QTextDocument* doc = new QTextDocument;
	//doc->setDocumentMargin(10);
	//doc->setPageSize(QPageSize::A4);
	QTextCursor cursor(doc);

	QTextCharFormat boldFormat;
	boldFormat.setFontWeight(QFont::Bold);

	QTextCharFormat textFormat;
	textFormat.setBackground(Qt::white);

	QTextCharFormat boldCellFormat;
	boldCellFormat.setFontWeight(QFont::Bold);
	boldCellFormat.setBackground(Qt::white);

	QTextCharFormat redFormat;
	redFormat.setBackground(Qt::red);

	QTextCharFormat alternateCellFormat;
	alternateCellFormat.setBackground(Qt::lightGray);

	QTextTableFormat tableFormat;
	tableFormat.setAlignment(Qt::AlignCenter);
	tableFormat.setBorderCollapse(true);
	QTextLength tableWidth = QTextLength(QTextLength::PercentageLength, 80);
	tableFormat.setWidth(tableWidth);
	tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);

	cursor.movePosition(QTextCursor::Start);

	int rowCount = consumeTableWidget->rowCount() + 2; // +2 because table header and sum row

	QTextTable* table = cursor.insertTable(rowCount, columnCount, tableFormat);

	spdlog::info("Generating document table header.");
	for (int column = 0; column < columnCount; column++)
	{
		QTextTableCell headerCell = table->cellAt(0, column);
		QTextCursor headerCellCursor = headerCell.firstCursorPosition();
		headerCellCursor.insertText(headerTexts.at(column), boldFormat);
	}

	double sumTurnover = 0.0;
	double sumDebt = 0.0;
	double sumCredit = 0.0;
	double sumDeposits = 0.0;

	spdlog::info("Generating document table items.");
	for (int row = 0; row < consumeTableWidget->rowCount(); row++)
	{
		QTextCharFormat cellFormat = row % 2 == 0 ? textFormat : alternateCellFormat;
		for (int column = 0; column < columnCount; column++)
		{
			int columnInConsumeTable = columnTranslatorMap.at(column);

			QTableWidgetItem* consumeTableItem = consumeTableWidget->item(row, columnInConsumeTable);

			if (consumeTableItem != nullptr && !consumeTableItem->text().isEmpty())
			{
				QTextTableCell cell = table->cellAt(row + 1, column);
				QTextCursor cellCursor = cell.firstCursorPosition();

				if (consumeTableItem->column() == columnDebt)
				{
					cell.setFormat(redFormat);
				}
				else
				{
					cell.setFormat(cellFormat);
				}

				std::string cellText = consumeTableItem->text().toStdString();
				StringHelper::checkDoubleDigitString(cellText);

				if (columnInConsumeTable == columnCredit)
					sumCredit += std::stod(cellText);
				else if (columnInConsumeTable == columnDebt)
					sumDebt += std::stod(cellText);
				else if (columnInConsumeTable == columnTurnover)
					sumTurnover += std::stod(cellText);
				else if (columnInConsumeTable == columnDeposits)
					sumDeposits += std::stod(cellText);

				spdlog::info("Inserting text: " + cellText + " from consume table into document.");
				cellCursor.insertText(QString::fromStdString(cellText));
			}
		}
	}

	//Create sum row
	QTextTableCell cell = table->cellAt(rowCount - 1, columnTurnoverPdf);
	QTextCursor cellCursor = cell.firstCursorPosition();
	cell.setFormat(boldCellFormat);
	std::string cellText = std::to_string(sumTurnover);
	StringHelper::checkDoubleDigitString(cellText);
	cellCursor.insertText(QString::fromStdString(cellText));

	cell = table->cellAt(rowCount - 1, columnCreditPdf);
	cellCursor = cell.firstCursorPosition();
	cell.setFormat(boldCellFormat);
	cellText = std::to_string(sumCredit);
	StringHelper::checkDoubleDigitString(cellText);
	cellCursor.insertText(QString::fromStdString(cellText));

	cell = table->cellAt(rowCount - 1, columnDebtPdf);
	cellCursor = cell.firstCursorPosition();
	cell.setFormat(boldCellFormat);
	cellText = std::to_string(sumDebt);
	StringHelper::checkDoubleDigitString(cellText);
	cellCursor.insertText(QString::fromStdString(cellText));

	cell = table->cellAt(rowCount - 1, columnDepositsPdf);
	cellCursor = cell.firstCursorPosition();
	cell.setFormat(boldCellFormat);
	cellText = std::to_string(sumDeposits);
	StringHelper::checkDoubleDigitString(cellText);
	cellCursor.insertText(QString::fromStdString(cellText));


	cursor.movePosition(QTextCursor::End);
	cursor.insertBlock();

	printDocumentToPdf(doc, pdfFilePath);
}

void PDFHelper::printDocumentToPdf(QTextDocument* doc, std::string pdfFilePath)
{
	spdlog::info("Trying to print document to pdf");

	QPrinter printer(QPrinter::HighResolution);
	printer.setPageOrientation(QPageLayout::Portrait);
	printer.setPageSize(QPageSize::A4);
	printer.setFullPage(true);
	printer.setOutputFileName(QString::fromStdString(pdfFilePath));
	doc->print(&printer);

	spdlog::info("Successfully printed document to pdf");
}
