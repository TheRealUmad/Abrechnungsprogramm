#pragma once
#ifndef SEARCHMEMBERDIALOG_H
#define SEARCHMEMBERDIALOG_H


#include <QLineEdit>
#include <QLabel>
#include <QDialog>
#include <QWidget>
#include <QStringList>

class SearchMemberDialog : public QDialog
{
	Q_OBJECT
public:
	SearchMemberDialog(QWidget* parent = nullptr);

	static QStringList getStrings(QWidget* parent, bool* ok = nullptr);

private:
	QList<QLineEdit*> fields;
};

#endif // SEARCHMEMBERDIALOG_H

