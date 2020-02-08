#ifndef LINK_H
#define LINK_H

//== ВКЛЮЧЕНИЯ.
#include "Server/protocol.h"
#include "element.h"

//== КЛАССЫ.
/// Класс линка.
class Link
{
public:
	/// Конструктор
	Link(PSchLinkBase& a_PSchLinkBase);
									///< \param[in] a_PSchLinkBase Ссылка на структуру с данными по линку.
public:
	bool bNew; ///< Признак созданного за цикл линка.
	char chTouchedBits; ///< Признаки затронутого линка за цикл.
	PSchLinkBase oPSchLinkBase; ///< Структура с данными по линку.
	Element* p_SrcElement; ///< Указатель на элемент-источник.
	Element* p_DstElement; ///< Указатель на элемент-приёмник.
};

#endif // LINK_H
