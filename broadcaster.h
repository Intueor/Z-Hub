#ifndef BROADCASTER_H
#define BROADCASTER_H

//== ВКЛЮЧЕНИЯ.
#include <QVector>
#include "group.h"

//== ПРЕД-ДЕКЛАРАЦИИ.
class Group;
class Element;
class Broadcaster;
class Receiver;

//== КЛАССЫ.
/// Класс транслятора.
class Broadcaster
{
public:
	/// Конструктор.
	Broadcaster(PSchBroadcasterBase& a_PSchBroadcasterBase);
									///< \param[in] a_PSchBroadcasterBase Ссылка на структуру с данными по транслятору.
public:
	PSchBroadcasterBase oPSchBroadcasterBase; ///< Структура с данными по транслятору.
	QVector<Element*> vp_LinkedElements; ///< Указатели на элементы-цели подключённых линков.
	QVector<Broadcaster*> vp_LinkedBroadcasters; ///< Указатели на трансляторы-цели подключённых линков.
	QVector<Receiver*> vp_LinkedReceivers; ///< Указатели на приёмники-цели подключённых линков.
	Group* p_Group; ///< Указатель на группу или nullptr.
};

#endif // BROADCASTER_H
