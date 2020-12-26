#ifndef GROUP_H
#define GROUP_H

//== ВКЛЮЧЕНИЯ.
#include <QVector>
#include "Server/protocol.h"
#include "element.h"

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
	Group(PSchGroupBase& a_PSchGroupBase, bool bConnectWithParent = true);
									///< \param[in] a_PSchGroupBase Ссылка на структуру с данными по группе.
									///< \param[in] bConnectWithParent false - в прогрузке сцены, там зависимости расставляются после созд. групп.
public:
	PSchGroupBase oPSchGroupBase; ///< Структура с данными по группе.
	QVector<Element*> vp_ConnectedElements; ///< Вектор указателей на элементы, включённые в группу.
	QVector<Group*> vp_ConnectedGroups; ///< Вектор указателей на группы, включённые в группу.
	Group* p_GroupAbove; ///< Указатель на группу или nullptr.
};

#endif // GROUP_H
