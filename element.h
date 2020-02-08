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
	/// Конструктор
	Element(PSchElementBase& a_PSchElementBase);
									///< \param[in] a_PSchElementBase Ссылка на структуру с данными по элементу.

public:
	bool bNew; ///< Признак созданного за цикл элемента.
	char chTouchedBits; ///< Признаки затронутого элемента за цикл.
	PSchElementBase oPSchElementBase; ///< Структура с данными по элементу.
	QVector<Element*> vp_LinkedElements; ///< Указатели на элементы-приёмники подключённых линков (для подгрузки вне окна обзора).
	Group* p_Group; ///< Указатель на группу или nullptr.
};

#endif // ELEMENT_H
