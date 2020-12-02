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
//============================ СТАНДАРТИЗАЦИЯ ОПРЕДЕЛЕНИЙ ==========================
#define _PSch_EG_Eraser(type)			struct PSch##type##Eraser{unsigned long long ullIDInt; bool bLastInQueue;}
#define _PSch_L_Eraser					struct PSchLinkEraser{unsigned long long ullIDSrc; unsigned long long ullIDDst;						\
											unsigned short int ushiSrcPort; unsigned short int ushiDstPort; bool bLastInQueue;}
#define _PSch_EG_Color(type)			struct PSch##type##Color{unsigned long long ullIDInt; unsigned int uiObjectBkgColor;				\
											bool bLastInQueue;}
#define _PSch_EG_Name(type)				struct PSch##type##Name{unsigned long long ullIDInt; char m_chName[SCH_OBJ_NAME_STR_LEN];			\
											bool bLastInQueue;}
#define _PSch_EG_Vars(type)				struct PSch##type##Vars{unsigned long long ullIDInt; unsigned long long ullIDGroup;					\
											SchEGGraph oSchEGGraph; bool bLastInQueue;}
#define _PSch_BR_Vars(type)				struct PSch##type##Vars{unsigned long long ullIDInt; unsigned long long ullIDGroup;					\
											SchBRGraph oSchBRGraph; bool bLastInQueue;}
#define _PSch_L_Vars					struct PSchLinkVars{unsigned long long ullIDSrc; unsigned char uchSrcType;							\
											unsigned long long ullIDDst; unsigned char uchDstType;											\
											unsigned short int ushiSrcPort; unsigned short int ushiDstPort;									\
											SchLGraph oSchLGraph; bool bLastInQueue;}
#define _PSch_EG_Base(type)				struct PSch##type##Base{PSch##type##Vars oPSch##type##Vars; char m_chName[SCH_OBJ_NAME_STR_LEN];	\
											unsigned int uiObjectBkgColor; bool bRequestGroupUpdate;}
#define _PSch_L_Base					struct PSchLinkBase{PSchLinkVars oPSchLinkVars;}
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
/// Структура определения графических качеств объекта схемы.
struct SchEGGraph
{
	bool bMinimized; ///< Признак свёрнутоого объекта.
	bool bHided; ///< Признак скрытого объекта.
	DbFrame oDbFrame; ///< Вмещающий прямоугольник.
	unsigned char uchChangesBits; ///< Байт с битами-признаками актуальных полей при изменении.
	bool bBusy; ///< Признак занятого объекта.
	double dbObjectZPos; ///< Z-позиция в схеме.
};
/// Структура определения графических качеств линка схемы.
struct SchLGraph
{
	DbPoint oDbSrcPortGraphPos; ///< Графическая позиция разъёма порта на периметре окна источника.
	DbPoint oDbDstPortGraphPos; ///< Графическая позиция разъёма порта на периметре окна приёмника.
	unsigned char uchChangesBits; ///< Байт с битами-признаками актуальных полей при изменении.
};
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
_PSch_EG_Eraser(Element);
_PSch_L_Eraser;
_PSch_EG_Eraser(Group);
_PSch_EG_Color(Element);
_PSch_EG_Color(Group);
_PSch_EG_Name(Element);
_PSch_EG_Name(Group);
_PSch_EG_Vars(Element);
_PSch_L_Vars;
_PSch_EG_Vars(Group);
_PSch_EG_Base(Element);
_PSch_L_Base;
_PSch_EG_Base(Group);

#endif // PROTOCOL_H
