#ifndef RECEIVER_H
#define RECEIVER_H

//== ВКЛЮЧЕНИЯ.
#include <QVector>
#include "group.h"

//== ПРЕД-ДЕКЛАРАЦИИ.
class Group;
class Element;

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
	QVector<Element*> vp_LinkedElements; ///< Указатели на элементы-приёмники подключённых линков.
	Group* p_Group; ///< Указатель на группу или nullptr.
};

#endif // RECEIVER_H
