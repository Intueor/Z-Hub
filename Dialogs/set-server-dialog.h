#ifndef SET_SERVER_DIALOG_H
#define SET_SERVER_DIALOG_H

//== ВКЛЮЧЕНИЯ.
#include "../Z-Server/main-hub.h"
#include "../Z-Server/Server/protocol.h"
#include <QDialog>

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
	/// Структура комбинации числовых данных адреса, порта и строки с паролем.
	struct NumAddrPassw
	{
		NumericAddress oNumericAddress; // Структура с данными и статусом операции.
		char m_chPassword[AUTH_PASSWORD_STR_LEN]; // Массив строки пароля.
	};
	/// Структура со строками IP и порта.
	struct IPPortStrings
	{
		QString strIP; ///< Строка с введённым IP.
		QString strPort; ///< Строка с введённым портом.
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
	/// Получение ссылки на принятые данные IP, порта и пароля.
	static NumAddrPassw &GetReceivedValues();
											///< \return Ссылка на структуру с данными и статусом операции.
	// Получение ссылки на принятые строки IP и порта.
	static IPPortStrings &GetReceivedIPPortStrings();
											///< \return Ссылка на структуру со строками.

private slots:
	/// Принято.
	void accept();
	/// Отменено.
	void reject();

private:
	static IPPortStrings oIPPortStrings; ///< Объект со строками IP и порта.
	Ui::Set_Server_Dialog* p_ui; ///< Указатель на UI.
	static NumAddrPassw oNumAddrPassw; ///< Объект структуры комбинации числовых данных адреса, порта и строки с паролем.
};

#endif // SET_SERVER_DIALOG_H
