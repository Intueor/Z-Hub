#ifndef GROUP_H
#define GROUP_H

//== ВКЛЮЧЕНИЯ.
#include "Server/protocol.h"

//== КЛАССЫ.
/// Класс линка.
class Group
{
public:
	/// Конструктор
	Group(PSchGroupBase& a_PSchGroupBase);
									///< \param[in] a_PSchGroupBase Ссылка на структуру с данными по группе.
};

#endif // GROUP_H
