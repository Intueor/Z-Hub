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
#include "Server/server.h"
#include "logger.h"
#include "parser-ext.h"
#include "main-hub.h"
#include "z-hub-defs.h"
#include "environment.h"
#include "p_buffer.h"

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

public:
	/// Конструктор.
	explicit MainWindow(QWidget* p_parent = nullptr);
							///< \param[in] p_parent Указатель на родительский виджет.
	/// Деструктор.
	~MainWindow();
	/// Процедуры при закрытии окна приложения.
	void closeEvent(QCloseEvent* p_Event);
							///< \param[in] p_Event Указатель на событие.
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
#ifdef LOG_LEVEL_2
	static void ClientRequestArrivedCallback(int iConnection, unsigned short ushRequest);
							///< \param[in] iConnection Номер соединения.
							///< \param[in] ushRequest Запрос клиента.
#else
	static void ClientRequestArrivedCallback(int, unsigned short);
#endif
	/// Для внешнего переключения чекбокса кнопки 'Схема'.
	static void UncheckSchemaCheckbox();

private:
	/// Процедуры запуска сервера.
	static bool ServerStartProcedures(NetHub::IPPortPassword& o_IPPortPassword);
							///< \param[in] o_IPPortPassword Ссылка на структуру с описанием IP, порта и пароля сервера.
							///< \return true, при удаче.
	/// Процедуры остановки сервера.
	static bool ServerStopProcedures();
							///< \return true, при удаче.
	/// Процедуры запуска среды.
	static bool EnvStartProcedures();
							///< \return true, при удаче.
	/// Процедуры остановки среды.
	static bool EnvStopProcedures();
							///< \return true, при удаче.
	/// Загрузка конфигурации сервера.
	static bool LoadServerConfig(NetHub::IPPortPassword& o_IPPortPassword, char* p_chServerName);
							///< \param[out] o_IPPortPassword Ссылка на структуру для заполнения полей IP, порта и пароля сервера.
							///< \param[out] p_chServerName Указатель на буфер с именем сервера для заполенения.
							///< \return true, при удаче.
	/// Сохранение конфигурации сервера.
	static bool SaveServerConfig();
							///< \return true, при удаче.
	/// Загрузка конфигурации среды.
	static bool LoadEnvConfig(char* p_chName);
							///< \param[out] p_chName Ссылка на строку для заполнения именем среды.
							///< \return true, при удаче.
	/// Сохранение конфигурации сервера.
	static bool SaveEnvConfig();
							///< \return true, при удаче.
	/// Установка текста строки статуса.
	static void SetStatusBarText(QString strMsg);
							///< \param[in] strMsg Строка с сообщением.
							///
private slots:
	// При переключении кнопки 'Запуск \ остановка сервера'.
	void on_action_StartStopServer_triggered(bool checked);
							///< \param[in] checked Позиция переключателя.
	/// При переключении кнопки 'Запуск при входе в приложение'.
	void on_action_StartOnLaunchApp_triggered(bool checked);
							///< \param[in] checked Позиция переключателя.
	/// При нажатии кнопки 'Имя сервера'.
	void on_action_ServerName_triggered();
	/// При нажатии кнопки 'Основные параметры сервера'.
	void on_action_ServerSettings_triggered();
	/// При нажатии кнопки 'Старт \ стоп среды'.
	void on_action_StartStopEnv_triggered(bool checked);
							///< \param[in] checked Позиция переключателя.
	// При нажатии кнопки 'Старт при запуске сервера'.
	void on_action_StartOnLaunchServer_triggered(bool checked);
							///< \param[in] checked Позиция переключателя.
	/// При нажатии кнопки 'Имя среды'.
	void on_action_ChangeEnv_triggered();
	// При нажатии кнопки 'Сохранение при выходе из приложения'.
	void on_action_Autosave_triggered(bool checked);
							///< \param[in] checked Позиция переключателя.
	// При нажатии кнопки 'Сохранение текущей среды'
	void on_action_SaveCurrent_triggered();

public:
	static unsigned char uchInitRes; ///< Результат инициализации.
	static Server* p_Server; ///< Указатель на объект сервера.

private:
	LOGDECL
	LOGDECL_PTHRD_INCLASS_ADD
	static Ui::MainWindow* p_ui; ///< Указатель на UI.
	static const char* cp_chUISettingsName; ///< Указатель на имя файла с установками UI.
	static QSettings* p_UISettings; ///< Указатель на строку установок UI.
	static QLabel* p_QLabelStatusBarText; ///< Указатель на объект метки статуса.
	static NetHub::IPPortPassword oIPPortPassword; ///< Структура со указателями на строки с установками сервера.
	static char m_chServerName[SERVER_NAME_STR_LEN]; ///< Буфер под строку с именем сервера.
	static char m_chIP[IP_STR_LEN]; ///< Буфер под строку адреса.
	static char m_chPort[PORT_STR_LEN]; ///< Буфер под строку порта.
	static char m_chPassword[AUTH_PASSWORD_STR_LEN]; ///< Буфер под строку пароля.
	static char m_chEnvName[ENV_NAME_LEN]; ///< Буфер под строку имени среды.
	static Environment* p_Environment; ///< Указатель на объект среды.
	static int iCurrentClientConnection; ///< Текущий подключённый клиент или NO_CLIENT.
	static bool bJustConnected; ///< Признак только что подключившегося клиента.
};

#endif // MAINWINDOW_H
