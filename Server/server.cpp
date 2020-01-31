//== ВКЛЮЧЕНИЯ.
#include <Server/server.h>
#include <string.h>

//== МАКРОСЫ.
#define LOG_NAME				"Z-Server"
#define LOG_DIR_PATH			"../Z-Server/logs/"

//== ДЕКЛАРАЦИИ СТАТИЧЕСКИХ ПЕРЕМЕННЫХ.
LOGDECL_INIT_INCLASS(Server)
LOGDECL_INIT_PTHRD_INCLASS_EXT_ADD(Server)
bool Server::bServerAlive = false;
bool Server::bExitSignal = false;
pthread_mutex_t Server::ptConnMutex = PTHREAD_MUTEX_INITIALIZER;
int Server::iListener = 0;
bool Server::bRequestNewConn = false;
Server::ConversationThreadData Server::mThreadDadas[MAX_CONN];
bool Server::bListenerAlive = false;
pthread_t Server::ServerThr;
CBClientRequestArrived Server::pf_CBClientRequestArrived = nullptr;
CBClientDataArrived Server::pf_CBClientDataArrived = nullptr;
CBClientStatusChanged Server::pf_CBClientStatusChanged = nullptr;
pthread_t Server::p_ThreadOverrunned;
vector<Server::IPBanUnit>* Server::p_vIPBansInt = nullptr;
NetHub::IPPortPassword* Server::p_IPPortPasswordInt = nullptr;
PServerName Server::oPServerName;

//== ФУНКЦИИ КЛАССОВ.
//== Класс сервера.
// Конструктор.
Server::Server(pthread_mutex_t ptLogMutex, vector<IPBanUnit>* p_vIPBans)
{
	LOG_CTRL_BIND_EXT_MUTEX(ptLogMutex);
	LOG_CTRL_INIT;
	p_vIPBansInt = p_vIPBans;
}

// Деструктор.
Server::~Server()
{
	LOG_CLOSE;
}

// Запрос запуска клиента.
bool Server::Start(NetHub::IPPortPassword* p_IPPortPassword, char* p_chServerName)
{
	if(!bServerAlive) // Предупреждение повторного запуска.
	{
		memcpy(&oPServerName.m_chServerName, p_chServerName, SERVER_NAME_STR_LEN);
		LOG_P_2(LOG_CAT_I, "Starting server thread.");
		p_IPPortPasswordInt = p_IPPortPassword;
		pthread_create(&ServerThr, nullptr, ServerThread, nullptr); // Запуск потока сервера.
		return true;
	}
	LOG_P_0(LOG_CAT_E, "The server can`t be started - it is already running.");
	return false;
}

// Запрос остановки клиента.
bool Server::Stop()
{
	if(bServerAlive) // Для контроля повторной остановки.
	{
		bExitSignal = true; // Установка сигнала для потоков сервера и обмена.
		return true;
	}
	LOG_P_0(LOG_CAT_E, "The server can`t be stopped - it has been stopped already.");
	return false;
}

// Запрос о действующем сервере.
bool Server::CheckServerAlive()
{
	return bServerAlive;
}

// Добавление пакета в буфер отправки.
bool Server::AddPocketToOutputBuffer(int iConnection, unsigned short ushCommand, char *p_chBuffer, int iLength, bool bTryLock)
{
	bool bRes;
	TryMutexInit;
	//
	if(bTryLock) TryMutexLock;
	if(iConnection != NO_CONNECTION)
	{
		// Добавление в буфер данного хаба.
		bRes = mThreadDadas[iConnection].oLocalNetHub.AddPocketToOutputBuffer(ushCommand, p_chBuffer, iLength);
	}
	else
	{
		LOG_P_0(LOG_CAT_E, MSG_WRONG_CONNECTION);
		bRes = false;
	}
	if(bTryLock) TryMutexUnlock;
	return bRes;
}

// Сброс указателя позиции в буфере пакетов выбранного соединения.
bool Server::ResetPocketsBufferPositionPointer(int iConnection, bool bTryLock)
{
	bool bRes = true;
	TryMutexInit;
	//
	if(bTryLock) TryMutexLock;
	if(iConnection != NO_CONNECTION)
	{
		mThreadDadas[iConnection].oLocalNetHub.ResetPocketsBufferPositionPointer(); // Сброс внутреннего казателя буфера данного хаба.
	}
	else
	{
		LOG_P_0(LOG_CAT_E, MSG_WRONG_CONNECTION);
		bRes = false;
	}
	if(bTryLock) TryMutexUnlock;
	return bRes;
}

// Отправка пакета по соединению немедленно.
bool Server::SendToConnectionImmediately(int iConnection, unsigned short ushCommand,
						  bool bFullFlag, char *p_chBuffer, int iLength, bool bResetPointer)
{
	if(bFullFlag == false)
	{
		if(AddPocketToOutputBuffer(iConnection, ushCommand, p_chBuffer, iLength, false) == false) // Добавляем в локальный буфер.
		{
			LOG_P_0(LOG_CAT_E, "Pockets buffer is full.");
			return false;
		}
		if(iConnection != NO_CONNECTION)
		{
			if(mThreadDadas[iConnection].oLocalNetHub.SendToAddress(
						mThreadDadas[iConnection].oConnectionData, bResetPointer) == false) // Отправка.
			{
				LOG_P_0(LOG_CAT_E, "Socket error on sending data.");
				return false;
			}
		}
		else
		{
			LOG_P_0(LOG_CAT_E, MSG_WRONG_CONNECTION);
			return false;
		}
	}
	else
	{
		LOG_P_0(LOG_CAT_E, "Client buffer is full.");
		return false;
	}
	return true;
}

// Отправка буфера по соединению.
bool Server::SendBufferToConnection(int iConnection, bool bFullFlag, bool bResetPointer)
{
	if(bFullFlag == false)
	{
		if(iConnection != NO_CONNECTION)
		{
			if(mThreadDadas[iConnection].oLocalNetHub.SendToAddress(mThreadDadas[iConnection].oConnectionData, // Отправка.
																			bResetPointer) == false)
			{
				LOG_P_0(LOG_CAT_E, "Socket error on sending data.");
				return false;
			}
		}
		else
		{
			LOG_P_0(LOG_CAT_E, MSG_WRONG_CONNECTION);
			return false;
		}
	}
	else
	{
		LOG_P_0(LOG_CAT_E, "Client buffer is full.");
		return false;
	}
	return true;
}

// Отправка пакета клиенту на текущее выбранное соединение немедленно.
bool Server::SendToClientImmediately(int iConnection, unsigned short ushCommand,
									 char* p_chBuffer, int iLength, bool bResetPointer, bool bTryLock)
{
	bool bRes = false;
	TryMutexInit;
	//
	if(bTryLock) TryMutexLock;
	if(iConnection == NO_CONNECTION) goto gUE;
	if((mThreadDadas[iConnection].bFullOnClient == false) && (mThreadDadas[iConnection].bSecured == true))
	{
		if(SendToConnectionImmediately(iConnection, ushCommand, false, p_chBuffer, iLength, bResetPointer) == true)
			bRes = true;
	}
gUE:if(bRes == false) // При невозможности отправки из-за переполнения буфера на клиенте или неавторизованном клиенте.
	{
		if(iConnection == NO_CONNECTION)
		{
			LOG_P_0(LOG_CAT_E, MSG_WRONG_CONNECTION); // При отсутствии соединения неавторизованность игнорируется.
		}
		else
		{
			if(mThreadDadas[iConnection].bSecured == false) // При неавторизованном клиенте отправка засчитывается.
			{
				LOG_P_2(LOG_CAT_W, "Sending rejected due to not secured for: " << iConnection);
				bRes = true;
			}
			else
			{
				LOG_P_0(LOG_CAT_E, "Sending failed for: " << iConnection);
			}
		}
	}
	if(bTryLock) TryMutexUnlock;
	return bRes;
}

// Отправка буфера клиенту на текущее выбранное соединение.
bool Server::SendBufferToClient(int iConnection, bool bResetPointer, bool bTryLock)
{
	bool bRes = false;
	TryMutexInit;
	//
	if(bTryLock) TryMutexLock;
	if(iConnection == NO_CONNECTION) goto gUE;
	if((mThreadDadas[iConnection].bFullOnClient == false) && (mThreadDadas[iConnection].bSecured == true))
	{
		if(SendBufferToConnection(iConnection, false, bResetPointer) == true)
			bRes = true;
	}
gUE:if(bRes == false)  // При невозможности отправки из-за переполнения буфера на клиенте или неавторизованном клиенте.
	{
		if(iConnection == NO_CONNECTION)
		{
			LOG_P_0(LOG_CAT_E, MSG_WRONG_CONNECTION); // При отсутствии соединения неавторизованность игнорируется.
		}
		else
		{
			if(mThreadDadas[iConnection].bSecured == false) // При неавторизованном клиенте отправка засчитывается.
			{
				LOG_P_2(LOG_CAT_W, "Sending rejected due to not secured for: " << iConnection);
				bRes = true;
			}
			else
			{
				LOG_P_0(LOG_CAT_E, "Sending failed for: " << iConnection);
			}
		}
	}
	if(bTryLock) TryMutexUnlock;
	return bRes;
}

// Установка указателя кэлбэка изменения статуса подключения клиента.
void Server::SetClientRequestArrivedCB(CBClientRequestArrived pf_CBClientRequestArrivedIn)
{
	TryMutexInit;
	//
	TryMutexLock;
	pf_CBClientRequestArrived = pf_CBClientRequestArrivedIn;
	TryMutexUnlock;
}

// Установка указателя кэлбэка обработки принятых пакетов от клиентов.
void Server::SetClientDataArrivedCB(CBClientDataArrived pf_CBClientDataArrivedIn)
{
	TryMutexInit;
	//
	TryMutexLock;
	pf_CBClientDataArrived = pf_CBClientDataArrivedIn;
	TryMutexUnlock;
}

// Установка указателя кэлбэка отслеживания статута клиентов.
void Server::SetClientStatusChangedCB(CBClientStatusChanged pf_CBClientStatusChangedIn)
{
	TryMutexInit;
	//
	TryMutexLock;
	pf_CBClientStatusChanged = pf_CBClientStatusChangedIn;
	TryMutexUnlock;
}

// Удаление выбранного элемента из массива принятых пакетов.
int Server::ReleaseDataInPosition(int iConnection, uint uiPos, bool bTryLock)
{
	int iRes = DATA_NOT_FOUND;
	TryMutexInit;
	//
	if(bTryLock) TryMutexLock;
	if(iConnection != NO_CONNECTION)
	{
		if(mThreadDadas[iConnection].bFullOnServer == true)
		{
			mThreadDadas[iConnection].bFullOnServer = false;
			SendToConnectionImmediately(iConnection, PROTO_A_BUFFER_READY); // При освоб. позиции в буфере на сервере в данном хабе.
		}
		iRes = mThreadDadas[iConnection].oLocalNetHub.ReleaseDataInPosition(mThreadDadas[iConnection].mReceivedPockets, uiPos);
//        if(iRes == RETVAL_OK)
//        {
//            LOG_P_2(LOG_CAT_I, "Position has been released.");
//        }
	}
	else
	{
		iRes = NO_CONNECTION;
		LOG_P_0(LOG_CAT_E, MSG_WRONG_CONNECTION);
	}
	if(bTryLock) TryMutexUnlock;
	if(iRes == DATA_NOT_FOUND)
	{
		LOG_P_0(LOG_CAT_E, "Trying to release empty position.");
	}
	return iRes;
}

// Доступ к первому элементу заданного типа из массива принятых пакетов от текущего клиента.
int Server::AccessSelectedTypeOfData(int iConnection, void** pp_vDataBuffer, unsigned short ushType, bool bTryLock)
{
	int iRes;
	TryMutexInit;
	//
	if(bTryLock) TryMutexLock;
	if(iConnection != NO_CONNECTION)
	{
		iRes = mThreadDadas[iConnection].
				oLocalNetHub.AccessSelectedTypeOfData(pp_vDataBuffer, mThreadDadas[iConnection].mReceivedPockets, ushType);
		if(bTryLock) TryMutexUnlock;
		return iRes;
	}
	LOG_P_0(LOG_CAT_E, "Wrong connection number (access).");
	if(bTryLock) TryMutexUnlock;
	return NO_CONNECTION;
}

// Принудительное отключение клиента.
bool Server::WithdrawClient(int iConnection, bool bTryLock)
{
	TryMutexInit;
	bool bRetval = true;
	//
	if(bTryLock) TryMutexLock;
	mThreadDadas[iConnection].bKick = true; // Установка признака принудительного отключения.
	if(SendToConnectionImmediately(iConnection, (char)PROTO_S_KICK, mThreadDadas[iConnection].bFullOnClient))
	{
		if(bTryLock) TryMutexUnlock;
		MSleep(WAITING_FOR_CLIENT_DSC);
		if(bTryLock) TryMutexLock;
	}
	else
	{
		bRetval = false;
	}
#ifndef WIN32
	shutdown(mThreadDadas[iConnection].oConnectionData.iSocket, SHUT_RDWR);
	close(mThreadDadas[iConnection].oConnectionData.iSocket);
#else
	closesocket(mThreadDadas[iConnection].oConnectionData.iSocket);
#endif
	if(bTryLock) TryMutexUnlock;
	return bRetval;
}

// Очистка позиции данных потока.
void Server::CleanThrDadaPos(int iConnection)
{
	for(uint uiC = 0; uiC < S_MAX_STORED_POCKETS; uiC++) // По всем сохранённым пакетам.
	{
		if(mThreadDadas[iConnection].mReceivedPockets[uiC].bBusy) // При данных в позиции буфера...
		{
			mThreadDadas[iConnection].mReceivedPockets[uiC].oProtocolStorage.Release(); // Вызов очистки.
		}
	}
	// Общее обнуление.
	mThreadDadas[iConnection].bFullOnClient = false;
	mThreadDadas[iConnection].bFullOnServer = false;
	mThreadDadas[iConnection].bSecured = false;
	mThreadDadas[iConnection].iCurrentFreePocket = 0;
	memset(&mThreadDadas[iConnection].mReceivedPockets, 0, sizeof(NetHub::ReceivedData));
	memset(&mThreadDadas[iConnection].m_chData, 0, sizeof(mThreadDadas[iConnection].m_chData));
}

// Поиск свободной позиции данных потока.
int Server::FindFreeThrDadaPos()
{
	uint uiPos = 0;
	TryMutexInit;
	//
	TryMutexLock;
	for(; uiPos != MAX_CONN; uiPos++)
	{
		if(mThreadDadas[uiPos].bInUse == false) // Если не стоит флаг занятости - годен.
		{
			TryMutexUnlock;
			return (int)uiPos;
		}
	}
	TryMutexUnlock;
	return NO_CONNECTION;
}

// Получение копии структуры описания соединения по индексу.
NetHub::ConnectionData Server::GetConnectionData(int iConnection, bool bTryLock)
{
	NetHub::ConnectionData oConnectionDataRes;
	TryMutexInit;
	//
	if(bTryLock) TryMutexLock;
	if((iConnection < MAX_CONN) && (mThreadDadas[iConnection].bInUse == true))
	{
		oConnectionDataRes = mThreadDadas[iConnection].oConnectionData;
		if(bTryLock) TryMutexUnlock;
		return oConnectionDataRes;
	}
	if(bTryLock) TryMutexUnlock;
	oConnectionDataRes.iStatus = NO_CONNECTION;
	return oConnectionDataRes;
}

// Получение флага авторизованного соединения.
bool Server::IsConnectionSecured(int iConnection)
{
	return mThreadDadas[iConnection].bSecured;
}

// Заполнение структуры описания соединения.
void Server::FillConnectionData(int iSocket, NetHub::ConnectionData& a_ConnectionData)
{
	TryMutexInit;
	//
	a_ConnectionData.ai_addrlen = sizeof(sockaddr_in6);
	TryMutexLock;
	a_ConnectionData.iSocket = iSocket;
	a_ConnectionData.iStatus = 0;
#ifndef WIN32
	getpeername(a_ConnectionData.iSocket, (sockaddr*)a_ConnectionData.ai_addr,
				&a_ConnectionData.ai_addrlen);
#else
	getpeername(a_ConnectionData.iSocket, (sockaddr*)a_ConnectionData.ai_addr,
				(int*)&a_ConnectionData.ai_addrlen);
#endif
	TryMutexUnlock;
}

// Заполнение буферов имён IP и порта.
void Server::FillIPAndPortNames(NetHub::ConnectionData& a_ConnectionData, char* p_chIP, char* p_chPort, bool bTryLock)
{
	TryMutexInit;
	//
	if(bTryLock) TryMutexLock;
#ifndef WIN32
	getnameinfo((sockaddr*)a_ConnectionData.ai_addr,
				a_ConnectionData.ai_addrlen,
				p_chIP, INET6_ADDRSTRLEN, p_chPort, PORT_STR_LEN, NI_NUMERICHOST);
#else
	getnameinfo((sockaddr*)a_ConnectionData.ai_addr,
				(socklen_t)a_ConnectionData.ai_addrlen,
				p_chIP, INET6_ADDRSTRLEN, p_chPort, PORT_STR_LEN, NI_NUMERICHOST);
#endif
	if(bTryLock) TryMutexUnlock;
}

// Поток соединения.
void* Server::ConversationThread(void* p_vNum)
{
	int iTPos;
	bool bKillListenerAccept;
	ProtoParser* p_ProtoParser;
	ProtoParser::ParseResult oParsingResult;
	char m_chIPNameBuffer[INET6_ADDRSTRLEN +2];
	char m_chPortNameBuffer[PORT_STR_LEN];
	bool bLocalExitSignal;
	NetHub::ReceivedData* p_CurrentData;
	int iTempListener;
	int iTempTPos;
	NetHub::ConnectionData oConnectionDataInt;
	char* p_chData;
	int iLength;
	NetHub::ReceivedData oReceivedDataReserve;
	bool bTempIPv4;
	//
	bKillListenerAccept = false;
	bServerAlive = true;
	bLocalExitSignal = false;
	iTempTPos = NO_CONNECTION;
	iTPos = *((int*)p_vNum); // Получили номер в массиве.
gBA:if(iTPos != NO_CONNECTION)
	{
		mThreadDadas[iTPos].bKick = false;
		mThreadDadas[iTPos].p_Thread = pthread_self(); // Задали ссылку на текущий поток.
#ifndef WIN32
		LOG_P_2(LOG_CAT_I, "Waiting for connection on thread: " << mThreadDadas[iTPos].p_Thread);
#else
		LOG_P_2(LOG_CAT_I, "Waiting connection on thread: " << mThreadDadas[iTPos].p_Thread.p);
#endif
	}
	else
	{
#ifndef WIN32
		LOG_P_1(LOG_CAT_W, "Waiting for connection on reserved thread: " << pthread_self());
#else
		LOG_P_1(LOG_CAT_W, "Waiting connection on reserved thread: " << pthread_self().p);
#endif
gAG:	iTempListener = (int)accept(iListener, nullptr, nullptr); // Ждём перегруженных входящих.
		FillConnectionData(iTempListener, oConnectionDataInt);
		FillIPAndPortNames(oConnectionDataInt, m_chIPNameBuffer, m_chPortNameBuffer);
		iTempTPos = FindFreeThrDadaPos();
		if(iTempTPos == NO_CONNECTION)
		{
#ifndef WIN32
			shutdown(iTempListener, SHUT_RDWR);
			close(iTempListener);
#else
			closesocket(iTempListener);
#endif
			if(NetHub::CheckIPv4(m_chIPNameBuffer))
			{
				LOG_P_2(LOG_CAT_W, "Connection is rejected for: " << m_chIPNameBuffer);
			}
			else
			{
				LOG_P_2(LOG_CAT_W, "Connection is rejected for: [" << m_chIPNameBuffer << "]");
			}
			if(bExitSignal) goto gOE;
			goto gAG;
		}
		iTPos = iTempTPos;
		mThreadDadas[iTPos].oConnectionData.iSocket = iTempListener;
	}
	if(iTempTPos == NO_CONNECTION) // Если пришло мимо перегруженного ожидания...
		mThreadDadas[iTPos].oConnectionData.iSocket = (int)accept(iListener, nullptr, nullptr); // Ждём входящих.
	if((mThreadDadas[iTPos].oConnectionData.iSocket < 0)) // При ошибке после выхода из ожидания входящих...
	{
		if(!bExitSignal) // Если не было сигнала на выход от основного потока...
		{
			pthread_mutex_lock(&ptConnMutex);
			LOG_P_0(LOG_CAT_E, "'accept': " << gai_strerror(mThreadDadas[iTPos].oConnectionData.iSocket)); // Про ошибку.
			RETVAL_SET(RETVAL_ERR);
			goto enc;
		}
		else
		{
gOE:		pthread_mutex_lock(&ptConnMutex);
			LOG_P_1(LOG_CAT_I, "Accepting connections is terminated."); // Норм. сообщение.
			bKillListenerAccept = true; // Заказываем подтверждение закрытия приёмника.
			goto enc;
		}
	}
	FillConnectionData(mThreadDadas[iTPos].oConnectionData.iSocket, mThreadDadas[iTPos].oConnectionData);
	FillIPAndPortNames(mThreadDadas[iTPos].oConnectionData, m_chIPNameBuffer, m_chPortNameBuffer);
	bTempIPv4 = NetHub::CheckIPv4(m_chIPNameBuffer);
	if(!bTempIPv4)
	{
		for(int iP = sizeof(m_chIPNameBuffer) - 1; iP != 0; iP--)
		{
			m_chIPNameBuffer[iP] = m_chIPNameBuffer[iP -1];
		}
		m_chIPNameBuffer[0] = '[';
		for(int iP = 0; iP != sizeof(m_chIPNameBuffer); iP++)
		{
			if(m_chIPNameBuffer[iP] == 0)
			{
				m_chIPNameBuffer[iP] = ']';
				m_chIPNameBuffer[iP + 1] = 0;
				break;
			}
		}
	}
	if(p_vIPBansInt != nullptr) // Обработка банов.
	{
		for(uint uiN = 0; uiN < (*p_vIPBansInt).size(); uiN++)
		{
			if(!strcmp((*p_vIPBansInt).at(uiN).m_chIP, m_chIPNameBuffer))
			{
				LOG_P_0(LOG_CAT_W, "Connection is rejected due to a ban for: " << m_chIPNameBuffer);
				SendToConnectionImmediately(iTPos, PROTO_S_BAN);
				MSleep(WAITING_FOR_CLIENT_DSC);
#ifndef WIN32
				shutdown(mThreadDadas[iTPos].oConnectionData.iSocket, SHUT_RDWR);
				close(mThreadDadas[iTPos].oConnectionData.iSocket);
#else
				closesocket(mThreadDadas[iTPos].oConnectionData.iSocket);
#endif
				goto gBA;
			}
		}
	}
	LOG_P_1(LOG_CAT_I, "New connection is accepted.");
	mThreadDadas[iTPos].bInUse = true; // Флаг занятости структуры.
	LOG_P_1(LOG_CAT_I, "Connected with: " << m_chIPNameBuffer << ":" << m_chPortNameBuffer << " ID: " << iTPos);
	if(mThreadDadas[iTPos].oConnectionData.iStatus == -1) // Если не вышло отправить...
	{
		pthread_mutex_lock(&ptConnMutex);
		LOG_P_0(LOG_CAT_E, "'send': " << gai_strerror(mThreadDadas[iTPos].oConnectionData.iStatus));
		RETVAL_SET(RETVAL_ERR);
		goto ec;
	}
	//
	bRequestNewConn = true; // Соединение готово - установка флага для главного потока на запрос нового.
	if(pf_CBClientStatusChanged != 0)
	{
		// Вызов кэлбэка смены статуса.
		pf_CBClientStatusChanged(iTPos, true);
	}
	p_ProtoParser = new ProtoParser;
	while(bExitSignal == false) // Пока не пришёл флаг общего завершения...
	{
		mThreadDadas[iTPos].oConnectionData.iStatus = (int) // Принимаем пакет в текущую позицию.
				recv(mThreadDadas[iTPos].oConnectionData.iSocket,
					 mThreadDadas[iTPos].m_chData,
				sizeof(mThreadDadas[iTPos].m_chData), 0);
		pthread_mutex_lock(&ptConnMutex);
		if (bExitSignal == true) // Если по выходу из приёмки обнаружен общий сигнал на выход...
		{
			LOG_P_2(LOG_CAT_I, "Exiting reading from ID: " << iTPos);
			goto ecd;
		}
		if (mThreadDadas[iTPos].oConnectionData.iStatus <= 0) // Если статус приёмки - отказ (вместо принятых байт)...
		{
			LOG_P_2(LOG_CAT_I, "Reading socket stopped for ID: " << iTPos);
			goto ecd;
		}
		p_chData = mThreadDadas[iTPos].m_chData;
		iLength = mThreadDadas[iTPos].oConnectionData.iStatus;
		//
gDp:	mThreadDadas[iTPos].iCurrentFreePocket =
				mThreadDadas[iTPos].oLocalNetHub.FindFreeReceivedPocketsPos(mThreadDadas[iTPos].mReceivedPockets);
		if(mThreadDadas[iTPos].iCurrentFreePocket != BUFFER_IS_FULL)
			p_CurrentData = &mThreadDadas[iTPos].mReceivedPockets[mThreadDadas[iTPos].iCurrentFreePocket];
		else p_CurrentData = &oReceivedDataReserve;
		oParsingResult =
				p_ProtoParser->ParsePocket(p_chData, iLength, p_CurrentData->oProtocolStorage, mThreadDadas[iTPos].bFullOnServer);
		switch(oParsingResult.iRes)
		{
			case PROTOPARSER_OK:
			{
				if(oParsingResult.bStored == true)
				{
//                    LOG_P_2(LOG_CAT_I, "Received pocket: " <<
//                        (mThreadDadas[iTPos].iCurrentFreePocket + 1) << " for ID: " << iTPos);
					mThreadDadas[iTPos].mReceivedPockets[mThreadDadas[iTPos].iCurrentFreePocket].bBusy = true;
				}
				else
				{
					if(pf_CBClientRequestArrived != nullptr)
					{
						// Вызов кэлбэка прибытия запроса.
						pf_CBClientRequestArrived(iTPos, oParsingResult.ushTypeCode);
					}
				}
				if(mThreadDadas[iTPos].bSecured == false)
				{
					if(oParsingResult.ushTypeCode == PROTO_C_SEND_PASSW)
					{
						char* p_PassW = ((PPassword*)p_CurrentData->oProtocolStorage.p_Data)->m_chPassw;
						if(!strcmp(p_IPPortPasswordInt->p_chPasswordNameBuffer, p_PassW))
						{
							SendToConnectionImmediately(iTPos, PROTO_S_PASSW_OK);
							mThreadDadas[iTPos].bSecured = true;
							SendToConnectionImmediately(iTPos, PROTO_S_SERVER_NAME, false,
														(char*)&oPServerName, sizeof(PServerName));
							LOG_P_1(LOG_CAT_I, "Connection is secured for ID: " << iTPos);
						}
						else
						{
							SendToConnectionImmediately(iTPos, PROTO_S_PASSW_ERR);
							mThreadDadas[iTPos].bSecured = false;
							LOG_P_0(LOG_CAT_W, "Authentification failed for ID: " << iTPos);
						}
						p_CurrentData->oProtocolStorage.Release();
						p_CurrentData->bBusy = false;
						goto gI;
					}
					else
					{
						if(oParsingResult.ushTypeCode != PROTO_C_REQUEST_LEAVING)
						{
							SendToConnectionImmediately(iTPos, PROTO_S_UNSECURED);
							LOG_P_0(LOG_CAT_W, "Client is not autherised, ID: " << iTPos);
						}
						goto gI;
					}
				}
				else
				{
					// Блок объектов.
					if(oParsingResult.bStored == true)
					{
						if(pf_CBClientDataArrived != nullptr)
						{
							// Вызов кэлбэка прибытия данных.
							pf_CBClientDataArrived(iTPos, oParsingResult.ushTypeCode,
												   mThreadDadas[iTPos].
												   mReceivedPockets[mThreadDadas[iTPos].iCurrentFreePocket].oProtocolStorage.p_Data,
									mThreadDadas[iTPos].iCurrentFreePocket);
						}
					}
				}
				// Блок взаимодействия.
gI:				switch(oParsingResult.ushTypeCode)
				{
					case PROTO_C_BUFFER_FULL:
					{
						LOG_P_1(LOG_CAT_W, "Buffer is full on ID: " << iTPos);
						mThreadDadas[iTPos].bFullOnClient = true;
						break;
					}
					case PROTO_A_BUFFER_READY:
					{
						LOG_P_1(LOG_CAT_I, "Buffer is ready on ID: " << iTPos);
						mThreadDadas[iTPos].bFullOnClient = false;
						break;
					}
					case PROTO_C_REQUEST_LEAVING:
					{
						LOG_P_1(LOG_CAT_I, "ID: " << iTPos << " request leaving.");
						SendToConnectionImmediately(iTPos, PROTO_S_ACCEPT_LEAVING);
						LOG_P_1(LOG_CAT_I, "ID: " << iTPos << " leaving accepted.");
						bLocalExitSignal = true; // Флаг самостоятельного отключения клиента.
						MSleep(WAITING_FOR_CLIENT_DSC); // Ожидание самостоятельного отключения клиента.
						goto ecd;
					}
				}
				if((mThreadDadas[iTPos].bSecured == false) && (oParsingResult.bStored == true) &&
				   (oParsingResult.ushTypeCode != PROTO_C_SEND_PASSW))
				{
					LOG_P_2(LOG_CAT_W, "Position is cleared.");
					p_CurrentData->oProtocolStorage.Release();
					p_CurrentData->bBusy = false;
				}
				break;
			}
			case PROTOPARSER_UNKNOWN_COMMAND:
			{
				SendToConnectionImmediately(iTPos, PROTO_S_UNKNOWN_COMMAND);
				LOG_P_0(LOG_CAT_W, (char*)MSG_UNKNOWN_COMMAND  << ": '" << oParsingResult.ushTypeCode << "'"
					  << " from ID: " << iTPos);
				break;
			}
			case PROTOPARSER_WRONG_FORMAT:
			{
				LOG_P_0(LOG_CAT_W, "ID: " << iTPos << (char*)MSG_WRONG_FORMAT);
				break;
			}
		}
		if(mThreadDadas[iTPos].oLocalNetHub.FindFreeReceivedPocketsPos(mThreadDadas[iTPos].mReceivedPockets) == BUFFER_IS_FULL)
		{
			LOG_P_1(LOG_CAT_W, "Buffer is full for ID: " << iTPos);
			mThreadDadas[iTPos].bFullOnServer = true;
			SendToConnectionImmediately(iTPos, PROTO_S_BUFFER_FULL);
		}
		if(oParsingResult.p_chExtraData != nullptr)
		{
//            LOG_P_2(LOG_CAT_I, "Have got merged pocket.");
			p_chData = oParsingResult.p_chExtraData;
			iLength = oParsingResult.iExtraDataLength;
			goto gDp;
		}
		pthread_mutex_unlock(&ptConnMutex);
	}
	pthread_mutex_lock(&ptConnMutex);
ecd:delete p_ProtoParser;
	//
ec: if(bLocalExitSignal == false) // Если не было локального сигнала - будут закрывать снаружи потоков.
	{
		if(bExitSignal == false)
		{
			if(mThreadDadas[iTPos].bKick)
			{
				LOG_P_1(LOG_CAT_W, "Closed due kicking out: " << m_chIPNameBuffer << " ID: " << iTPos)
			}
			else
			{
				LOG_P_0(LOG_CAT_W, "Closed by client absence: " << m_chIPNameBuffer << " ID: " << iTPos);
			}
		}
	}
	else // При локальном - закрываем здесь.
	{
#ifndef WIN32
		shutdown(mThreadDadas[iTPos].oConnectionData.iSocket, SHUT_RDWR);
		close(mThreadDadas[iTPos].oConnectionData.iSocket);
#else
		closesocket(mThreadDadas[iTPos].oConnectionData.iSocket);
#endif
		LOG_P_2(LOG_CAT_I, "Closed ordinary: " << m_chIPNameBuffer << ":" << m_chPortNameBuffer << " ID: " << iTPos);
	}
enc:if(iTPos != NO_CONNECTION)
	{
#ifndef WIN32
	LOG_P_2(LOG_CAT_I, "Exiting thread: " << mThreadDadas[iTPos].p_Thread);
#else
	LOG_P_2(LOG_CAT_I, "Exiting thread: " << mThreadDadas[iTPos].p_Thread.p);
#endif
	}
	else
	{
#ifndef WIN32
		LOG_P_1(LOG_CAT_W, "Exiting reserved thread: " << pthread_self());
#else
		LOG_P_1(LOG_CAT_W, "Exiting reserved thread: " << pthread_self().p);
#endif
	}
	if(!bKillListenerAccept)
	{
		if(pf_CBClientStatusChanged != nullptr)
		{
			// Вызов кэлбэка смены статуса.
			pf_CBClientStatusChanged(iTPos, false);
		}
	}
	if(iTPos != NO_CONNECTION)
	{
		CleanThrDadaPos(iTPos);
		if(!bExitSignal)
		{
			mThreadDadas[iTPos].bInUse = false;
		}
	}
	if(bKillListenerAccept) bListenerAlive = false;
	pthread_mutex_unlock(&ptConnMutex);
	RETURN_THREAD;
}

// Серверный поток.
void* Server::ServerThread(void *p_vPlug)
{
	p_vPlug = p_vPlug;
	int iServerStatus;
	addrinfo o_Hints;
	addrinfo* p_Res;
	int iCurrPos = 0;
	char m_chIPNameBuffer[INET6_ADDRSTRLEN + 2];
	char m_chPortNameBuffer[PORT_STR_LEN];
#ifdef WIN32
	WSADATA wsadata = WSADATA();
#endif
	//
	LOG_P_1(LOG_CAT_I, "Server thread is started.");
	// Подготовка соединения сервера.
#ifdef WIN32
	if(WSAStartup(MAKEWORD(2, 2), &wsadata) != NO_ERROR)
	{
		LOG(LOG_CAT_E, "'WSAStartup' failed");
		RETVAL_SET(RETVAL_ERR);
	}
#endif
	memset(&o_Hints, 0, sizeof o_Hints);
	o_Hints.ai_family = PF_UNSPEC;
	o_Hints.ai_socktype = SOCK_STREAM;
	o_Hints.ai_flags = AI_PASSIVE;
	o_Hints.ai_protocol = IPPROTO_TCP;
	iServerStatus = getaddrinfo(p_IPPortPasswordInt->p_chIPNameBuffer, p_IPPortPasswordInt->p_chPortNameBuffer, &o_Hints, &p_Res);
	if(iServerStatus != 0)
	{
		LOG_P_0(LOG_CAT_E, "'getaddrinfo': " << gai_strerror(iServerStatus));
		RETVAL_SET(RETVAL_ERR);
		goto ex;
	}
	iListener = (int)socket(p_Res->ai_family, p_Res->ai_socktype, p_Res->ai_protocol); // Получение сокета для приёмника.
	if(iListener < 0 )
	{
		LOG_P_0(LOG_CAT_E, "'socket': "  << gai_strerror(iServerStatus));
		RETVAL_SET(RETVAL_ERR);
		goto ex;
	}
	iServerStatus = (int)bind(iListener, p_Res->ai_addr, (socklen_t)p_Res->ai_addrlen); // Привязка к указанному IP.
	if(iServerStatus < 0)
	{
		LOG_P_0(LOG_CAT_E, "'bind': " << gai_strerror(iServerStatus));
		RETVAL_SET(RETVAL_ERR);
		goto ex;
	}
	iServerStatus = listen(iListener, 10); // Запуск приёмника.
	if(iServerStatus < 0)
	{
		LOG_P_0(LOG_CAT_E, "'listen': " << gai_strerror(iServerStatus));
		RETVAL_SET(RETVAL_ERR);
		goto ex;
	}
	freeaddrinfo(p_Res);
	//
	LOG_P_1(LOG_CAT_I, "Accepting connections...");
	bListenerAlive = true;
	// Цикл ожидания входа клиентов.
nc:	bRequestNewConn = false; // Вход в звено цикла ожидания клиентов - сброс флага запроса.
	iCurrPos = FindFreeThrDadaPos();
	if(iCurrPos == NO_CONNECTION)
	{
		LOG_P_0(LOG_CAT_W, "Server is full.");
		pthread_create(&p_ThreadOverrunned, NULL,
					   ConversationThread, &iCurrPos);
	}
	else
	{
		LOG_P_1(LOG_CAT_I, "Free ID slot: " << iCurrPos);
		pthread_create(&mThreadDadas[iCurrPos].p_Thread, nullptr,
					   ConversationThread, &iCurrPos); // Создание нового потока приёмки.
	}
	while(!bExitSignal)
	{
		if(bRequestNewConn == true)
			goto nc;
		MSleep(USER_RESPONSE_MS);
	}
	LOG_P_2(LOG_CAT_I, "Terminated by admin.");
	// Закрытие приёмника.
	LOG_P_2(LOG_CAT_I, "Closing listener...");
#ifndef WIN32
	shutdown(iListener, SHUT_RDWR);
	close(iListener);
#else
	closesocket(iListener);
#endif
	while(bListenerAlive)
	{
		MSleep(USER_RESPONSE_MS);
	}
	LOG_P_2(LOG_CAT_I, "Listener is closed.");
	// Закрытие сокетов клиентов.
	LOG_P_2(LOG_CAT_I, "Disconnecting clients...");
	for(iCurrPos = 0; iCurrPos != MAX_CONN; iCurrPos++) // Закрываем все клиентские сокеты.
	{
		if(mThreadDadas[iCurrPos].bInUse == true)
		{
			SendToConnectionImmediately(iCurrPos, PROTO_S_SHUTDOWN_INFO);
		}
	}
	MSleep(WAITING_FOR_CLIENT_DSC * 2);
	pthread_mutex_lock(&ptConnMutex);
	for(iCurrPos = 0; iCurrPos != MAX_CONN; iCurrPos++) // Закрываем все клиентские сокеты.
	{
		if(mThreadDadas[iCurrPos].bInUse == true)
		{
			pthread_mutex_unlock(&ptConnMutex);
			FillIPAndPortNames(mThreadDadas[iCurrPos].oConnectionData, m_chIPNameBuffer, m_chPortNameBuffer);
			pthread_mutex_lock(&ptConnMutex);
#ifndef WIN32
			shutdown(mThreadDadas[iCurrPos].oConnectionData.iSocket, SHUT_RDWR);
			close(mThreadDadas[iCurrPos].oConnectionData.iSocket);
#else
			closesocket(mThreadDadas[iCurrPos].oConnectionData.iSocket);
#endif
			if(NetHub::CheckIPv4(m_chIPNameBuffer))
			{
				LOG_P_1(LOG_CAT_I, "Socket has been closed internally: " << m_chIPNameBuffer << ":" << m_chPortNameBuffer);
			}
			else
			{
				LOG_P_1(LOG_CAT_I, "Socket has been closed internally: [" << m_chIPNameBuffer << "]:" << m_chPortNameBuffer);
			}
		}
	}
	pthread_mutex_unlock(&ptConnMutex);
	LOG_P_2(LOG_CAT_I, "All clients are disconnected.");
ex:
#ifdef WIN32
	WSACleanup();
#endif
	LOG_P_1(LOG_CAT_I, "Exiting server thread.");
	bExitSignal = false;
	bServerAlive = false;
	RETURN_THREAD;
}
