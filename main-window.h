#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//== ВКЛЮЧЕНИЯ.
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#endif
#include <QMainWindow>
#include <QSettings>
#include <QLabel>
#include <QVector>
#include "logger.h"
#include "Server/server.h"
#include "parser-ext.h"
#include "main-hub.h"
#include "z-hub-defs.h"

//== МАКРОСЫ.
#define LOG_DIR_PATH			"../Z-Hub/logs/"

//== ПРОСТРАНСТВА ИМЁН.
namespace Ui {
	class MainWindow;
}

// Для избежания ошибки при доступе из другого потока.
Q_DECLARE_METATYPE(QVector<int>)
//

//== ПРЕД-ДЕКЛАРАЦИИ.

//== КЛАССЫ.
/// Класс главного окна.
class MainWindow : public QMainWindow
{
	Q_OBJECT

private:

public:
	static int iInitRes; ///< Результат инициализации.

public:
	/// Конструктор.
	explicit MainWindow(QWidget* p_parent = nullptr);
							///< \param[in] p_parent Указатель на родительский виджет.
	/// Деструктор.
	~MainWindow();
	/// Процедуры при закрытии окна приложения.
	void closeEvent(QCloseEvent* event);
							///< \param[in] event Указатель на событие.
	/// Кэлбэк обработки отслеживания статута клиентов.
	static void ClientStatusChangedCallback(int iConnection, bool bConnected);
							///< \param[in] iConnection Номер соединения.
							///< \param[in] bConnected Статус подключения.
	/// Кэлбэк обработки приходящих пакетов данных.
	static void ClientDataArrivedCallback(int iConnection, unsigned short ushType, void *p_ReceivedData, int iPocket);
							///< \param[in] iConnection Номер соединения.
							///< \param[in] ushType Тип принятого.
							///< \param[in] p_ReceivedData Указатель на принятый пакет.
							///< \param[in] iPocket Номер пакета для освобождения с ReleaseDataInPosition после использования.
	/// Кэлбэк обработки приходящих запросов.
	static void ClientRequestArrivedCallback(int iConnection, unsigned short ushRequest);
							///< \param[in] iConnection Номер соединения.
							///< \param[in] ushRequest Запрос клиента.

private:
	/// Загрузка каталога банов.
	static bool LoadBansCatalogue(vector<Server::IPBanUnit>& o_vec_IPBanUnits);
							///< \param[in] o_vec_IPBanUnits Ссылка на заполняемый вектор.
							///< \return true, при удаче.
	/// Процедуры запуска сервера.
	static bool ServerStartProcedures(NetHub::IPPortPassword& o_IPPortPassword, char* p_chServerName);
							///< \param[in] o_IPPortPassword Ссылка на структуру с описанием IP, порта и пароля сервера.
							///< \param[in] p_chServerName Указатель на буфер с именем сервера.
							///< \return true, при удаче.
	/// Загрузка конфигурации сервера.
	static bool LoadServerConfig(NetHub::IPPortPassword& o_IPPortPassword, char* p_chServerName);
							///< \param[in] o_IPPortPassword Ссылка на структуру для заполнения полей IP, порта и пароля сервера.
							///< \param[in] p_chServerName Указатель на буфер с именем сервера для заполенения.
							///< \return true, при удаче.
	/// Сохранение конфигурации сервера.
	static bool SaveServerConfig();
							///< \return true, при удаче.
	/// Процедуры остановки сервера.
	static bool ServerStopProcedures();
							///< \return true, при удаче.
private slots:

private:
	static const QString cstrStatusReady; ///< Строка состояния.
	static const QString cstrStatusStopServer; ///< Строка состояния.
	static const QString cstrStatusShutdown; ///< Строка состояния.
	LOGDECL
	LOGDECL_PTHRD_INCLASS_ADD
	static Ui::MainWindow *p_ui; ///< Указатель на UI.
	static const char* cp_chUISettingsName; ///< Указатель на имя файла с установками UI.
	static QSettings* p_UISettings; ///< Указатель на строку установок UI.
	static QLabel* p_QLabelStatusBarText; ///< Указатель на объект метки статуса.
	static Server* p_Server; ///< Ссылка на объект сервера.
	static vector<Server::IPBanUnit> vec_IPBanUnits; ///< Список банов по адресам.
	static NetHub::IPPortPassword oIPPortPassword; ///< Структура со строками с установками сервера.
	static char m_chServerName[SERVER_NAME_STR_LEN]; ///< Буфер под строку с именем сервера.
	static char m_chIP[IP_STR_LEN]; ///< Буфер под строку адреса.
	static char m_chPort[PORT_STR_LEN]; ///< Буфер под строку порта.
	static char m_chPassword[AUTH_PASSWORD_STR_LEN]; ///< Буфер под строку пароля.
};

#endif // MAINWINDOW_H

