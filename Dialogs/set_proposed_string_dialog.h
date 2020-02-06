#ifndef SET_PROPOSED_STRING_DIALOG_H
#define SET_PROPOSED_STRING_DIALOG_H

//== ВКЛЮЧЕНИЯ.
#include <QDialog>

//== ПРОСТРАНСТВА ИМЁН.
namespace Ui {
class Set_Proposed_String_Dialog;
}

//== КЛАССЫ.
/// Класс диалога установки предложенной строки.
class Set_Proposed_String_Dialog : public QDialog
{
	Q_OBJECT

public:
	/// Конструктор.
	explicit Set_Proposed_String_Dialog(char* p_chDialogCaption, char* p_chText, unsigned char uchMaxLength, QWidget* p_parent = nullptr);
								///< \param[in] p_chDialogCaption Указатель на массив строки с именем.
								///< \param[in,out] p_chText Указатель на массив строки для начального отображения и последующего заполнения.
								///< \param[in] uchMaxLength Максимальная длина строки.
								///< \param[in] p_parent Указатель на родительский виджет.
	/// Деструктор.
	~Set_Proposed_String_Dialog();

private slots:
	/// Принято.
	void accept();
	/// Отменено.
	void reject();

private:
	Ui::Set_Proposed_String_Dialog* p_ui;
	static char* p_chTextInt;
	static unsigned char uchMaxLengthInt;
};

#endif // SET_PROPOSED_STRING_DIALOG_H
