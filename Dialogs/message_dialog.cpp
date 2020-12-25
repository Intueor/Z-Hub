//== ВКЛЮЧЕНИЯ.
#include <QPushButton>
#include "message_dialog.h"
#include "ui_message_dialog.h"

//== ФУНКЦИИ КЛАССОВ.
//== Класс диалога сообщения об ошибке.
// Конструктор.
Message_Dialog::Message_Dialog(const char* p_chCaption, const char* p_chMessage, QWidget* p_parent) :
	QDialog(p_parent),
	p_ui(new Ui::Message_Dialog)
{
	p_ui->setupUi(this);
	setWindowTitle(QString(p_chCaption));
	p_ui->label->setText(QString(p_chMessage));
	p_ui->buttonBox->button(QDialogButtonBox::StandardButton::Close)->setText("Закрыть");
}

// Деструктор.
Message_Dialog::~Message_Dialog()
{
	delete p_ui;
}
