#ifndef PROTOCOL_H
#define PROTOCOL_H

//== ВКЛЮЧЕНИЯ.
#include "proto-util.h"
#include "net-hub-defs.h"

//== МАКРОСЫ.
// ============================ ПОЛЬЗОВАТЕЛЬСКИЕ КОДЫ ПАКЕТОВ =================
#define PROTO_O_SCH_READY               1
#define PROTO_O_SCH_ELEMENT_BASE        2
#define PROTO_O_SCH_ELEMENT_VARS        3
#define PROTO_O_SCH_ELEMENT_NAME        4
#define PROTO_O_SCH_ELEMENT_COLOR		5
#define PROTO_O_SCH_LINK_BASE           6
#define PROTO_O_SCH_LINK_VARS           7
#define PROTO_O_SCH_GROUP_BASE          8
#define PROTO_O_SCH_GROUP_VARS          9
#define PROTO_O_SCH_GROUP_NAME          10
#define PROTO_O_SCH_GROUP_COLOR			11
#define PROTO_O_SCH_ELEMENT_ERASE		12
#define PROTO_O_SCH_LINK_ERASE			13
#define PROTO_O_SCH_GROUP_ERASE			14
#define PROTO_O_SCH_STATUS              15
#define PROTO_O_SCH_BROADCASTER_BASE    16
#define PROTO_O_SCH_BROADCASTER_VARS    17
#define PROTO_O_SCH_BROADCASTER_PORTS   18
#define PROTO_O_SCH_BROADCASTER_COLOR	19
#define PROTO_O_SCH_BROADCASTER_ERASE	20
#define PROTO_O_SCH_BROADCASTER_NAME	21
#define PROTO_O_SCH_RECEIVER_BASE		22
#define PROTO_O_SCH_RECEIVER_VARS		23
#define PROTO_O_SCH_RECEIVER_PORTS		24
#define PROTO_O_SCH_RECEIVER_COLOR		25
#define PROTO_O_SCH_RECEIVER_ERASE		26
#define PROTO_O_SCH_RECEIVER_NAME		27

//========================== ПРИВЯЗКА ТИПОВ ПАКЕТОВ ===========================
#define PocketTypesHub												\
CasePocket(PROTO_O_SCH_GROUP_ERASE, PSchGroupEraser);				\
CasePocket(PROTO_O_SCH_LINK_ERASE, PSchLinkEraser);					\
CasePocket(PROTO_O_SCH_ELEMENT_ERASE, PSchElementEraser);			\
CasePocket(PROTO_O_SCH_GROUP_NAME, PSchGroupName);					\
CasePocket(PROTO_O_SCH_GROUP_VARS, PSchGroupVars);					\
CasePocket(PROTO_O_SCH_GROUP_BASE, PSchGroupBase);					\
CasePocket(PROTO_O_SCH_LINK_VARS, PSchLinkVars);					\
CasePocket(PROTO_O_SCH_LINK_BASE, PSchLinkBase);					\
CasePocket(PROTO_O_SCH_ELEMENT_NAME, PSchElementName);				\
CasePocket(PROTO_O_SCH_ELEMENT_VARS, PSchElementVars);				\
CasePocket(PROTO_O_SCH_ELEMENT_BASE, PSchElementBase);				\
CasePocket(PROTO_O_SCH_READY, PSchReadyFrame);						\
CasePocket(PROTO_O_SCH_STATUS, PSchStatusInfo);						\
CasePocket(PROTO_O_SCH_GROUP_COLOR, PSchGroupColor);				\
CasePocket(PROTO_O_SCH_ELEMENT_COLOR, PSchElementColor);			\
CasePocket(PROTO_O_SCH_BROADCASTER_NAME, PSchBroadcasterName);		\
CasePocket(PROTO_O_SCH_BROADCASTER_ERASE, PSchBroadcasterEraser);	\
CasePocket(PROTO_O_SCH_BROADCASTER_PORTS, PSchBroadcasterPorts);	\
CasePocket(PROTO_O_SCH_BROADCASTER_VARS, PSchBroadcasterVars);		\
CasePocket(PROTO_O_SCH_BROADCASTER_BASE, PSchBroadcasterBase);		\
CasePocket(PROTO_O_SCH_BROADCASTER_COLOR, PSchBroadcasterColor);	\
CasePocket(PROTO_O_SCH_RECEIVER_NAME, PSchReceiverName);			\
CasePocket(PROTO_O_SCH_RECEIVER_ERASE, PSchReceiverEraser);			\
CasePocket(PROTO_O_SCH_RECEIVER_PORTS, PSchReceiverPorts);			\
CasePocket(PROTO_O_SCH_RECEIVER_VARS, PSchReceiverVars);			\
CasePocket(PROTO_O_SCH_RECEIVER_BASE, PSchReceiverBase);			\
CasePocket(PROTO_O_SCH_RECEIVER_COLOR, PSchReceiverColor);			\

//============================= РАЗНОЕ ДЛЯ ПАКЕТОВ ============================
#define BROADCASTER_AND_RECEIVER_PORTS	32

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
struct SchElementGraph
{
	bool bMinimized; ///< Признак свёрнутоого элемента.
	bool bHided; ///< Признак скрытого элемента.
	DbFrame oDbObjectFrame; ///< Вмещающий прямоугольник.
	unsigned char uchChangesBits; ///< Байт с битами-признаками актуальных полей при изменении.
	bool bBusy; ///< Признак занятого элемента.
	double dbObjectZPos; ///< Z-позиция в схеме.
};
/// Структура определения графических качеств линка схемы.
struct SchLinkGraph
{
	DbPoint oDbSrcPortGraphPos; ///< Графическая позиция разъёма порта на периметре окна источника.
	DbPoint oDbDstPortGraphPos; ///< Графическая позиция разъёма порта на периметре окна приёмника.
	unsigned char uchChangesBits; ///< Байт с битами-признаками актуальных полей при изменении.
};
/// Структура определения графических качеств объекта группы.
struct SchGroupGraph
{
	bool bMinimized; ///< Признак свёрнутой группы.
	bool bHided; ///< Признак скрытой группы.
	DbFrame oDbObjectFrame; ///< Вмещающий прямоугольник.
	unsigned char uchChangesBits; ///< Байт с битами-признаками актуальных полей при изменении.
	bool bBusy; ///< Признак занятой группы.
	double dbObjectZPos; ///< Z-позиция в схеме.
};
//========================== ИСПОЛЬЗУЕМЫЕ СТРУКТУРЫ ===========================
/// Структура готовности Хаба к работе с клиентом.
struct PSchStatusInfo
{
	unsigned char uchBits; ///< Биты статуса.
};
/// Структура команды удаления группы.
struct PSchGroupEraser
{
	unsigned long long ullIDInt; ///< Уникальный номер группы.
	bool bLastInQueue; ///< Признак последнего пункта в цепочке ответов.
};
/// Структура команды удаления линка.
struct PSchLinkEraser
{
	unsigned long long ullIDSrc; ///< Номер элемента источника.
	unsigned long long ullIDDst; ///< Номер элемента приёмника.
	unsigned short int ushiSrcPort; ///< Порт на источнике.
	unsigned short int ushiDstPort; ///< Порт на приёмнике.
	bool bLastInQueue; ///< Признак последнего пункта в цепочке ответов.
};
/// Структура команды удаления элемента.
struct PSchElementEraser
{
	unsigned long long ullIDInt; ///< Уникальный номер элемента.
	bool bLastInQueue; ///< Признак последнего пункта в цепочке ответов.
};
/// Структура команды смены цвета группы.
struct PSchGroupColor
{
	unsigned long long ullIDInt; ///< Уникальный номер группы.
	unsigned int uiObjectBkgColor; ///< Цвет подложки.
	bool bLastInQueue; ///< Признак последнего пункта в цепочке ответов.
};
/// Структура команды смены цвета элемента.
struct PSchElementColor
{
	unsigned long long ullIDInt; ///< Уникальный номер элемента.
	unsigned int uiObjectBkgColor; ///< Цвет подложки.
	bool bLastInQueue; ///< Признак последнего пункта в цепочке ответов.
};
/// Структура для передачи изменений в имени группы.
struct PSchGroupName
{
	unsigned long long ullIDInt; ///< Уникальный номер группы.
	char m_chName[SCH_OBJ_NAME_STR_LEN]; ///< Буфер текста имени.
	bool bLastInQueue; ///< Признак последнего пункта в цепочке ответов.
};
/// Структура для передачи качеств группы схемы с указанием актуальных для изменения полей.
struct PSchGroupVars
{
	unsigned long long ullIDInt; ///< Уникальный номер группы.
	unsigned long long ullIDGroup; ///< Номер группы группы или 0.
	SchGroupGraph oSchGroupGraph; ///< Объект структуры определения графических качеств группы.
	bool bLastInQueue; ///< Признак последнего пункта в цепочке ответов.
};
/// Структура для передачи группы.
struct PSchGroupBase
{
	PSchGroupVars oPSchGroupVars; ///< Структура для передачи качеств группы.
	char m_chName[SCH_OBJ_NAME_STR_LEN]; ///< Буфер текста имени.
	unsigned int uiObjectBkgColor; ///< Цвет подложки.
	bool bRequestGroupUpdate; ///< Флаг запроса обновления геометрии соответствующей группы после принятия новой группы.
};
/// Структура для передачи качеств линка схемы с указанием актуальных для изменения полей.
struct PSchLinkVars
{
	unsigned long long ullIDSrc; ///< Номер элемента источника.
	unsigned long long ullIDDst; ///< Номер элемента приёмника.
	unsigned short int ushiSrcPort; ///< Порт на источнике.
	unsigned short int ushiDstPort; ///< Порт на приёмнике.
	SchLinkGraph oSchLinkGraph; ///< Описания графических качеств линка схемы.
	bool bLastInQueue; ///< Признак последнего пункта в цепочке ответов.
};
/// Структура для передачи линка схемы.
struct PSchLinkBase
{
	PSchLinkVars oPSchLinkVars;
};
/// Структура для передачи изменений в имени элемента.
struct PSchElementName
{
	unsigned long long ullIDInt; ///< Уникальный номер элемента.
	char m_chName[SCH_OBJ_NAME_STR_LEN]; ///< Буфер текста имени.
	bool bLastInQueue; ///< Признак последнего пункта в цепочке ответов.
};
/// Структура для передачи качеств элемента схемы с указанием актуальных для изменения полей.
struct PSchElementVars
{
	unsigned long long ullIDInt; ///< Уникальный номер элемента.
	unsigned long long ullIDGroup; ///< Номер группы элемента или 0.
	SchElementGraph oSchElementGraph; ///< Объект структуры определения графических качеств элемента схемы.
	bool bLastInQueue; ///< Признак последнего пункта в цепочке ответов.
};
/// Структура для передачи элемента схемы.
struct PSchElementBase
{
	PSchElementVars oPSchElementVars; ///< Структура для передачи качеств элемента схемы.
	char m_chName[SCH_OBJ_NAME_STR_LEN]; ///< Буфер текста имени.
	unsigned int uiObjectBkgColor; ///< Цвет подложки.
	bool bRequestGroupUpdate; ///< Флаг запроса обновления геометрии соответствующей группы после принятия нового элемента.
};
/// Структура для передачи изменений в имени транслятора.
struct PSchBroadcasterName
{
	unsigned long long ullIDInt; ///< Уникальный номер транслятора.
	char m_chName[SCH_OBJ_NAME_STR_LEN]; ///< Буфер текста имени.
	bool bLastInQueue; ///< Признак последнего пункта в цепочке ответов.
};
/// Структура для передачи изменений портов транслятора.
struct PSchBroadcasterPorts
{
	unsigned long long ullIDInt; ///< Уникальный номер транслятора.
	unsigned short int m_ushiBroadcastPorts[BROADCASTER_AND_RECEIVER_PORTS]; ///< Порты транслятора.
	bool bLastInQueue; ///< Признак последнего пункта в цепочке ответов.
};
/// Структура определения графических качеств объекта схемы.
struct SchBroadcasterGraph
{
	bool bMinimized; ///< Признак свёрнутоого транслятора.
	bool bHided; ///< Признак скрытого транслятора.
	DbFrame oDbObjectFrame; ///< Вмещающий прямоугольник.
	unsigned char uchChangesBits; ///< Байт с битами-признаками актуальных полей при изменении.
	bool bBusy; ///< Признак занятого транслятора.
	double dbObjectZPos; ///< Z-позиция в схеме.
};
/// Структура команды смены цвета транслятора.
struct PSchBroadcasterColor
{
	unsigned long long ullIDInt; ///< Уникальный номер транслятора.
	unsigned int uiObjectBkgColor; ///< Цвет подложки.
	bool bLastInQueue; ///< Признак последнего пункта в цепочке ответов.
};
/// Структура команды удаления транслятора.
struct PSchBroadcasterEraser
{
	unsigned long long ullIDInt; ///< Уникальный номер транслятора.
	bool bLastInQueue; ///< Признак последнего пункта в цепочке ответов.
};
/// Структура для передачи качеств транслятора схемы с указанием актуальных для изменения полей.
struct PSchBroadcasterVars
{
	unsigned long long ullIDInt; ///< Уникальный номер транслятора.
	unsigned long long ullIDGroup; ///< Номер группы транслятора или 0.
	SchBroadcasterGraph oSchBroadcasterGraph; ///< Объект структуры определения графических качеств транслятора схемы.
	bool bLastInQueue; ///< Признак последнего пункта в цепочке ответов.
};
/// Структура для передачи транслятора схемы.
struct PSchBroadcasterBase
{
	PSchBroadcasterVars oPSchBroadcasterVars; ///< Структура для передачи качеств транслятора схемы.
	unsigned short int m_ushiBroadcastPorts[BROADCASTER_AND_RECEIVER_PORTS]; ///< Порты транслятора.
	char m_chName[SCH_OBJ_NAME_STR_LEN]; ///< Буфер текста имени.
	unsigned int uiObjectBkgColor; ///< Цвет подложки.
	bool bRequestGroupUpdate; ///< Флаг запроса обновления геометрии соответствующей группы после принятия нового транслятора.
};
/// Структура для передачи изменений в имени приёмника.
struct PSchReceiverName
{
	unsigned long long ullIDInt; ///< Уникальный номер приёмника.
	char m_chName[SCH_OBJ_NAME_STR_LEN]; ///< Буфер текста имени.
	bool bLastInQueue; ///< Признак последнего пункта в цепочке ответов.
};
/// Структура для передачи изменений портов приёмника.
struct PSchReceiverPorts
{
	unsigned long long ullIDInt; ///< Уникальный номер приёмника.
	unsigned short int m_ushiReceiverPorts[BROADCASTER_AND_RECEIVER_PORTS]; ///< Порты приёмника.
	bool bLastInQueue; ///< Признак последнего пункта в цепочке ответов.
};
/// Структура определения графических качеств объекта схемы.
struct SchReceiverGraph
{
	bool bMinimized; ///< Признак свёрнутоого приёмника.
	bool bHided; ///< Признак скрытого приёмника.
	DbFrame oDbObjectFrame; ///< Вмещающий прямоугольник.
	unsigned char uchChangesBits; ///< Байт с битами-признаками актуальных полей при изменении.
	bool bBusy; ///< Признак занятого приёмника.
	double dbObjectZPos; ///< Z-позиция в схеме.
};
/// Структура команды смены цвета приёмника.
struct PSchReceiverColor
{
	unsigned long long ullIDInt; ///< Уникальный номер приёмника.
	unsigned int uiObjectBkgColor; ///< Цвет подложки.
	bool bLastInQueue; ///< Признак последнего пункта в цепочке ответов.
};
/// Структура команды удаления приёмника.
struct PSchReceiverEraser
{
	unsigned long long ullIDInt; ///< Уникальный номер приёмника.
	bool bLastInQueue; ///< Признак последнего пункта в цепочке ответов.
};
/// Структура для передачи качеств приёмника схемы с указанием актуальных для изменения полей.
struct PSchReceiverVars
{
	unsigned long long ullIDInt; ///< Уникальный номер приёмника.
	unsigned long long ullIDGroup; ///< Номер группы приёмника или 0.
	SchReceiverGraph oSchReceiverGraph; ///< Объект структуры определения графических качеств приёмника схемы.
	bool bLastInQueue; ///< Признак последнего пункта в цепочке ответов.
};
/// Структура для передачи приёмника схемы.
struct PSchReceiverBase
{
	PSchReceiverVars oPSchReceiverVars; ///< Структура для передачи качеств приёмника схемы.
	unsigned short int m_ushiReceiverPorts[BROADCASTER_AND_RECEIVER_PORTS]; ///< Порты приёмника.
	char m_chName[SCH_OBJ_NAME_STR_LEN]; ///< Буфер текста имени.
	unsigned int uiObjectBkgColor; ///< Цвет подложки.
	bool bRequestGroupUpdate; ///< Флаг запроса обновления геометрии соответствующей группы после принятия нового приёмника.
};
/// Структура ответа готовности принятия фрейма схемы.
struct PSchReadyFrame
{
	DbFrame oDbFrame; ///< Фрейм.
};

#endif // PROTOCOL_H
