#ifndef LINK_H
#define LINK_H

//== ВКЛЮЧЕНИЯ.
#include "Server/protocol.h"

//== КЛАССЫ.
/// Класс линка.
class Link
{
public:
	/// Конструктор
	Link(PSchLinkBase& a_PSchLinkBase);
									///< \param[in] a_PSchLinkBase Ссылка на структуру с данными по линку.
};

#endif // LINK_H
