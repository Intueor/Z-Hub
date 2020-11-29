#ifndef GROUP_H
#define GROUP_H

//== ВКЛЮЧЕНИЯ.
#include <QVector>
#include "Server/protocol.h"
#include "element.h"
#include "broadcaster.h"
#include "receiver.h"

//== ПРЕД-ДЕКЛАРАЦИИ.
class Element;
class Broadcaster;
class Receiver;

//== КЛАССЫ.
/// Класс линка.
class Group
{
public:
	/// Конструктор
	Group(PSchGroupBase& a_PSchGroupBase);
									///< \param[in] a_PSchGroupBase Ссылка на структуру с данными по группе.
public:
	PSchGroupBase oPSchGroupBase; ///< Структура с данными по группе.
	QVector<Element*> vp_ConnectedElements; ///< Вектор указателей на элементы, включённые в группу.
	QVector<Group*> vp_ConnectedGroups; ///< Вектор указателей на группы, включённые в группу.
	QVector<Broadcaster*> vp_ConnectedBroadcasters; ///< Вектор указателей на трансляторы, включённые в группу.
	QVector<Receiver*> vp_ConnectedReceivers; ///< Вектор указателей на приёмники, включённые в группу.
	Group* p_GroupAbove; ///< Указатель на группу или nullptr.
};

#endif // GROUP_H
