#ifndef PROTOCOL_H
#define PROTOCOL_H

//== МАКРОСЫ.
#define MSG_STR_LEN				128
#define SCH_OBJ_NAME_STR_LEN    64
#define WORLD_FILENAME_STR_LEN	64
#define _NMG					-32768		// !!! Текущий свободный номер _NMG-9 !!!
#define PROTOCOL_CODE			314159265358

//== ВКЛЮЧЕНИЯ.
#include "proto-util.h"

// ============================ ПОЛЬЗОВАТЕЛЬСКИЕ КОДЫ ПАКЕТОВ ==================================
//#define PROTO_O_TEXT_MSG				1

//========================== ПРИВЯЗКА ТИПОВ ПАКЕТОВ =========================
#define PocketTypesHub												\
//CasePocket(PROTO_O_TEXT_MSG, PTextMsg);

//========================== СТРУКТУРЫ ДЛЯ ПАКЕТОВ =========================
/// Структура текстового сообщения.
//struct PTextMsg
//{
//	char m_chLogin[AUTH_LOGIN_STR_LEN]; ///< Буфер ника.
//	char m_chMsg[MSG_STR_LEN]; ///< Буфер сообщения.
//};

#endif // PROTOCOL_H
