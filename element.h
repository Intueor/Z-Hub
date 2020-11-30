#ifndef ELEMENT_H
#define ELEMENT_H

//== ВКЛЮЧЕНИЯ.
#include <QVector>
#include "group.h"

//== ПРЕД-ДЕКЛАРАЦИИ.
class Group;
class Broadcaster;
class Receiver;

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
	QVector<Broadcaster*> vp_LinkedBroadcasters; ///< Указатели на трансляторы-цели подключённых линков.
	QVector<Receiver*> vp_LinkedReceivers; ///< Указатели на приёмники-цели подключённых линков.
	Group* p_Group; ///< Указатель на группу или nullptr.
};

#endif // ELEMENT_H
