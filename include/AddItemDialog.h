#pragma once

#ifndef ADDITEMDIALOG_H
#define ADDITEMDIALOG_H


#include <QLineEdit>
#include <QLabel>
#include <QDialog>
#include <QWidget>
#include <QStringList>

class AddItemDialog : public QDialog
{
	Q_OBJECT
public:
	AddItemDialog(QWidget* parent = nullptr);

	static QStringList getStrings(QWidget* parent, bool* ok = nullptr);

private:
	QList<QLineEdit*> fields;
};

#endif // ADDITEMDIALOG_H