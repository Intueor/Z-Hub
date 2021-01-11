#ifndef PSEUDONYM_H
#define PSEUDONYM_H

//== ВКЛЮЧЕНИЯ.
#include "Server/protocol.h"

//== КЛАССЫ.
/// Класс псевдонима.
class Pseudonym
{
public:
	/// Конструктор.
	Pseudonym(PSchPseudonym& a_PSchPseudonym);
					///< \param[in] a_PSchPseudonymBase Ссылка на структуру с данными по псевдониму.
public:
	PSchPseudonym oPSchPseudonym; ///< Структура с данными по псевдониму.
};

#endif // PSEUDONYM_H
