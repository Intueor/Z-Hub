#ifndef PBUFFER_H
#define PBUFFER_H

//=== П-БУФЕР (оболочка обслуживания буфера указателей) ===
//
// << Наименования >>
// Type - имя класса\структуры для указателей.
// Name - расширение имени для использования множественных буферов одного Type (можно пропустить, сразу ставить запятую).
// TypeName - соединённое наименование.
// Size - количество указателей в буфере.
// Pointer - указатель на объект для буфера.
// Pos - позиция в буфере.
//
// << Макросы >>
// PBInit(Type,Name,Size)							- простая нициализация массива буфера и счётчика (далее - ПБ).
// StaticPBHeaderInit(Type,Name,Size)				- инициализация ПБ для исп. в статическом виде в заголовке класса (в описании класса).
// StaticPBSourceInit(Type,Name,Class,Size)			- инициализация ПБ для исп. в статическом виде в исходнике класса (вне функций класса).
// PBResetMem(TypeName)								- очистка памяти всего ПБ (без работы с указателями в нём).
// ReleasePB(TypeName)								- очистка ПБ с удалением всех объектов по указателям в нём и сбросом счётчика.
// AppendToPB(TypeName,Pointer)						- добавление указателя на объект выбранного типа в конец буфера.
// PBCount(TypeName)								- доступ к переменной счётчика элементов ПБ.
// PBAccess(TypeName,Pos)							- доступ к элементу ПБ в выбранной позиции.
// RemoveObjectFromPBByPos(TypeName,Pos)			- удаление по указ. в позиции ПБ с замещением его указ. с конца (при удалении из середины).
// RemoveObjectFromPBByPointer(TypeName,Pointer)	- то же, что и 'RemoveObjectFromPBByPos(Type,Pos)', но с поиском по указателю.

//== МАКРОСЫ.
#define RemoveObjectFromPBByPos(TypeName,Pos)						\
	delete mp_##TypeName##s[Pos];									\
	mp_##TypeName##s[Pos] = nullptr;								\
	ui##TypeName##s--;												\
	if((int)ui##TypeName##s != Pos)									\
	{																\
		mp_##TypeName##s[Pos] = mp_##TypeName##s[ui##TypeName##s];	\
		mp_##TypeName##s[ui##TypeName##s] = nullptr;				\
	}																\
	else mp_##TypeName##s[ui##TypeName##s] = nullptr;
#define RemoveObjectFromPBByPointer(TypeName,Pointer)				\
	for(int iP = 0; iP != (int)ui##TypeName##s; iP++)				\
	{																\
		if(mp_##TypeName##s[iP] == Pointer)							\
		{															\
			RemoveObjectFromPBByPos(TypeName,iP);					\
			break;													\
		}															\
	}
#define PBInit(Type,Name,Size)										\
	Type* mp_##Type##Name##s[Size];									\
	unsigned int ui##Type##Name##s;
#define StaticPBHeaderInit(Type,Name,Size)							\
	static Type* mp_##Type##Name##s[Size];							\
	static unsigned int ui##Type##Name##s;
#define StaticPBSourceInit(Type,Name,Class,Size)					\
	Type* Class::mp_##Type##Name##s[Size];							\
	unsigned int Class::ui##Type##Name##s = 0;
#define PBResetMem(TypeName) memset(mp_##TypeName##s, 0, sizeof(mp_##TypeName##s));
#define PBResetMemExternal(TypeName,Class) memset(Class::mp_##TypeName##s, 0, sizeof(Class::mp_##TypeName##s));
#define PBAccess(TypeName,Pos) mp_##TypeName##s[Pos]
#define PBAccessExternal(TypeName,Pos,Class) Class::mp_##TypeName##s[Pos]
#define PBCount(TypeName) ui##TypeName##s
#define PBCountExternal(TypeName,Class) Class::ui##TypeName##s
#define ReleasePB(TypeName)											\
	for(uint iP = 0; iP != ui##TypeName##s; iP++)					\
	{																\
			delete mp_##TypeName##s[iP];							\
			mp_##TypeName##s[iP] = nullptr;							\
	}																\
	ui##TypeName##s = 0;
#define ReleasePBExternal(TypeName,Class)							\
	for(uint iP = 0; iP != Class::ui##TypeName##s; iP++)			\
	{																\
			delete Class::mp_##TypeName##s[iP];						\
			Class::mp_##TypeName##s[iP] = nullptr;					\
	}																\
	Class::ui##TypeName##s = 0;
#define AppendToPB(TypeName,Pointer)								\
	mp_##TypeName##s[ui##TypeName##s] = Pointer;					\
	ui##TypeName##s++
#define AppendToPBExternal(TypeName,Pointer,Class)					\
	Class::mp_##TypeName##s[Class::ui##TypeName##s] = Pointer;		\
	Class::ui##TypeName##s++
#endif // PBUFFER_H
