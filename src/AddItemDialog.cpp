#include "AddItemDialog.h"

#include <QLabel>
#include <QLineEdit>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QStringList>


AddItemDialog::AddItemDialog(QWidget* parent) : QDialog(parent)
{
	setWindowTitle("Neuer Artikel");

	QFormLayout* layout = new QFormLayout(this);

	QLabel* lbl_ItemName = new QLabel(QString("Artikelname:"),this);
	QLineEdit* tb_ItemName = new QLineEdit(this);

	layout->addRow(lbl_ItemName, tb_ItemName);
	fields << tb_ItemName;

	QLabel* lbl_ItemValue = new QLabel(QString("Preis:"), this);
	QLineEdit* tb_ItemValue = new QLineEdit(this);

	layout->addRow(lbl_ItemValue, tb_ItemValue);
	fields << tb_ItemValue;

	QDialogButtonBox* buttonBox = new QDialogButtonBox
	(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
		Qt::Horizontal, this);
	layout->addWidget(buttonBox);

	bool conn = connect(buttonBox, &QDialogButtonBox::accepted,
		this, &AddItemDialog::accept);
	Q_ASSERT(conn);
	conn = connect(buttonBox, &QDialogButtonBox::rejected,
		this, &AddItemDialog::reject);
	Q_ASSERT(conn);

	setLayout(layout);

}

QStringList AddItemDialog::getStrings(QWidget* parent, bool* ok)
{
	AddItemDialog* dialog = new AddItemDialog(parent);

	QStringList list;

	const int ret = dialog->exec();
	if (ok)
		*ok = !!ret;

	if (ret) {
		foreach(auto field, dialog->fields) {
			list << field->text();
		}
	}

	return list;
}