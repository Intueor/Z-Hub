//== ВКЛЮЧЕНИЯ.
#include "element.h"
#include "main-window.h"

//== ФУНКЦИИ КЛАССОВ.
//== Класс элемента.
// Конструктор.
Element::Element(PSchElementBase& a_PSchElementBase)
{
	oPSchElementBase = a_PSchElementBase;
	if(oPSchElementBase.oPSchElementVars.ullIDInt == 0)
	{
		oPSchElementBase.oPSchElementVars.ullIDInt = GenerateID();
	}
	oPSchElementBase.oPSchElementVars.oSchElementGraph.bBusy = false;
	bNew = true;
	chTouchedBits = 0;
	p_Group = nullptr;
	// Если не был вписан в группу, а надо...
	if(oPSchElementBase.oPSchElementVars.ullIDGroup != 0)
	{
		for(uint uiG = 0; uiG < PBCountExternal(Group, Environment); uiG++)
		{
			Group* p_GroupInt = PBAccessExternal(Group, uiG, Environment);
			//
			if(p_GroupInt->oPSchGroupBase.oPSchGroupVars.ullIDInt == oPSchElementBase.oPSchElementVars.ullIDGroup)
			{
				p_Group = p_GroupInt;
				p_Group->vp_ConnectedElements.append(this);
				break;
			}
		}
	}
}
