//== ВКЛЮЧЕНИЯ.
#include <QPushButton>
#include "set_proposed_bool_dialog.h"
#include "ui_set_proposed_bool_dialog.h"
#include "../Z-Hub/main-hub.h"

//== ФУНКЦИИ КЛАССОВ.
//== Класс диалога запроса да\нет.
// Конструктор.
Set_Proposed_Bool_Dialog::Set_Proposed_Bool_Dialog(char* p_chDialogCaption, char* p_chText, QWidget* p_parent) :
	QDialog(p_parent),
	p_ui(new Ui::Set_Proposed_Bool_Dialog)
{
	p_ui->setupUi(this);
	p_ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setText("Принять");
	p_ui->buttonBox->button(QDialogButtonBox::StandardButton::Cancel)->setText("Отмена");
	setWindowTitle(p_chDialogCaption);
	p_ui->label->setText(p_chText);
}

// Деструктор.
Set_Proposed_Bool_Dialog::~Set_Proposed_Bool_Dialog()
{
	delete p_ui;
}

// Принято.
void Set_Proposed_Bool_Dialog::accept()
{
	done(DIALOGS_ACCEPT);
}

// Отменено.
void Set_Proposed_Bool_Dialog::reject()
{
	done(DIALOGS_REJECT);
}
