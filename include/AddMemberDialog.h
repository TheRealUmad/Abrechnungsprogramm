#pragma once

#ifndef ADDMEMBERDIALOG_H
#define ADDMEMBERDIALOG_H


#include <QLineEdit>
#include <QLabel>
#include <QDialog>
#include <QWidget>
#include <QStringList>

class AddMemberDialog :  public QDialog
{
	Q_OBJECT
public:
	AddMemberDialog(QWidget* parent = nullptr);

	static QStringList getStrings(QWidget* parent, bool* ok = nullptr);

private:
	QList<QLineEdit*> fields;
};

#endif // ADDMEMBERDIALOG_H

