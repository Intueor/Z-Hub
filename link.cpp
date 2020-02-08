//== ВКЛЮЧЕНИЯ.
#include "link.h"
#include "main-window.h"

//== ФУНКЦИИ КЛАССОВ.
//== Класс линка.
// Конструктор.
Link::Link(PSchLinkBase& a_PSchLinkBase)
{
	oPSchLinkBase = a_PSchLinkBase;
	bNew = true;
	p_SrcElement = nullptr;
	p_DstElement = nullptr;
	for(uint uiE = 0; uiE < PBCountExternal(Element, Environment); uiE++)
	{
		if(PBAccessExternal(Element, uiE, Environment)->oPSchElementBase.oPSchElementVars.ullIDInt == a_PSchLinkBase.oPSchLinkVars.ullIDSrc)
		{
			p_SrcElement = PBAccessExternal(Element, uiE, Environment);
		}
		else if(PBAccessExternal(Element, uiE, Environment)->oPSchElementBase.oPSchElementVars.ullIDInt ==
				a_PSchLinkBase.oPSchLinkVars.ullIDDst)
		{
			p_DstElement = PBAccessExternal(Element, uiE, Environment);
		}
	}
	if(p_SrcElement != nullptr && p_DstElement != nullptr)
	{
		p_SrcElement->vp_LinkedElements.append(p_DstElement);
		p_DstElement->vp_LinkedElements.append(p_SrcElement);
	}
}
