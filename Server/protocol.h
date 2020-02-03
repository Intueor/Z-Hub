#ifndef PROTOCOL_H
#define PROTOCOL_H

//== ВКЛЮЧЕНИЯ.
#include "proto-util.h"
#include "net-hub-defs.h"

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
