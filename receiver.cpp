//== ВКЛЮЧЕНИЯ.
#include "receiver.h"
#include "main-window.h"

//== ФУНКЦИИ КЛАССОВ.
//== Класс приёмника.
// Конструктор.
Receiver::Receiver(PSchReceiverBase& a_PSchReceiverBase)
{
	oPSchReceiverBase = a_PSchReceiverBase;
	if(oPSchReceiverBase.oPSchReceiverVars.ullIDInt == 0)
	{
		oPSchReceiverBase.oPSchReceiverVars.ullIDInt = GenerateID();
	}
	oPSchReceiverBase.oPSchReceiverVars.oSchReceiverGraph.bBusy = false;
	p_Group = nullptr;
	// Если не был вписан в группу, а надо...
	if(oPSchReceiverBase.oPSchReceiverVars.ullIDGroup != 0)
	{
		for(uint uiG = 0; uiG < PBCountExternal(Group, Environment); uiG++)
		{
			Group* p_GroupInt = PBAccessExternal(Group, uiG, Environment);
			//
			if(p_GroupInt->oPSchGroupBase.oPSchGroupVars.ullIDInt == oPSchReceiverBase.oPSchReceiverVars.ullIDGroup)
			{
				p_Group = p_GroupInt;
				p_Group->vp_ConnectedReceivers.append(this);
				break;
			}
		}
	}
}
