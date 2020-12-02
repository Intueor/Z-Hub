#ifndef NET_HUB_H
#define NET_HUB_H

//== ВКЛЮЧЕНИЯ.
#ifdef WIN32
#include <WinSock2.h>
#include <Ws2ipdef.h>
#include "../dlfcn-win32/dlfcn.h"
#include <vector>
#define uint unsigned int
#else
#include <dlfcn.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#include "../Z-Hub/logger.h"
#include "proto-parser.h"
#include "net-hub-defs.h"

//== МАКРОСЫ.
#define TryMutexInit			int* p_iLocked = 0;
#define TryMutexLock(Mutex)		{p_iLocked = new int;																					\
								unsigned short* p_ushC = new unsigned short;															\
								for(*p_ushC = 0; *p_ushC < PTHREAD_TRYLOCK_ATTEMPTS; *p_ushC += 1)										\
								{																										\
									*p_iLocked = pthread_mutex_trylock(&Mutex);															\
									if(*p_iLocked == 0) break;																			\
									if(*p_ushC > (PTHREAD_TRYLOCK_ATTEMPTS / 2))                                                        \
									{                                                                                                   \
										LOG_P_2(LOG_CAT_W, "'ptherad' locking is too long.");											\
										RETVAL_SET(RETVAL_ERR);                                                                         \
										DebugHelper(-1);																				\
									}                                                                                                   \
									MSleep(PTHREAD_TRYLOCK_TIMESTEP);																	\
								}																										\
								delete p_ushC;																							\
								if(*p_iLocked != 0)																						\
								{																										\
									LOG_P_0(LOG_CAT_E, "'ptherad' locking fault.");														\
								}}
#define TryMutexUnlock(Mutex)	{if(*p_iLocked == 0) pthread_mutex_unlock(&Mutex);														\
								delete p_iLocked;}

#define ResetBit(field,bit)					field |= bit; field ^= bit
#define CopyBit(field_src,field_dst,bit)	ResetBit(field_dst,bit); field_dst |= (field_src & bit)
// Сообщения.
#define MSG_WRONG_CONNECTION    "Wrong connection number."

/// Класс хаба сетевых операций.
class NetHub
{
public:
	/// Структура входных строк адреса и пароля.
	struct IPPortPassword
	{
		char* p_chIPNameBuffer; ///< Указатель буфер строки с IP.
		char* p_chPortNameBuffer; ///< Указатель буфер строки с портом.
		char* p_chPasswordNameBuffer; ///< Указатель буфер строки с паролем.
	};
	/// Сруктура для данных по соединению.
	struct ConnectionData
	{
		int iSocket; ///< Сокет.
		char ai_addr[sizeof(sockaddr_in6)]; ///< Адрес для Ipv4 и Ipv6.
#ifndef WIN32
		socklen_t ai_addrlen; ///< Длина адреса.
#else
		size_t ai_addrlen; ///< Длина адреса.
#endif
		int iStatus; ///< Статус последней операции.
	};
	/// Структура принятого пакета.
	struct ReceivedData
	{
		bool bBusy; ///< Признак занятой данными структуры.
		ProtocolStorage oProtocolStorage; ///< Структура с данными.
	};

public:
	/// Конструктор.
	NetHub();
	/// Сброс указателя позиции в буфере пакетов.
	void ResetPocketsBufferPositionPointer();
	/// Проверка 'пустого' буфера пакетов.
	bool CheckIsBufferFree();
													///< \return true при указателе на начале буфера.
	/// Добавление пакета в буфер отправки.
	bool AddPocketToOutputBuffer(unsigned short ushCommand, char *p_chBuffer = nullptr, int iLength = 0);
													///< \param[in] ushCommand Команда, которая будет задана в начале пакета.
													///< \param[in] p_chBuffer Указатель на буффер с данными.
													///< \param[in] iLength Длина пакета в байтах.
													///< \return true при удаче.
	/// Отправка пакета адресату.
	bool SendToAddress(ConnectionData &oConnectionData, bool bResetPointer = true);
													///< \param[in,out] oConnectionData Ссылка на структуру описания соединения.
													///< \param[in] bResetPointer Сбрасывать ли указатель на начало буфера.
													///< \return true при удаче.
	/// Поиск свободного элемента хранилища пакетов.
	int FindFreeReceivedPocketsPos(ReceivedData* p_mReceivedPockets);
													///< \param[in] p_mReceivedPockets Указатель на массив с пакетами хабов.
													///< \return Номер первой свободной позиции или BUFFER_IS_FULL.
	/// Доступ к первому элементу заданного типа из массива принятых пакетов.
	int AccessSelectedTypeOfData(void** pp_vDataBuffer, ReceivedData* p_mReceivedPockets, unsigned short ushType);
													///< \param[in,out] pp_vDataBuffer Указатель на указатель на буфер с данными.
													///< \param[in] p_mReceivedPockets Указатель на массив с пакетами хабов.
													///< \param[in] ushType Тип искомого пакета.
													///< \return Номер в массиве при удаче или NO_CONNECTION и DATA_NOT_FOUND соотв.
	/// Удаление выбранного элемента в массиве принятых пакетов.
	int ReleaseDataInPosition(ReceivedData* p_mReceivedPockets, uint uiPos);
													///< \param[in] p_mReceivedPockets Указатель на массив с пакетами хабов.
													///< \param[in] uiPos Позиция в массиве.
													///< \return RETVAL_OK, если удачно, DATA_ACCESS_ERROR и NO_CONNECTION соотв.
	/// Проверка на тип протокола IPv4 в буфере.
	static bool CheckIPv4(char* p_chIPNameBuffer);
													///< \param[in] p_chIPNameBuffer Указатель на буфер строки со значением адреса.
													///< \return true при протоколе IPv4.

private:
	char m_chPocketsBuffer[MAX_DATA]; ///< Рабочий буфер пакетов.
	char* p_chPocketsBufferPositionPointer; ///< Указатель на позицию в буфере.

public:
	static char m_chDec[];
	static char m_chHex[];
};
#endif // NET_HUB_H
