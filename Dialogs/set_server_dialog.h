#ifndef SET_SERVER_DIALOG_H
#define SET_SERVER_DIALOG_H

//== ВКЛЮЧЕНИЯ.
#include <QDialog>
#include "../Z-Hub/Server/net-hub-defs.h"
#include "../Z-Hub/main-hub.h"

//== ПРОСТРАНСТВА ИМЁН.
namespace Ui {
	class Set_Server_Dialog;
}

//== КЛАССЫ.
/// Класс диалога добавления сервера.
class Set_Server_Dialog : public QDialog
{
	Q_OBJECT

public:
	/// Структура со строками IP и порта.
	struct IPPortPasswordStrings
	{
		QString strIP; ///< Строка с введённым IP.
		QString strPort; ///< Строка с введённым портом.
		QString strPassword; ///< Строка с паролем.
	};

public:
	/// Конструктор.
	explicit Set_Server_Dialog(char* p_chIP, char* p_Port, char* p_chPassword, QWidget *p_parent = 0);
											///< \param[in] p_chIP - Указатель на массив строки для предустановки IP.
											///< \param[in] p_Port - Указатель на массив строки для предустановки порта.
											///< \param[in] p_chPassword - Указатель на массив строки для предустановки пароля.
											///< \param[in] p_parent - Указатель на родительский виджет.
	/// Деструктор.
	~Set_Server_Dialog();

private slots:
	/// Принято.
	void accept();
	/// Отменено.
	void reject();

public:
	static IPPortPasswordStrings oIPPortPasswordStrings; ///< Объект со строками IP и порта.

private:
	Ui::Set_Server_Dialog* p_ui; ///< Указатель на UI.
};

#endif // SET_SERVER_DIALOG_H
