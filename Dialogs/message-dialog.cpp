//== ВКЛЮЧЕНИЯ.
#include <QPushButton>
#include "message-dialog.h"
#include "ui_message-dialog.h"

//== ФУНКЦИИ КЛАССОВ.
//== Класс диалога сообщения об ошибке.
// Конструктор.
Message_Dialog::Message_Dialog(const char* p_chCaption, const char* p_chMessage, QWidget* p_parent) :
	QDialog(p_parent),
	p_ui(new Ui::Message_Dialog)
{
	p_ui->setupUi(this);
	p_ui->retranslateUi(this);
	this->setWindowTitle(QString(p_chCaption));
	p_ui->label->setText(QString(p_chMessage));
	p_ui->buttonBox->button(QDialogButtonBox::StandardButton::Close)->setText(tr("Закрыть"));
}

// Деструктор.
Message_Dialog::~Message_Dialog()
{
	delete p_ui;
}
