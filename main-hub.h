#ifndef MAINHUB_H
#define MAINHUB_H

//== ВКЛЮЧЕНИЯ.
#include <QtGlobal>
#include "main-hub-defs.h"

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

/// Создание уникального номера.
unsigned long long GenerateID();
												///< \return Номер.

#endif // MAINHUB_H
