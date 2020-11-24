//== ВКЛЮЧЕНИЯ.
#include "group.h"
#include "p_buffer.h"
#include "main-hub.h"
#include "environment.h"

//== ФУНКЦИИ КЛАССОВ.
//== Класс группы.
// Конструктор.
Group::Group(PSchGroupBase& a_PSchGroupBase)
{
	oPSchGroupBase = a_PSchGroupBase;
	if(oPSchGroupBase.oPSchGroupVars.ullIDInt == 0)
	{
		oPSchGroupBase.oPSchGroupVars.ullIDInt = GenerateID();
	}
	oPSchGroupBase.oPSchGroupVars.oSchGroupGraph.bBusy = false;
}
