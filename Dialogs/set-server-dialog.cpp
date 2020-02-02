//== ВКЛЮЧЕНИЯ.
#include <QPushButton>
#include "set-server-dialog.h"
#include "ui_set-server-dialog.h"

//== ДЕКЛАРАЦИИ СТАТИЧЕСКИХ ПЕРЕМЕННЫХ.
Set_Server_Dialog::NumAddrPassw Set_Server_Dialog::oNumAddrPassw;
Set_Server_Dialog::IPPortStrings Set_Server_Dialog::oIPPortStrings;

//== ФУНКЦИИ КЛАССОВ.
//== Класс диалога добавления сервера.
// Конструктор.
Set_Server_Dialog::Set_Server_Dialog(char* p_chIP, char* p_Port, char* p_chPassword, QWidget* p_parent) :
	QDialog(p_parent),
	p_ui(new Ui::Set_Server_Dialog)
{
	p_ui->setupUi(this);
	p_ui->retranslateUi(this);
	p_ui->Accept_buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setText(tr("Принять"));
	p_ui->Accept_buttonBox->button(QDialogButtonBox::StandardButton::Cancel)->setText(tr("Отмена"));
	p_ui->IP_lineEdit->setText(p_chIP);
	p_ui->Port_lineEdit->setText(p_Port);
	p_ui->Password_lineEdit->setText(p_chPassword);
	memset(&oNumAddrPassw, 0, sizeof(NumAddrPassw));
}

// Деструктор.
Set_Server_Dialog::~Set_Server_Dialog()
{
	delete p_ui;
}

// Принято.
void Set_Server_Dialog::accept()
{
	oIPPortStrings.strIP = p_ui->IP_lineEdit->text();
	oIPPortStrings.strPort = p_ui->Port_lineEdit->text();
	FillNumericStructWithIPPortStrs(oNumAddrPassw.oNumericAddress, oIPPortStrings.strIP, oIPPortStrings.strPort);
	CopyStrArray((char*)p_ui->Password_lineEdit->text().toStdString().c_str(), oNumAddrPassw.m_chPassword, AUTH_PASSWORD_STR_LEN);
	this->done(DIALOGS_ACCEPT);
}

// Отменено.
void Set_Server_Dialog::reject()
{
	this->done(DIALOGS_REJECT);
}

// Получение ссылки на принятые данные IP, порта и пароля.
Set_Server_Dialog::NumAddrPassw& Set_Server_Dialog::GetReceivedValues()
{
	return oNumAddrPassw;
}

// Получение ссылки на принятые строки IP и порта.
Set_Server_Dialog::IPPortStrings& Set_Server_Dialog::GetReceivedIPPortStrings()
{
	return oIPPortStrings;
}
