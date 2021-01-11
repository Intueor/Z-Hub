//== ВКЛЮЧЕНИЯ.
#include "proto-util.h"
#include <stdlib.h>

//== ФУНКЦИИ КЛАССОВ.
//== Класс хранилища пакета протокола.
// Конструктор.
ProtocolStorage::ProtocolStorage() : ushTypeCode(0)
{
	p_Data = nullptr;
}
// Инициализация хранилища.
void ProtocolStorage::Init(int iSize)
{
	p_Data = malloc(iSize);
}
// Освобождение хранилища.
void ProtocolStorage::Release()
{
	if(p_Data != nullptr)
	{
		free(p_Data);
		p_Data = nullptr;
	}
}
// Деструктор.
ProtocolStorage::~ProtocolStorage()
{
	Release();
}
