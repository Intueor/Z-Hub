//== ВКЛЮЧЕНИЯ.
#include "link.h"
#include "main-window.h"

//== ФУНКЦИИ КЛАССОВ.
//== Класс линка.
// Конструктор.
Link::Link(PSchLinkBase& a_PSchLinkBase)
{
	oPSchLinkBase = a_PSchLinkBase;
	v_SrcObject = nullptr;
	v_DstObject = nullptr;
	switch(oPSchLinkBase.oPSchLinkVars.uchSrcType)
	{
		case SCH_LINK_TYPE_ELEMENT:
		{
			for(uint uiE = 0; uiE < PBCountExternal(Element, Environment); uiE++)
			{
				if(PBAccessExternal(Element, uiE, Environment)->oPSchElementBase.oPSchElementVars.ullIDInt ==
				   a_PSchLinkBase.oPSchLinkVars.ullIDSrc)
				{
					v_SrcObject = (void*)PBAccessExternal(Element, uiE, Environment);
				}
			}
			break;
		}
		case SCH_LINK_TYPE_BROADCASTER:
		{
			for(uint uiE = 0; uiE < PBCountExternal(Broadcaster, Environment); uiE++)
			{
				if(PBAccessExternal(Broadcaster, uiE, Environment)->oPSchBroadcasterBase.oPSchBroadcasterVars.ullIDInt ==
				   a_PSchLinkBase.oPSchLinkVars.ullIDSrc)
				{
					v_SrcObject = (void*)PBAccessExternal(Broadcaster, uiE, Environment);
				}
			}
			break;
		}
		case SCH_LINK_TYPE_RECEIVER:
		{
			for(uint uiE = 0; uiE < PBCountExternal(Receiver, Environment); uiE++)
			{
				if(PBAccessExternal(Receiver, uiE, Environment)->oPSchReceiverBase.oPSchReceiverVars.ullIDInt ==
				   a_PSchLinkBase.oPSchLinkVars.ullIDSrc)
				{
					v_SrcObject = (void*)PBAccessExternal(Receiver, uiE, Environment);
				}
			}
			break;
		}
	}
	switch(oPSchLinkBase.oPSchLinkVars.uchDstType)
	{
		case SCH_LINK_TYPE_ELEMENT:
		{
			for(uint uiE = 0; uiE < PBCountExternal(Element, Environment); uiE++)
			{
				if(PBAccessExternal(Element, uiE, Environment)->oPSchElementBase.oPSchElementVars.ullIDInt ==
				   a_PSchLinkBase.oPSchLinkVars.ullIDSrc)
				{
					v_DstObject = (void*)PBAccessExternal(Element, uiE, Environment);
				}
			}
			break;
		}
		case SCH_LINK_TYPE_BROADCASTER:
		{
			for(uint uiE = 0; uiE < PBCountExternal(Broadcaster, Environment); uiE++)
			{
				if(PBAccessExternal(Broadcaster, uiE, Environment)->oPSchBroadcasterBase.oPSchBroadcasterVars.ullIDInt ==
				   a_PSchLinkBase.oPSchLinkVars.ullIDSrc)
				{
					v_DstObject = (void*)PBAccessExternal(Broadcaster, uiE, Environment);
				}
			}
			break;
		}
		case SCH_LINK_TYPE_RECEIVER:
		{
			for(uint uiE = 0; uiE < PBCountExternal(Receiver, Environment); uiE++)
			{
				if(PBAccessExternal(Receiver, uiE, Environment)->oPSchReceiverBase.oPSchReceiverVars.ullIDInt ==
				   a_PSchLinkBase.oPSchLinkVars.ullIDSrc)
				{
					v_DstObject = (void*)PBAccessExternal(Receiver, uiE, Environment);
				}
			}
			break;
		}
	}
	if(v_SrcObject != nullptr && v_DstObject != nullptr)
	{
		switch(oPSchLinkBase.oPSchLinkVars.uchSrcType)
		{
			case SCH_LINK_TYPE_ELEMENT:
			{
				switch(oPSchLinkBase.oPSchLinkVars.uchDstType)
				{
					case SCH_LINK_TYPE_ELEMENT:
					{
						((Element*)v_SrcObject)->vp_LinkedElements.append((Element*)v_DstObject);
						break;
					}
					case SCH_LINK_TYPE_BROADCASTER:
					{
						((Element*)v_SrcObject)->vp_LinkedBroadcasters.append((Broadcaster*)v_DstObject);
						break;
					}
					case SCH_LINK_TYPE_RECEIVER:
					{
						((Element*)v_SrcObject)->vp_LinkedReceivers.append((Receiver*)v_DstObject);
						break;
					}
				}
				break;
			}
			case SCH_LINK_TYPE_BROADCASTER:
			{
				switch(oPSchLinkBase.oPSchLinkVars.uchDstType)
				{
					case SCH_LINK_TYPE_ELEMENT:
					{
						((Broadcaster*)v_SrcObject)->vp_LinkedElements.append((Element*)v_DstObject);
						break;
					}
					case SCH_LINK_TYPE_BROADCASTER:
					{
						((Broadcaster*)v_SrcObject)->vp_LinkedBroadcasters.append((Broadcaster*)v_DstObject);
						break;
					}
					case SCH_LINK_TYPE_RECEIVER:
					{
						((Broadcaster*)v_SrcObject)->vp_LinkedReceivers.append((Receiver*)v_DstObject);
						break;
					}
				}
				break;
			}
			case SCH_LINK_TYPE_RECEIVER:
			{
				switch(oPSchLinkBase.oPSchLinkVars.uchDstType)
				{
					case SCH_LINK_TYPE_ELEMENT:
					{
						((Receiver*)v_SrcObject)->vp_LinkedElements.append((Element*)v_DstObject);
						break;
					}
					case SCH_LINK_TYPE_BROADCASTER:
					{
						((Receiver*)v_SrcObject)->vp_LinkedBroadcasters.append((Broadcaster*)v_DstObject);
						break;
					}
					case SCH_LINK_TYPE_RECEIVER:
					{
						((Receiver*)v_SrcObject)->vp_LinkedReceivers.append((Receiver*)v_DstObject);
						break;
					}
				}
				break;
			}
		}
		switch(oPSchLinkBase.oPSchLinkVars.uchDstType)
		{
			case SCH_LINK_TYPE_ELEMENT:
			{
				switch(oPSchLinkBase.oPSchLinkVars.uchSrcType)
				{
					case SCH_LINK_TYPE_ELEMENT:
					{
						((Element*)v_DstObject)->vp_LinkedElements.append((Element*)v_SrcObject);
						break;
					}
					case SCH_LINK_TYPE_BROADCASTER:
					{
						((Element*)v_DstObject)->vp_LinkedBroadcasters.append((Broadcaster*)v_SrcObject);
						break;
					}
					case SCH_LINK_TYPE_RECEIVER:
					{
						((Element*)v_DstObject)->vp_LinkedReceivers.append((Receiver*)v_SrcObject);
						break;
					}
				}
				break;
			}
			case SCH_LINK_TYPE_BROADCASTER:
			{
				switch(oPSchLinkBase.oPSchLinkVars.uchSrcType)
				{
					case SCH_LINK_TYPE_ELEMENT:
					{
						((Broadcaster*)v_DstObject)->vp_LinkedElements.append((Element*)v_SrcObject);
						break;
					}
					case SCH_LINK_TYPE_BROADCASTER:
					{
						((Broadcaster*)v_DstObject)->vp_LinkedBroadcasters.append((Broadcaster*)v_SrcObject);
						break;
					}
					case SCH_LINK_TYPE_RECEIVER:
					{
						((Broadcaster*)v_DstObject)->vp_LinkedReceivers.append((Receiver*)v_SrcObject);
						break;
					}
				}
				break;
			}
			case SCH_LINK_TYPE_RECEIVER:
			{
				switch(oPSchLinkBase.oPSchLinkVars.uchSrcType)
				{
					case SCH_LINK_TYPE_ELEMENT:
					{
						((Receiver*)v_DstObject)->vp_LinkedElements.append((Element*)v_SrcObject);
						break;
					}
					case SCH_LINK_TYPE_BROADCASTER:
					{
						((Receiver*)v_DstObject)->vp_LinkedBroadcasters.append((Broadcaster*)v_SrcObject);
						break;
					}
					case SCH_LINK_TYPE_RECEIVER:
					{
						((Receiver*)v_DstObject)->vp_LinkedReceivers.append((Receiver*)v_SrcObject);
						break;
					}
				}
				break;
			}
		}
	}
}
