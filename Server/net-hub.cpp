//== ВКЛЮЧЕНИЯ.
#include "net-hub.h"
#ifndef WIN32
#include <signal.h>
#endif
#include <cstring>

//== ДЕКЛАРАЦИИ СТАТИЧЕСКИХ ПЕРЕМЕННЫХ.
char NetHub::m_chDec[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
char NetHub::m_chHex[] = {'a', 'b', 'c', 'd', 'e', 'f', 'A', 'B', 'C', 'D', 'E', 'F'};
pthread_attr_t NetHub::_attr;

//== ФУНКЦИИ КЛАССОВ.
//== Класс хаба сетевых операций.
// Конструктор.
NetHub::NetHub() : m_chPocketsBuffer("")
{
	ResetPocketsBufferPositionPointer();
	memset(m_chPocketsBuffer, 0, sizeof(m_chPocketsBuffer));
}

// Сброс указателя позиции в буфере пакетов.
void NetHub::ResetPocketsBufferPositionPointer()
{
	p_chPocketsBufferPositionPointer = m_chPocketsBuffer;
}

// Проверка 'пустого' буфера пакетов.
bool NetHub::CheckIsBufferFree()
{
	return p_chPocketsBufferPositionPointer == m_chPocketsBuffer;
}

// Добавление пакета в буфер отправки.
bool NetHub::AddPocketToOutputBuffer(unsigned short ushCommand, char *p_chBuffer, int iLength)
{
	uint* p_uiCode;
	//
	if(iLength > (int)(MAX_DATA - 1 -
					   (unsigned long)(p_chPocketsBufferPositionPointer - m_chPocketsBuffer) - sizeof(unsigned short) - sizeof(int)))
	{
		return false;
	}
	p_uiCode = (uint*)p_chPocketsBufferPositionPointer;
	*p_uiCode = (uint)PROTOCOL_CODE;
	p_uiCode += 1;
	p_chPocketsBufferPositionPointer = (char*)p_uiCode;
	*(unsigned short*)p_chPocketsBufferPositionPointer = ushCommand;
	p_chPocketsBufferPositionPointer += sizeof(unsigned short);
	if((iLength > 0) && (p_chBuffer != nullptr))
		memcpy((void*)p_chPocketsBufferPositionPointer, (void*)p_chBuffer, (size_t)iLength);
	p_chPocketsBufferPositionPointer += iLength;
	return true;
}

// Отправка пакета адресату.
bool NetHub::SendToAddress(ConnectionData &oConnectionData, bool bResetPointer)
{
	int iLength;
#ifndef WIN32
	sigset_t ssOldset, ssNewset;
	siginfo_t sI;
	struct timespec tsTime = {0, 0};
	sigset_t* p_ssNewset;
	//
	p_ssNewset = &ssNewset;
	sigemptyset(&ssNewset);
	sigaddset(&ssNewset, SIGPIPE);
	pthread_sigmask(SIG_BLOCK, &ssNewset, &ssOldset);
	iLength = (int)(p_chPocketsBufferPositionPointer - m_chPocketsBuffer);
	oConnectionData.iStatus = (int)send(oConnectionData.iSocket, (const char*)m_chPocketsBuffer, (size_t)iLength, 0);
#else
	iLength = (int)(p_chPocketsBufferPositionPointer - m_chPocketsBuffer);
	oConnectionData.iStatus = send(oConnectionData.iSocket,
								   (const char*)m_chPocketsBuffer, (size_t)iLength, 0);
#endif
#ifndef WIN32
	while(sigtimedwait(p_ssNewset, &sI, &tsTime) >= 0 || errno != EAGAIN);
	pthread_sigmask(SIG_SETMASK, &ssOldset, nullptr);
#endif
	if(bResetPointer) ResetPocketsBufferPositionPointer();
	if(oConnectionData.iStatus == -1)
	{
		return false;
	}
	return true;
}

// Поиск свободного элемента хранилища пакетов.
int NetHub::FindFreeReceivedPocketsPos(ReceivedData* p_mReceivedPockets)
{
	uint uiPos = 0;
	for(; uiPos != S_MAX_STORED_POCKETS; uiPos++)
	{
		if(p_mReceivedPockets[uiPos].bBusy == false)
		{
			return (int)uiPos;
		}
	}
	return BUFFER_IS_FULL;
}

// Доступ к первому элементу заданного типа из массива принятых пакетов.
int NetHub::AccessSelectedTypeOfData(void** pp_vDataBuffer, ReceivedData* p_mReceivedPockets, unsigned short ushType)
{
	for(uint uiPos = 0; uiPos < S_MAX_STORED_POCKETS; uiPos++)
	{
		if(p_mReceivedPockets[uiPos].bBusy == true)
		{
			if(p_mReceivedPockets[uiPos].oProtocolStorage.ushTypeCode == ushType)
			{
				*pp_vDataBuffer = p_mReceivedPockets[uiPos].oProtocolStorage.p_Data;
				return (int)uiPos;
			}
		}
	}
	return DATA_NOT_FOUND;
}

// Удаление выбранного элемента в массиве принятых пакетов.
int NetHub::ReleaseDataInPosition(ReceivedData* p_mReceivedPockets, uint uiPos)
{
	if(p_mReceivedPockets[uiPos].bBusy == true)
	{
		p_mReceivedPockets[uiPos].bBusy = false;
		p_mReceivedPockets[uiPos].oProtocolStorage.Release();
		return RETVAL_OK;
	}
	return DATA_NOT_FOUND;
}

// Проверка на тип протокола IPv4 в буфере.
bool NetHub::CheckIPv4(char* p_chIPNameBuffer)
{
	int iDots = 0;
	//
gN: while(*p_chIPNameBuffer != 0)
	{
		if(*p_chIPNameBuffer == '.')
		{
			iDots++;
			if(iDots > 3)
			{
				return false;
			}
			p_chIPNameBuffer++;
			continue;
		}
		for(int iP = 0; iP != sizeof(m_chDec); iP++)
		{
			if(*p_chIPNameBuffer == m_chDec[iP])
			{
				p_chIPNameBuffer++;
				goto gN;
			}
		}
		return false;
	}
	return true;
}
