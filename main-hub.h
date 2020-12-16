#ifndef MAINHUB_H
#define MAINHUB_H

//== ВКЛЮЧЕНИЯ.
#include <QtGlobal>
#include "main-hub-defs.h"

//== СТРУКТУРЫ.
struct NumericAddress
{
	bool bIsCorrect; // Признак корректно принятых данных.
	bool bIsIPv4; // Признак протокола IPv4.
	int iIPNrs[8]; // Значения int по адресу IP.
	int iPort; // Значение int по порту.
};

//== ФУНКЦИИ.
/// Проверка на наличие файла.
bool IsFileExists(char *p_chPath);
						///< \param[in] p_chPath Указатель на массив строки с путём к файлу.
						///< \return true, при удаче.
/// Копирование строчного массива в другой.
void CopyStrArray(char m_chFrom[], char m_chTo[], uint uiDestSize);
												///< \param[in] m_chFrom Ссылка на массив откуда.
												///< \param[in] m_chTo Ссылка на массив куда.
												///< \param[in] uiDestSize Размер буфера доставки.
/// Проверка строки на содержание числа.
bool IsStrInteger(char* p_chNumber, bool bIsHex = false);
												///< \param[in] p_chNumber Указатель на массив строки с цифрой.
												///< \param[in] bIsHex Признак шестнадцатиричной системы.
												///< \return true, при удаче.
/// Заполнение структуры цифрового представления IP и порта из строк.
void FillNumericStructWithIPPortStrs(NumericAddress& a_NumericAddress, const QString &a_strIP, const QString &a_strPort);
												///< \param[out] a_NumericAddress Ссылка на структуру для заполнения.
												///< \param[in] a_strIP Ссылка на строку с IP.
												///< \param[in] a_strPort Ссылка на строку с портом.
/// Создание уникального номера.
unsigned long long GenerateID();
												///< \return Номер.
/// Установка бита в

#endif // MAINHUB_H
