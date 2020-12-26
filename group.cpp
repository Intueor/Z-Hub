//== ВКЛЮЧЕНИЯ.
#include "group.h"
#include "p_buffer.h"
#include "main-hub.h"
#include "environment.h"

//== ФУНКЦИИ КЛАССОВ.
//== Класс группы.
// Конструктор.
Group::Group(PSchGroupBase& a_PSchGroupBase, bool bConnectWithParent)
{
	oPSchGroupBase = a_PSchGroupBase;
	if(oPSchGroupBase.oPSchGroupVars.ullIDInt == 0)
	{
		oPSchGroupBase.oPSchGroupVars.ullIDInt = GenerateID();
	}
	p_GroupAbove = nullptr;
	if(bConnectWithParent)
	{
		// Если не был вписан в группу, а надо...
		if(oPSchGroupBase.oPSchGroupVars.ullIDGroup != 0)
		{
			for(uint uiG = 0; uiG < PBCountExternal(Group, Environment); uiG++)
			{
				Group* p_GroupInt = PBAccessExternal(Group, uiG, Environment);
				//
				if(p_GroupInt->oPSchGroupBase.oPSchGroupVars.ullIDInt == oPSchGroupBase.oPSchGroupVars.ullIDGroup)
				{
					p_GroupAbove = p_GroupInt;
					p_GroupAbove->vp_ConnectedGroups.append(this);
					break;
				}
			}
		}
	}
}
