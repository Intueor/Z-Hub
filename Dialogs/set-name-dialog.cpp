//== ВКЛЮЧЕНИЯ.
#include <QPushButton>
#include "set-name-dialog.h"
#include "ui_set-name-dialog.h"
#include "../Z-Hub/Server/net-hub-defs.h"
#include "main-hub.h"

//== ДЕКЛАРАЦИИ СТАТИЧЕСКИХ ПЕРЕМЕННЫХ.
char* Set_Name_Dialog::p_chNameInt;

//== ФУНКЦИИ КЛАССОВ.
//== Класс диалога установки имени.
// Конструктор.
Set_Name_Dialog::Set_Name_Dialog(char* p_chName, QWidget* p_parent) :
	QDialog(p_parent),
	p_ui(new Ui::Set_Name_Dialog)
{
	p_ui->setupUi(this);
	p_ui->Accept_buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setText("Принять");
	p_ui->Accept_buttonBox->button(QDialogButtonBox::StandardButton::Cancel)->setText("Отмена");
	p_ui->ServerName_lineEdit->setText(QString(p_chName));
	p_chNameInt = p_chName;
}

// Деструктор.
Set_Name_Dialog::~Set_Name_Dialog()
{
	delete p_ui;
}

// Принято.
void Set_Name_Dialog::accept()
{
	CopyStrArray((char*)p_ui->ServerName_lineEdit->text().toStdString().c_str(), p_chNameInt, SERVER_NAME_STR_LEN);
	this->done(DIALOGS_ACCEPT);
}

// Отменено.
void Set_Name_Dialog::reject()
{
	this->done(DIALOGS_REJECT);
}
