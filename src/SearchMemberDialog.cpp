#include "SearchMemberDialog.h"
#include <QFormLayout>
#include <QDialogButtonBox>
#include <spdlog/spdlog.h>
#include <StringHelper.h>

SearchMemberDialog::SearchMemberDialog(QWidget* parent)
{
    setWindowTitle("Mitglied suchen");

    QFormLayout* layout = new QFormLayout(this);

    QLabel* lbl_memberAlias = new QLabel(QString("Coleurname:"), this);
    QLineEdit* tb_memberAlias = new QLineEdit(this);

    layout->addRow(lbl_memberAlias, tb_memberAlias);
    fields << tb_memberAlias;

    QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    buttonBox->addButton(QString("Suchen"), QDialogButtonBox::ButtonRole::AcceptRole);
    buttonBox->addButton(QDialogButtonBox::StandardButton::Cancel);
    layout->addWidget(buttonBox);

    bool conn = connect(buttonBox, &QDialogButtonBox::accepted, this, &SearchMemberDialog::accept);
    Q_ASSERT(conn);
    conn = connect(buttonBox, &QDialogButtonBox::rejected, this, &SearchMemberDialog::reject);
    Q_ASSERT(conn);
    conn = connect(tb_memberAlias, &QLineEdit::returnPressed, this, &SearchMemberDialog::accept);


    setLayout(layout);
    tb_memberAlias->setFocus();

}

QStringList SearchMemberDialog::getStrings(QWidget* parent, bool* ok)
{
    spdlog::info("Trying to show search member dialog.");

    SearchMemberDialog* dialog = new SearchMemberDialog(parent);

    QStringList list;

    const int ret = dialog->exec();

    if (ok)
        *ok = !!ret;

    spdlog::debug("Result of search member dialog: " + StringHelper::boolToString(ok));

    if (ret)
    {
        foreach(auto field, dialog->fields)
        {
            list << field->text();
            spdlog::debug("Text in field: " + field->text().toStdString());
        }
    }

    spdlog::info("Finished search member dialog.");

    return list;
}
