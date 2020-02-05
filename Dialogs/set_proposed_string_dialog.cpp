//== ВКЛЮЧЕНИЯ.
#include <QPushButton>
#include "set_proposed_string_dialog.h"
#include "ui_set_proposed_string_dialog.h"
#include "main-hub.h"

unsigned char Set_Proposed_String_Dialog::uchMaxLengthInt;
char* Set_Proposed_String_Dialog::p_chTextInt = nullptr;

//== ФУНКЦИИ КЛАССОВ.
//== Класс диалога установки предложенной строки.
// Конструктор.
Set_Proposed_String_Dialog::Set_Proposed_String_Dialog(char* p_chDialogCaption, char* p_chText, unsigned char uchMaxLength, QWidget* p_parent) :
	QDialog(p_parent),
	p_ui(new Ui::Set_Proposed_String_Dialog)
{
	p_ui->setupUi(this);
	p_ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setText("Принять");
	p_ui->buttonBox->button(QDialogButtonBox::StandardButton::Cancel)->setText("Отмена");
	this->setWindowTitle(p_chDialogCaption);
	p_ui->lineEdit->setText(p_chText);
	p_chTextInt = p_chText;
	uchMaxLengthInt = uchMaxLength;
}

// Деструктор.
Set_Proposed_String_Dialog::~Set_Proposed_String_Dialog()
{
	delete p_ui;
}

// Принято.
void Set_Proposed_String_Dialog::accept()
{
	CopyStrArray((char*)p_ui->lineEdit->text().toStdString().c_str(), p_chTextInt, uchMaxLengthInt);
	this->done(DIALOGS_ACCEPT);
}

// Отменено.
void Set_Proposed_String_Dialog::reject()
{
	this->done(DIALOGS_REJECT);
}
