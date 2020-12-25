#ifndef MESSAGE_DIALOG_H
#define MESSAGE_DIALOG_H

//== ВКЛЮЧЕНИЯ.
#ifdef WIN32
#include <QIcon> // Баг нового QT Desinger.
#endif
#include <QDialog>

//== ПРОСТРАНСТВА ИМЁН.
namespace Ui {
	class Message_Dialog;
}

//== КЛАССЫ.
/// Класс диалога сообщения об ошибке.
class Message_Dialog : public QDialog
{
	Q_OBJECT

public:
	/// Конструктор.
	explicit Message_Dialog(const char* p_chCaption, const char* p_chMessage, QWidget* p_parent = nullptr);
								///< \param[in] p_chCaption Указатель на массив строки с заголовком.
								///< \param[in] p_chMessage Указатель на массив строки с сообщением.
								///< \param[in] p_parent Указатель на родительский виджет.
	/// Деструктор.
	~Message_Dialog();

private:
	Ui::Message_Dialog* p_ui; ///< Указатель на интерфейс.
};

#endif // MESSAGE_DIALOG_H
