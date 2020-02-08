#ifndef ELEMENT_H
#define ELEMENT_H

//== ВКЛЮЧЕНИЯ.
#include "Server/protocol.h"

//== КЛАССЫ.
/// Класс элемента.
class Element
{
public:
	/// Конструктор
	Element(PSchElementBase& a_PSchElementBase);
									///< \param[in] a_PSchElementBase Ссылка на структуру с данными по элементу.
public:
	PSchElementBase oPSchElementBase; ///< Структура с данными по элементу.
};

#endif // ELEMENT_H
