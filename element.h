#ifndef ELEMENT_H
#define ELEMENT_H

//== ВКЛЮЧЕНИЯ.
#include <QVector>
#include "group.h"

//== ПРЕД-ДЕКЛАРАЦИИ.
class Group;

//== КЛАССЫ.
/// Класс элемента.
class Element
{
public:
	/// Конструктор.
	Element(PSchElementBase& a_PSchElementBase);
									///< \param[in] a_PSchElementBase Ссылка на структуру с данными по элементу.
public:
	PSchElementBase oPSchElementBase; ///< Структура с данными по элементу.
	QVector<Element*> vp_LinkedElements; ///< Указатели на элементы-цели подключённых линков.
	Group* p_Group; ///< Указатель на группу или nullptr.
};

#endif // ELEMENT_H
