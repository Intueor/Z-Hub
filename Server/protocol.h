#ifndef PROTOCOL_H
#define PROTOCOL_H

//== ВКЛЮЧЕНИЯ.
#include "proto-util.h"
#include "net-hub-defs.h"

//== МАКРОСЫ.
// ============================ ПОЛЬЗОВАТЕЛЬСКИЕ КОДЫ ПАКЕТОВ =================
#define PROTO_O_SCH_READY               1
#define PROTO_O_SCH_STATUS              2
#define PROTO_O_SCH_ELEMENT_BASE        3
#define PROTO_O_SCH_ELEMENT_VARS        4
#define PROTO_O_SCH_ELEMENT_NAME        5
#define PROTO_O_SCH_ELEMENT_COLOR		6
#define PROTO_O_SCH_ELEMENT_ERASE		7
#define PROTO_O_SCH_LINK_BASE           8
#define PROTO_O_SCH_LINK_VARS           9
#define PROTO_O_SCH_LINK_ERASE			10
#define PROTO_O_SCH_GROUP_BASE			11
#define PROTO_O_SCH_GROUP_VARS          12
#define PROTO_O_SCH_GROUP_NAME          13
#define PROTO_O_SCH_GROUP_COLOR			14
#define PROTO_O_SCH_GROUP_ERASE			15
#define PROTO_O_SCH_BROADCASTER_BASE    16
#define PROTO_O_SCH_BROADCASTER_VARS    17
#define PROTO_O_SCH_BROADCASTER_PORTS   18
#define PROTO_O_SCH_BROADCASTER_NAME	19
#define PROTO_O_SCH_BROADCASTER_COLOR	20
#define PROTO_O_SCH_BROADCASTER_ERASE	21
#define PROTO_O_SCH_RECEIVER_BASE		22
#define PROTO_O_SCH_RECEIVER_VARS		23
#define PROTO_O_SCH_RECEIVER_PORTS		24
#define PROTO_O_SCH_RECEIVER_NAME		25
#define PROTO_O_SCH_RECEIVER_COLOR		26
#define PROTO_O_SCH_RECEIVER_ERASE		27
//========================== ПРИВЯЗКА ТИПОВ ПАКЕТОВ ===========================
#define PocketTypesHub												\
CasePocket(PROTO_O_SCH_READY, PSchReadyFrame);						\
CasePocket(PROTO_O_SCH_STATUS, PSchStatusInfo);						\
CasePocket(PROTO_O_SCH_ELEMENT_BASE, PSchElementBase);				\
CasePocket(PROTO_O_SCH_ELEMENT_VARS, PSchElementVars);				\
CasePocket(PROTO_O_SCH_ELEMENT_NAME, PSchElementName);				\
CasePocket(PROTO_O_SCH_ELEMENT_COLOR, PSchElementColor);			\
CasePocket(PROTO_O_SCH_ELEMENT_ERASE, PSchElementEraser);			\
CasePocket(PROTO_O_SCH_GROUP_BASE, PSchGroupBase);					\
CasePocket(PROTO_O_SCH_GROUP_VARS, PSchGroupVars);					\
CasePocket(PROTO_O_SCH_GROUP_NAME, PSchGroupName);					\
CasePocket(PROTO_O_SCH_GROUP_COLOR, PSchGroupColor);				\
CasePocket(PROTO_O_SCH_GROUP_ERASE, PSchGroupEraser);				\
CasePocket(PROTO_O_SCH_LINK_BASE, PSchLinkBase);					\
CasePocket(PROTO_O_SCH_LINK_VARS, PSchLinkVars);					\
CasePocket(PROTO_O_SCH_LINK_ERASE, PSchLinkEraser);					\
CasePocket(PROTO_O_SCH_BROADCASTER_BASE, PSchBroadcasterBase);		\
CasePocket(PROTO_O_SCH_BROADCASTER_VARS, PSchBroadcasterVars);		\
CasePocket(PROTO_O_SCH_BROADCASTER_PORTS, PSchBroadcasterPorts);	\
CasePocket(PROTO_O_SCH_BROADCASTER_COLOR, PSchBroadcasterColor);	\
CasePocket(PROTO_O_SCH_BROADCASTER_NAME, PSchBroadcasterName);		\
CasePocket(PROTO_O_SCH_BROADCASTER_ERASE, PSchBroadcasterEraser);	\
CasePocket(PROTO_O_SCH_RECEIVER_BASE, PSchReceiverBase);			\
CasePocket(PROTO_O_SCH_RECEIVER_VARS, PSchReceiverVars);			\
CasePocket(PROTO_O_SCH_RECEIVER_PORTS, PSchReceiverPorts);			\
CasePocket(PROTO_O_SCH_RECEIVER_COLOR, PSchReceiverColor);			\
CasePocket(PROTO_O_SCH_RECEIVER_NAME, PSchReceiverName);			\
CasePocket(PROTO_O_SCH_RECEIVER_ERASE, PSchReceiverEraser);			\
//============================= РАЗНОЕ ДЛЯ ПАКЕТОВ ============================
#define BROADCASTER_AND_RECEIVER_PORTS	32
//============================ СТАНДАРТИЗАЦИЯ ОПРЕДЕЛЕНИЙ ==========================
#define _Sch_EGBR_Graph(type)			struct Sch##type##Graph{bool bMinimized; bool bHided; DbFrame oDbObjectFrame;						\
											unsigned char uchChangesBits; bool bBusy; double dbObjectZPos;}
#define _Sch_L_Graph					struct SchLinkGraph{DbPoint oDbSrcPortGraphPos; DbPoint oDbDstPortGraphPos;							\
											unsigned char uchChangesBits;}
#define _PSch_EGBR_Eraser(type)			struct PSch##type##Eraser{unsigned long long ullIDInt; bool bLastInQueue;}
#define _PSch_L_Eraser					struct PSchLinkEraser{unsigned long long ullIDSrc; unsigned long long ullIDDst;						\
											unsigned short int ushiSrcPort; unsigned short int ushiDstPort; bool bLastInQueue;}
#define _PSch_EGBR_Color(type)			struct PSch##type##Color{unsigned long long ullIDInt; unsigned int uiObjectBkgColor;				\
											bool bLastInQueue;}
#define _PSch_EGBR_Name(type)			struct PSch##type##Name{unsigned long long ullIDInt; char m_chName[SCH_OBJ_NAME_STR_LEN];			\
											bool bLastInQueue;}
#define _PSch_BR_Ports(type)			struct PSch##type##Ports{unsigned long long ullIDInt;												\
											unsigned short int m_ushiPorts[BROADCASTER_AND_RECEIVER_PORTS]; bool bLastInQueue;}
#define _PSch_EGBR_Vars(type)			struct PSch##type##Vars{unsigned long long ullIDInt; unsigned long long ullIDGroup;					\
											Sch##type##Graph oSch##type##Graph; bool bLastInQueue;}
#define _PSch_L_Vars					struct PSchLinkVars{unsigned long long ullIDSrc; unsigned long long ullIDDst;						\
											unsigned short int ushiSrcPort; unsigned short int ushiDstPort;									\
											SchLinkGraph oSchLinkGraph; bool bLastInQueue;}
#define _PSch_Obj_Base(type)			struct PSch##type##Base{PSch##type##Vars oPSch##type##Vars; char m_chName[SCH_OBJ_NAME_STR_LEN];	\
											unsigned int uiObjectBkgColor; bool bRequestGroupUpdate;}
#define _PSch_Link_Base					struct PSchLinkBase{PSchLinkVars oPSchLinkVars;}
#define _PSch_RB_Base(type)				struct PSch##type##Base{PSch##type##Vars oPSch##type##Vars;											\
											unsigned short int m_ushiPorts[BROADCASTER_AND_RECEIVER_PORTS];									\
											char m_chName[SCH_OBJ_NAME_STR_LEN]; unsigned int uiObjectBkgColor; bool bRequestGroupUpdate;}
//=========================== СТРУКТУРЫ ДЛЯ ПАКЕТОВ ===========================
//========================== ДОПОЛНИТЕЛЬНЫЕ СТРУКТУРЫ =========================
/// Структура определения точки.
struct DbPoint
{
	double dbX; ///< Коорд. X.
	double dbY; ///< Коорд. Y.
};
/// Структура определения фрейма.
struct DbFrame
{
	double dbX; ///< Коорд. X.
	double dbY; ///< Коорд. Y.
	double dbW; ///< Ширина.
	double dbH; ///< Высота.
};
/// Структура определения параметрического фрейма.
struct DbPFrame
{
	double dbX; ///< Коорд. X.
	double dbY; ///< Коорд. Y.
	double dbR; ///< Радиус.
};
/// Стандартизируемые определения.
_Sch_EGBR_Graph(Element);
_Sch_L_Graph;
_Sch_EGBR_Graph(Group);
_Sch_EGBR_Graph(Broadcaster);
_Sch_EGBR_Graph(Receiver);
//========================== ИСПОЛЬЗУЕМЫЕ СТРУКТУРЫ ===========================
/// Структура ответа готовности принятия фрейма схемы.
struct PSchReadyFrame
{
	DbFrame oDbFrame; ///< Фрейм.
};
/// Структура готовности Хаба к работе с клиентом.
struct PSchStatusInfo
{
	unsigned char uchBits; ///< Биты статуса.
};
/// Стандартизируемые определения.
_PSch_EGBR_Eraser(Element);
_PSch_L_Eraser;
_PSch_EGBR_Eraser(Group);
_PSch_EGBR_Eraser(Broadcaster);
_PSch_EGBR_Eraser(Receiver);
_PSch_EGBR_Color(Element);
_PSch_EGBR_Color(Group);
_PSch_EGBR_Color(Broadcaster);
_PSch_EGBR_Color(Receiver);
_PSch_EGBR_Name(Element);
_PSch_EGBR_Name(Group);
_PSch_EGBR_Name(Broadcaster);
_PSch_EGBR_Name(Receiver);
_PSch_BR_Ports(Broadcaster);
_PSch_BR_Ports(Receiver);
_PSch_EGBR_Vars(Element);
_PSch_L_Vars;
_PSch_EGBR_Vars(Group);
_PSch_EGBR_Vars(Broadcaster);
_PSch_EGBR_Vars(Receiver);
_PSch_Obj_Base(Element);
_PSch_Link_Base;
_PSch_Obj_Base(Group);
_PSch_RB_Base(Broadcaster);
_PSch_RB_Base(Receiver);

#endif // PROTOCOL_H
