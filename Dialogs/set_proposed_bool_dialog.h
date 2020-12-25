#ifndef SET_PROPOSED_BOOL_DIALOG_H
#define SET_PROPOSED_BOOL_DIALOG_H

//== ВКЛЮЧЕНИЯ.
#include <QDialog>

//== ПРОСТРАНСТВА ИМЁН.
namespace Ui {
class Set_Proposed_Bool_Dialog;
}

//== КЛАССЫ.
/// Класс диалога запроса да\нет.
class Set_Proposed_Bool_Dialog : public QDialog
{
	Q_OBJECT

public:
	/// Конструктор.
	explicit Set_Proposed_Bool_Dialog(char* p_chDialogCaption, char* p_chText, QWidget* p_parent = nullptr);
											///< \param[in] p_chDialogCaption Указатель на массив строки с именем.
											///< \param[in,out] p_chText Указатель на массив строки выбора.
											///< \param[in] p_parent Указатель на родительский виджет.
	/// Деструктор.
	~Set_Proposed_Bool_Dialog();

private slots:
	/// Принято.
	void accept();
	/// Отменено.
	void reject();

private:
	Ui::Set_Proposed_Bool_Dialog* p_ui; ///< Указатель на интерфейс.
};

#endif // SET_PROPOSED_BOOL_DIALOG_H
