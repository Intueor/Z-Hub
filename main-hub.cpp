//== ВКЛЮЧЕНИЯ.
#include <QFileInfo>
#include "main-hub.h"
//
#include <random>
#include <cmath>

//== ФУНКЦИИ.
// Проверка на наличие файла.
bool IsFileExists(char *p_chPath)
{
	QFileInfo oCheckFile(p_chPath);
	//
	if (oCheckFile.exists() && oCheckFile.isFile())
		return true;
	return false;
}

// Копирование строчного массива в другой.
void CopyStrArray(char m_chFrom[], char m_chTo[], uint uiDestSize)
{
	strncpy(m_chTo, m_chFrom, uiDestSize);
	m_chTo[uiDestSize - 1] = 0;
}

// Проверка, содержит ли строка исключительно число.
bool IsStrInteger(char* p_chNumber, bool bIsHex)
{
	char m_chDec[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
	char m_chHex[] = {'a', 'b', 'c', 'd', 'e', 'f', 'A', 'B', 'C', 'D', 'E', 'F'};
	//
	while(*p_chNumber != 0)
	{
		for(int iP = 0; iP != 10; iP++)
		{
			if(*p_chNumber == m_chDec[iP])
			{
				goto gOk;
			}
		}
		if(bIsHex)
		{
			for(int iP = 0; iP != 12; iP++)
			{
				if(*p_chNumber == m_chHex[iP])
				{
					goto gOk;
				}
			}
		}
		return false;
gOk:    p_chNumber++;
	}
	return true;
}

// Заполнение структуры цифрового представления IP и порта из строк.
void FillNumericStructWithIPPortStrs(NumericAddress& a_NumericAddress, const QString& a_strIP, const QString& a_strPort)
{
	int iNumHelper, iCount, iDiff;
	QStringList l_strIPParts;
	unsigned char uchP;
	bool bIpSh;
	//
	a_NumericAddress.bIsCorrect = true;
	if(a_strIP.contains('.'))
	{
		a_NumericAddress.bIsIPv4 = true;
		l_strIPParts = a_strIP.split('.');
		if(l_strIPParts.count() != 4)
		{
gE:         a_NumericAddress.bIsCorrect = false;
			return;
		}
		for(unsigned char uchI = 0; uchI != 4; uchI++)
		{
			if(!IsStrInteger((char*)l_strIPParts[uchI].toStdString().c_str()))
			{
				goto gE;
			}
			iNumHelper = l_strIPParts[uchI].toInt();
			if((iNumHelper < 0) || (iNumHelper > 255))
			{
				goto gE;
			}
			else
			{
				a_NumericAddress.iIPNrs[uchI] = iNumHelper;
			}
		}
	}
	else
	{
		if(a_strIP.contains(':'))
		{
			a_NumericAddress.bIsIPv4 = false;
			l_strIPParts = a_strIP.split(':');
			iCount = l_strIPParts.count();
			if(iCount > 8)
			{
				goto gE;
			}
			iDiff = 9 - iCount;
			uchP = 0;
			bIpSh = false;
			for(unsigned char uchI = 0; uchI != 8; uchI++)
			{
				if(l_strIPParts[uchP] == "")
				{
					if(uchP == iCount - 1)
					{
						a_NumericAddress.iIPNrs[uchI] = 0;
						break;
					}
					if(bIpSh)
					{
						goto gE;
					}
					if(iDiff != 0)
					{
						a_NumericAddress.iIPNrs[uchI] = 0;
						if(uchP == 0)
						{
							uchP++;
							continue;
						}
						iDiff--;
						if(iDiff == 0)
						{
							uchP++;
							bIpSh = true;
						}
						continue;
					}
				}
				if(!IsStrInteger((char*)l_strIPParts[uchP].toStdString().c_str(), true))
				{
					goto gE;
				}
				iNumHelper = l_strIPParts[uchP].toInt(nullptr, 16);
				if((iNumHelper < 0) || (iNumHelper > 65535))
				{
					goto gE;
				}
				else
				{
					a_NumericAddress.iIPNrs[uchI] = iNumHelper;
				}
				uchP++;
			}
		}
		else
		{
			goto gE;
		}
	}
	if(!IsStrInteger((char*)a_strPort.toStdString().c_str()))
	{
		goto gE;
	}
	iNumHelper = a_strPort.toInt();
	if((iNumHelper < 0) || (iNumHelper > 65535))
	{
		goto gE;
	}
	else
	{
		a_NumericAddress.iPort = iNumHelper;
	}
}

// Создание уникального номера.
unsigned long long GenerateID()
{
	// Код генератора со "StackOverflow".
	std::random_device rd;
	std::mt19937_64 e2(rd());
	std::uniform_int_distribution<long long int> dist(std::llround(std::pow(2,61)), std::llround(std::pow(2,62)));
	//
	return (unsigned long long)dist(e2);
}
