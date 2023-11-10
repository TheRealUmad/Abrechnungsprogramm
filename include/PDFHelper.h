#pragma once
#include <string>
#include <QTableWidget>
#include <QTextDocument>

class PDFHelper
{
public:
	static void printCalculationToPdf(QTableWidget* consumeTableWidget, std::string pdfFilePath);
	static void printDocumentToPdf(QTextDocument* doc, std::string pdfFilePath);
};

