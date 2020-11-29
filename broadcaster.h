#ifndef BROADCASTER_H
#define BROADCASTER_H

//== ВКЛЮЧЕНИЯ.
#include <QVector>
#include "group.h"

//== ПРЕД-ДЕКЛАРАЦИИ.
class Group;

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
	Group* p_Group; ///< Указатель на группу или nullptr.
};

#endif // BROADCASTER_H
