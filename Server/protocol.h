#ifndef PROTOCOL_H
#define PROTOCOL_H

//== ВКЛЮЧЕНИЯ.
#include "proto-util.h"
#include "net-hub-defs.h"

//== МАКРОСЫ.
// ============================ ПОЛЬЗОВАТЕЛЬСКИЕ КОДЫ ПАКЕТОВ ==================================
#define PROTO_C_SCH_READY               1
#define PROTO_O_SCH_ELEMENT_BASE        2
#define PROTO_O_SCH_ELEMENT_VARS        3
#define PROTO_O_SCH_ELEMENT_NAME        4
#define PROTO_O_SCH_LINK_BASE           5
#define PROTO_O_SCH_LINK_VARS           6
#define PROTO_O_SCH_GROUP_BASE          7
#define PROTO_O_SCH_GROUP_VARS          8
#define PROTO_O_SCH_GROUP_NAME          9
#define PROTO_O_SCH_ELEMENT_ERASE		10
#define PROTO_O_SCH_LINK_ERASE			11
#define PROTO_O_SCH_GROUP_ERASE			12
#define PROTO_O_SCH_STATUS               13

//========================== ПРИВЯЗКА ТИПОВ ПАКЕТОВ =========================
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
CasePocket(PROTO_C_SCH_READY, PSchReadyFrame);						\
CasePocket(PROTO_O_SCH_STATUS, PSchStatusInfo);						\

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
	unsigned int uiObjectBkgColor; ///< Цвет подложки.
	DbFrame oDbObjectFrame; ///< Вмещающий прямоугольник.
	DbPoint oDbObjectPos; ///< Позиция в схеме.
	unsigned char uchChangesBits; ///< Байт с битами-признаками актуальных полей при изменении.
	bool bBusy; ///< Признак занятого объекта.
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
	unsigned int uiObjectBkgColor; ///< Цвет подложки.
	DbFrame oDbObjectFrame; ///< Вмещающий прямоугольник.
	unsigned char uchChangesBits; ///< Байт с битами-признаками актуальных полей при изменении.
	bool bBusy; ///< Признак занятого объекта.
	double dbObjectZPos; ///< Z-позиция в схеме.
};
//========================== ИСПОЛЬЗУЕМЫЕ СТРУКТУРЫ ===========================
/// Структура готовности Хаба к работе с клиентом.
struct PSchStatusInfo
{
	bool bReady; ///< Признак готовности.
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
	bool bLastInQueue; ///< Признак последнего пункта в цепочке ответов.
};
/// Структура команды удаления элемента.
struct PSchElementEraser
{
	unsigned long long ullIDInt; ///< Уникальный номер элемента.
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
	SchGroupGraph oSchGroupGraph; ///< Объект структуры определения графических качеств группы.
	bool bLastInQueue; ///< Признак последнего пункта в цепочке ответов.
};
/// Структура для передачи группы.
struct PSchGroupBase
{
	PSchGroupVars oPSchGroupVars; ///< Структура для передачи качеств группы.
	char m_chName[SCH_OBJ_NAME_STR_LEN]; ///< Буфер текста имени.
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
	bool bRequestGroupUpdate; ///< Флаг запроса обновления геометрии соответствующей группы после принятия нового элемента.
};

/// Структура ответа готовности принятия фрейма схемы.
struct PSchReadyFrame
{
	DbFrame oDbFrame; ///< Фрейм.
};

#endif // PROTOCOL_H
