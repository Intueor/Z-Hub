#ifndef GROUP_H
#define GROUP_H

//== ВКЛЮЧЕНИЯ.
#include <QVector>
#include "Server/protocol.h"
#include "element.h"

//== ПРЕД-ДЕКЛАРАЦИИ.
class Element;

//== КЛАССЫ.
/// Класс линка.
class Group
{
public:
	/// Конструктор
	Group(PSchGroupBase& a_PSchGroupBase);
									///< \param[in] a_PSchGroupBase Ссылка на структуру с данными по группе.
public:
	bool bNew; ///< Признак созданного за цикл элемента.
	char chTouchedBits; ///< Признаки затронутого элемента за цикл.
	PSchGroupBase oPSchGroupBase; ///< Структура с данными по группе.
	QVector<Element*> vp_ConnectedElements; ///< Вектор указателей на элементы, включённые в группу.
};

#endif // GROUP_H
