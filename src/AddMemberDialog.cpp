#include "AddMemberDialog.h"

#include <QLabel>
#include <QLineEdit>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QStringList>
#include <spdlog/spdlog.h>
#include <StringHelper.h>


AddMemberDialog::AddMemberDialog(QWidget* parent) : QDialog(parent)
{
	setWindowTitle("Neues Mitglied");

	QFormLayout* layout = new QFormLayout(this);

	QLabel* lbl_memberAlias = new QLabel(QString("Coleurname:"), this);
	QLineEdit* tb_memberAlias = new QLineEdit(this);

	layout->addRow(lbl_memberAlias, tb_memberAlias);
	fields << tb_memberAlias;

	QLabel* lbl_memberFirstName = new QLabel(QString("Vorname:"), this);
	QLineEdit* tb_memberFirstName = new QLineEdit(this);

	layout->addRow(lbl_memberFirstName, tb_memberFirstName);
	fields << tb_memberFirstName;

	QLabel* lbl_memberLastName = new QLabel(QString("Nachname:"), this);
	QLineEdit* tb_memberLastName = new QLineEdit(this);

	layout->addRow(lbl_memberLastName, tb_memberLastName);
	fields << tb_memberLastName;

	QDialogButtonBox* buttonBox = new QDialogButtonBox
	(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
		Qt::Horizontal, this);
	layout->addWidget(buttonBox);

	bool conn = connect(buttonBox, &QDialogButtonBox::accepted,
		this, &AddMemberDialog::accept);
	Q_ASSERT(conn);
	conn = connect(buttonBox, &QDialogButtonBox::rejected,
		this, &AddMemberDialog::reject);
	Q_ASSERT(conn);

	setLayout(layout);

}

QStringList AddMemberDialog::getStrings(QWidget* parent, bool* ok)
{
	spdlog::info("Trying to show add member dialog.");

	AddMemberDialog* dialog = new AddMemberDialog(parent);

	QStringList list;

	const int ret = dialog->exec();
	if (ok)
		*ok = !!ret;

	spdlog::debug("Result of add member dialog: " + StringHelper::boolToString(ok));

	if (ret) {
		foreach(auto field, dialog->fields) {
			list << field->text();
			spdlog::debug("Text in field: " + field->text().toStdString());
		}
	}

	spdlog::info("Finished add member dialog.");

	return list;
}