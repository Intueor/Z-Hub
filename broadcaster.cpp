//== ВКЛЮЧЕНИЯ.
#include "broadcaster.h"
#include "main-window.h"

//== ФУНКЦИИ КЛАССОВ.
//== Класс элемента.
// Конструктор.
Broadcaster::Broadcaster(PSchBroadcasterBase& a_PSchBroadcasterBase)
{
	oPSchBroadcasterBase = a_PSchBroadcasterBase;
	if(oPSchBroadcasterBase.oPSchBroadcasterVars.ullIDInt == 0)
	{
		oPSchBroadcasterBase.oPSchBroadcasterVars.ullIDInt = GenerateID();
	}
	oPSchBroadcasterBase.oPSchBroadcasterVars.oSchBroadcasterGraph.bBusy = false;
	p_Group = nullptr;
	// Если не был вписан в группу, а надо...
	if(oPSchBroadcasterBase.oPSchBroadcasterVars.ullIDGroup != 0)
	{
		for(uint uiG = 0; uiG < PBCountExternal(Group, Environment); uiG++)
		{
			Group* p_GroupInt = PBAccessExternal(Group, uiG, Environment);
			//
			if(p_GroupInt->oPSchGroupBase.oPSchGroupVars.ullIDInt == oPSchBroadcasterBase.oPSchBroadcasterVars.ullIDGroup)
			{
				p_Group = p_GroupInt;
				p_Group->vp_ConnectedBroadcasters.append(this);
				break;
			}
		}
	}
}
