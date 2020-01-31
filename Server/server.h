#ifndef SERVER_H
#define SERVER_H

//== ВКЛЮЧЕНИЯ.
#ifdef WIN32
#define _WINSOCKAPI_
#define _TIMESPEC_DEFINED
#endif
#ifndef WIN32
#include <algorithm>
#endif
#include <signal.h>
#include <vector>
#ifndef WIN32
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
//#include <X11/Xlib.h> // (console)
//#include <X11/Xutil.h> // (console)
#include <termios.h>
#else
#include <WS2tcpip.h>
#endif
#include "net-hub.h"
#include "proto-parser.h"

//== ОПРЕДЕЛЕНИЯ ТИПОВ.
typedef void (*CBClientRequestArrived)(int iConnection, unsigned short ushRequest);
typedef void (*CBClientDataArrived)(int iConnection, unsigned short ushType, void *p_ReceivedData, int iPocket);
typedef void (*CBClientStatusChanged)(int iConnection, bool bConnected);

//== ПРОСТРАНСТВА.
using namespace std;

//== КЛАССЫ.
/// Класс сервера.
class Server
{
public:
	/// Структура бана по адресу.
	struct IPBanUnit
	{
		char m_chIP[IP_STR_LEN]; ///< Адрес пользователя.
	};

private:
	/// Структура описания данных потока соединения.
	struct ConversationThreadData
	{
		bool bInUse; ///< Флаг использования в соотв. потоке.
		pthread_t p_Thread; ///< Указатель на рабочий поток.
		NetHub::ReceivedData mReceivedPockets[S_MAX_STORED_POCKETS]; ///< Массив принятых пакетов.
		NetHub::ConnectionData oConnectionData; ///< Данные по соединению.
		char m_chData[MAX_DATA]; ///< Принятый пакет.
		int iCurrentFreePocket; ///< Текущий свободный пакет в массиве.
		bool bFullOnServer; ///< Флаг переполнения буфера на сервере.
		bool bFullOnClient; ///< Флаг переполнения буфера на клиенте.
		bool bSecured; ///< Флаг защищённого соединения.
		bool bKick; ///< Флаг команды на выброс клиента.
		NetHub oLocalNetHub; ///< Локальный буфер с набором функций для потока.
	};

private:
	static PServerName oPServerName; ///< Объект структуры для заполнения именем сервера.
	static bool bServerAlive; ///< Признак жизни потока сервера.
	static bool bExitSignal; ///< Сигнал на общее завершение.
	static pthread_mutex_t ptConnMutex; ///< Инициализатор мьютекса соединений.
	static int iListener; ///< Сокет приёмника.
	static bool bRequestNewConn; ///< Сигнал запроса нового соединения.
	static ConversationThreadData mThreadDadas[MAX_CONN]; ///< Массив структур описания потоков соединений.
	static bool bListenerAlive; ///< Признак жизни потока приёмника.
	static pthread_t ServerThr; ///< Идентификатор потока сервера.
	static CBClientRequestArrived pf_CBClientRequestArrived; ///< Указатель на кэлбэк приёма запросов.
	static CBClientDataArrived pf_CBClientDataArrived; ///< Указатель на кэлбэк приёма пакетов.
	static CBClientStatusChanged pf_CBClientStatusChanged; ///< Указатель на кэлбэк отслеживания статута клиентов.
	static pthread_t p_ThreadOverrunned; ///< Указатель на рабочий поток при переполнении.
	static vector<IPBanUnit>* p_vIPBansInt; ///< Внутреннй указатель на список с банами по IP.
	static NetHub::IPPortPassword* p_IPPortPasswordInt; ///< Внутреннй указатель на структуру со строками с установками сервера.
	LOGDECL
	LOGDECL_PTHRD_INCLASS_ADD

public:
	/// Конструктор.
	Server(pthread_mutex_t ptLogMutex, vector<IPBanUnit>* p_vIPBans = nullptr);
								///< \param[in] ptLogMutex Инициализатор мьютекса лога.
								///< \param[in] p_vIPBans Установка внутреннего указателя на список с банами по IP.
	/// Деструктор.
	~Server();
	/// Запрос запуска сервера.
	static bool Start(NetHub::IPPortPassword* p_IPPortPassword, char* p_chServerName);
								///< \param[in] p_IPPortPassword ///< Указатель на строку с путём к установкам сервера.
								///< \param[in] p_chServerName Указатель на строку с именем сервера.
								///< \return true - при удаче.
	/// Запрос остановки сервера.
	static bool Stop();
								///< \return true - при удаче.
	/// Запрос о действующем сервере.
	static bool CheckServerAlive();
				///< \return true - готов.
	/// Добавление пакета в буфер отправки на выбранное соединение.
	static bool AddPocketToOutputBuffer(int iConnection, unsigned short ushCommand, char *p_chBuffer = nullptr,
										int iLength = 0, bool bTryLock = true);
								///< \param[in] iConnection Номер соединения.
								///< \param[in] ushCommand Команда, которая будет задана в начале пакета.
								///< \param[in] p_chBuffer Указатель на буффер с данными.
								///< \param[in] iLength Длина пакета в байтах.
								///< \param[in] bTryLock Установить в false при использовании внутри кэлбэков.
								///< \return true, при удаче.
	/// Сброс указателя позиции в буфере пакетов выбранного соединения.
	static bool ResetPocketsBufferPositionPointer(int iConnection, bool bTryLock = true);
								///< \param[in] iConnection Номер соединения.
								///< \param[in] bTryLock Установить в false при использовании внутри кэлбэков.
								///< \return true, при удаче.
	/// Отправка пакета клиенту на текущее выбранное соединение немедленно.
	static bool SendToClientImmediately(int iConnection, unsigned short ushCommand, char* p_chBuffer, int iLength,
										bool bResetPointer = true, bool bTryLock = true);
								///< \param[in] iConnection Номер соединения.
								///< \param[in] ushCommand Код команды протокола.
								///< \param[in] p_chBuffer Указатель на буфер с данными для отправки.
								///< \param[in] iLength Длина буфера в байтах.
								///< \param[in] bResetPointer Сбрасывать ли указатель на начало буфера (для нового заполнения).
								///< \param[in] bTryLock Установить в false при использовании внутри кэлбэков.
								///< \return true - при удаче.
	/// Отправка буфера клиенту на текущее выбранное соединение.
	static bool SendBufferToClient(int iConnection, bool bResetPointer = true, bool bTryLock = true);
								///< \param[in] iConnection Номер соединения.
								///< \param[in] bResetPointer Сбрасывать ли указатель на начало буфера (для нового заполнения).
								///< \param[in] bTryLock Установить в false при использовании внутри кэлбэков.
								///< \return true - при удаче.
	/// Установка указателя кэлбэка изменения статуса подключения клиента.;
	/// Внутри кэлбэка ОБЯЗАТЕЛЬНО в вызовах с возможностью установки bTryLock - ставить false, кэлбэки и так под локом.
	static void SetClientRequestArrivedCB(CBClientRequestArrived pf_CBClientRequestArrivedIn);
								///< \param[in] pf_CBClientRequestArrivedIn Указатель на пользовательскую функцию.
	/// Установка указателя кэлбэка обработки принятых пакетов от клиентов.;
	/// Внутри кэлбэка ОБЯЗАТЕЛЬНО в вызовах с возможностью установки bTryLock - ставить false, кэлбэки и так под локом.
	static void SetClientDataArrivedCB(CBClientDataArrived pf_CBClientDataArrivedIn);
								///< \param[in] pf_CBClientDataArrivedIn Указатель на пользовательскую функцию.
	/// Установка указателя кэлбэка отслеживания статута клиентов;
	/// Внутри кэлбэка ОБЯЗАТЕЛЬНО в вызовах с возможностью установки bTryLock - ставить false, кэлбэки и так под локом.
	static void SetClientStatusChangedCB(CBClientStatusChanged pf_CBClientStatusChangedIn);
								///< \param[in] pf_CBClientStatusChangedIn Указатель на пользовательскую функцию.
	/// Доступ к первому элементу заданного типа из массива принятых пакетов от текущего клиента.
	static int AccessSelectedTypeOfData(int iConnection, void** pp_vDataBuffer, unsigned short ushType, bool bTryLock = true);
								///< \param[in] iConnection Номер соединения.
								///< \param[in,out] pp_vDataBuffer Указатель на указатель на буфер с данными.
								///< \param[in] ushType Тип искомого пакета.
								///< \param[in] bTryLock Установить в false при использовании внутри кэлбэков.
								///< \return Номер в массиве при удаче или NO_CONNECTION и DATA_NOT_FOUND соотв.
	/// Удаление выбранного элемента в массиве принятых пакетов.
	static int ReleaseDataInPosition(int iConnection, uint uiPos, bool bTryLock = true);
								///< \param[in] iConnection Номер соединения.
								///< \param[in] uiPos Позиция в массиве.
								///< \param[in] bTryLock Установить в false при использовании внутри кэлбэков.
								///< \return RETVAL_OK, если удачно и NO_CONNECTION соотв.
	/// Получение копии структуры описания соединения по индексу.
	static NetHub::ConnectionData GetConnectionData(int iConnection, bool bTryLock = true);
								///< \param[in] iConnection Номер соединения.
								///< \param[in] bTryLock Установить в false при использовании внутри кэлбэков.
								///< \return NetHub::ConnectionData.iStatus == NO_CONNECTION - соединение не действительно.
	/// Заполнение буферов имён IP и порта.
	static void FillIPAndPortNames(NetHub::ConnectionData& a_ConnectionData, char* p_chIP,
								   char* p_chPort = nullptr, bool bTryLock = true);
								///< \param[in] a_ConnectionData Ссылка на структуру описания соединения.
								///< \param[in,out] p_chIP Указатель на буфер имени IP.
								///< \param[in,out] p_chPort Указатель на буфер имени порта.
								///< \param[in] bTryLock Установить в false при использовании внутри кэлбэков.
	/// Отказ от работы или принудительное отключение клиента.
	static bool RejectOrWithdrawClient(int iConnection, bool bKick, bool bTryLock = true);
								///< \param[in] iConnection Номер соединения.
								///< \param[in] bKick При true - выбросить клиента, при false - отключить.
								///< \param[in] bTryLock Установить в false при использовании внутри кэлбэков.
								///< \return true - при удаче.
	/// Получение флага авторизованного соединения.
	static bool IsConnectionSecured(int iConnection);
								///< \param[in] iConnection Номер соединения.
								///< \return true - при авторизованном.
private:
	/// Отправка пакета по соединению немедленно.
	static bool SendToConnectionImmediately(int iConnection, unsigned short ushCommand, bool bFullFlag = false, char* p_chBuffer = nullptr,
											int iLength = 0, bool bResetPointer = true);
								///< \param[in] iConnection Номер соединения.
								///< \param[in] ushCommand Код команды протокола.
								///< \param[in] bFullFlag Признак переполнения на сервере для фиктивной попытки отправки.
								///< \param[in] p_chBuffer Указатель на буфер с данными для отправки.
								///< \param[in] iLength Длина буфера в байтах.
								///< \param[in] bResetPointer Сбрасывать ли указатель на начало буфера (для нового заполнения).
								///< \return true - при удаче.
	/// Отправка буфера по соединению.
	static bool SendBufferToConnection(int iConnection, bool bFullFlag = false, bool bResetPointer = true);
								///< \param[in] iConnection Номер соединения.
								///< \param[in] bFullFlag Признак переполнения на сервере для фиктивной попытки отправки.
								///< \param[in] bResetPointer Сбрасывать ли указатель на начало буфера (для нового заполнения).
								///< \return true - при удаче.
	/// Очистка позиции данных потока.
	static void CleanThrDadaPos(int iConnection);
								///< \param[in] iConnection Позиция в массиве.
	/// Поиск свободной позиции данных потока.
	static int FindFreeThrDadaPos();
								///< \return Возвращает номер свободной позиции, иначе - NO_CONNECTION.
	/// Поток соединения.
	static void* ConversationThread(void* p_vNum);
								///< \param[in] p_vNum Неопределённый указатель на int-переменную с номером соединения.
								///< \return Заглушка.
	/// Серверный поток.
	static void* ServerThread(void *p_vPlug);
								///< \param[in] p_vPlug Заглушка.
								///< \return Заглушка.
	/// Заполнение структуры описания соединения.
	static void FillConnectionData(int iSocket, NetHub::ConnectionData& a_ConnectionData);
								///< \param[in] iSocket Сокет.
								///< \param[in,out] a_ConnectionData Ссылка на структуру для заполнения.
};

#endif // SERVER_H
