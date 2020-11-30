#ifndef RECEIVER_H
#define RECEIVER_H

//== ВКЛЮЧЕНИЯ.
#include <QVector>
#include "group.h"

//== ПРЕД-ДЕКЛАРАЦИИ.
class Group;
class Element;
class Broadcaster;
class Receiver;

//== КЛАССЫ.
/// Класс приёмника.
class Receiver
{
public:
	/// Конструктор.
	Receiver(PSchReceiverBase& a_PSchReceiverBase);
									///< \param[in] a_PSchReceiverBase Ссылка на структуру с данными приёмника.
public:
	PSchReceiverBase oPSchReceiverBase; ///< Структура с данными по приёмника.
	QVector<Element*> vp_LinkedElements; ///< Указатели на элементы-цели подключённых линков.
	QVector<Broadcaster*> vp_LinkedBroadcasters; ///< Указатели на трансляторы-цели подключённых линков.
	QVector<Receiver*> vp_LinkedReceivers; ///< Указатели на приёмники-цели подключённых линков.
	Group* p_Group; ///< Указатель на группу или nullptr.
};

#endif // RECEIVER_H
