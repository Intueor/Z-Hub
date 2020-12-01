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
	PSchLinkBase oPSchLinkBase; ///< Структура с данными по линку.
	void* v_SrcObject; ///< Указатель на объект-источник, тип указан в oPSchLinkBase.
	void* v_DstObject; ///< Указатель на объект-приёмник, тип указан в oPSchLinkBase.
};

#endif // LINK_H
