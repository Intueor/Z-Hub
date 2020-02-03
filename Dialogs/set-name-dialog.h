#ifndef SET_NAME_DIALOG_H
#define SET_NAME_DIALOG_H

//== ВКЛЮЧЕНИЯ.
#include <QDialog>

//== ПРОСТРАНСТВА ИМЁН.
namespace Ui {
	class Set_Name_Dialog;
}

//== КЛАССЫ.
/// Класс диалога установки имени.
class Set_Name_Dialog : public QDialog
{
	Q_OBJECT

public:
	/// Конструктор.
	explicit Set_Name_Dialog(char* p_chName, QWidget* p_parent = nullptr);
											///< \param[in,out] p_chName Указатель на массив строки с именем.
											///< \param[in] p_parent Указатель на родительский виджет.
	/// Деструктор.
	~Set_Name_Dialog();

private slots:
	/// Принято.
	void accept();
	/// Отменено.
	void reject();

private:
	Ui::Set_Name_Dialog *p_ui; //< Указатель на UI.
	static char* p_chNameInt; //< Внутренний указатель на массив строки имени.
};

#endif // SET_NAME_DIALOG_H
