//== ВКЛЮЧЕНИЯ.
#include <QPushButton>
#include "set-server-dialog.h"
#include "ui_set-server-dialog.h"

//== ДЕКЛАРАЦИИ СТАТИЧЕСКИХ ПЕРЕМЕННЫХ.
Set_Server_Dialog::IPPortPasswordStrings Set_Server_Dialog::oIPPortPasswordStrings;

//== ФУНКЦИИ КЛАССОВ.
//== Класс диалога добавления сервера.
// Конструктор.
Set_Server_Dialog::Set_Server_Dialog(char* p_chIP, char* p_Port, char* p_chPassword, QWidget* p_parent) :
	QDialog(p_parent),
	p_ui(new Ui::Set_Server_Dialog)
{
	p_ui->setupUi(this);
	p_ui->Accept_buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setText("Принять");
	p_ui->Accept_buttonBox->button(QDialogButtonBox::StandardButton::Cancel)->setText("Отмена");
	p_ui->IP_lineEdit->setText(p_chIP);
	p_ui->Port_lineEdit->setText(p_Port);
	p_ui->Password_lineEdit->setText(p_chPassword);
}

// Деструктор.
Set_Server_Dialog::~Set_Server_Dialog()
{
	delete p_ui;
}

// Принято.
void Set_Server_Dialog::accept()
{
	oIPPortPasswordStrings.strIP = p_ui->IP_lineEdit->text();
	oIPPortPasswordStrings.strPort = p_ui->Port_lineEdit->text();
	oIPPortPasswordStrings.strPassword = p_ui->Password_lineEdit->text();
	this->done(DIALOGS_ACCEPT);
}

// Отменено.
void Set_Server_Dialog::reject()
{
	this->done(DIALOGS_REJECT);
}
