#ifndef PROTO_UTIL_H
#define PROTO_UTIL_H

//== МАКРОСЫ.
#define SERVER_NAME_STR_LEN		64
#define AUTH_PASSWORD_STR_LEN	64
// Утиль.
#define _PPControlSize(name)			iSizeOfStructure = (int)sizeof(name);													\
										if(iSizeOfStructure < iCurrentLength)                                                   \
										{																						\
											iCurrentLength = iSizeOfStructure;                                                  \
											bOutOfRange = true;																	\
										} // Если зашкалило, то длина ставится по нормальной длине первого элемента.
#define _PCControlSize                  if(iCurrentLength > 0)																	\
										{                                                                                       \
											iCurrentLength = 0;                                                                 \
											bOutOfRange = true;																	\
										} // То же, но с длиной в 0 для команд.
// _FillNewStructure(имя структуры в протоколе)
// - копирует данные в новую структуру, проверяя совпадение размера и доверяя любому содержимому (именование по шаблону).
#define _FillNewStructure(name)			_PPControlSize(name);																	\
										if(!bDoNotStore)																		\
										{																						\
											oParseResult.bStored = true;														\
											aProtocolStorage.Init(sizeof(name));												\
											memcpy(aProtocolStorage.p_Data, (void*)p_ushCurrPos, sizeof(name));					\
										}
#define CaseCommand(typecode)			case typecode: oParseResult.iRes = PROTOPARSER_OK; _PCControlSize; break
#define CasePocket(typecode, name)		case typecode: oParseResult.iRes = PROTOPARSER_OK; _FillNewStructure(name); break

// ========================= КОДЫ ВЗАИМОДЕЙСТВИЯ ==============================
#define PROTO_S_SERVER_NAME         50001
#define PROTO_C_SEND_PASSW			50002
#define PROTO_S_PASSW_OK			50003
#define PROTO_S_PASSW_ERR			50004
#define PROTO_S_BAN					50005
#define PROTO_S_KICK				50006
#define PROTO_S_UNSECURED			50007
#define PROTO_S_UNKNOWN_COMMAND		50008
#define PROTO_C_REQUEST_LEAVING		50009
#define PROTO_S_ACCEPT_LEAVING		50010
#define PROTO_S_SHUTDOWN_INFO		50011
#define PROTO_S_BUFFER_FULL			50012
#define PROTO_C_BUFFER_FULL			50013
#define PROTO_A_BUFFER_READY		50014

//========================== ИСПОЛЬЗУЕМЫЕ СЛУЖЕБНЫЕ СТРУКТУРЫ ===========================
/// Структура сообщения имени сервера.
struct PServerName
{
	char m_chServerName[SERVER_NAME_STR_LEN];
};
/// Структура пароля.
struct PPassword
{
	char m_chPassw[AUTH_PASSWORD_STR_LEN];
};

//========================== ПРИВЯЗКА ТИПОВ КОМАНДНЫХ ПАКЕТОВ =========================
#define CommandPocketTypesHub				\
CaseCommand(PROTO_S_PASSW_OK);				\
CaseCommand(PROTO_S_PASSW_ERR);				\
CaseCommand(PROTO_C_REQUEST_LEAVING);		\
CaseCommand(PROTO_S_ACCEPT_LEAVING);		\
CaseCommand(PROTO_S_SHUTDOWN_INFO);			\
CaseCommand(PROTO_S_BUFFER_FULL);			\
CaseCommand(PROTO_C_BUFFER_FULL);			\
CaseCommand(PROTO_A_BUFFER_READY);			\
CaseCommand(PROTO_S_UNSECURED);				\
CaseCommand(PROTO_S_BAN);					\
CaseCommand(PROTO_S_KICK)
//========================== ПРИВЯЗКА ТИПОВ СЛУЖЕБНЫХ ПАКЕТОВ =========================
#define UtilPocketTypesHub						\
CasePocket(PROTO_S_SERVER_NAME, PServerName);	\
CasePocket(PROTO_C_SEND_PASSW, PPassword);


//== КЛАССЫ.
/// Класс хранилища пакета протокола.
class ProtocolStorage
{
public:
	void* p_Data;								///< Указатель данные.
	unsigned short ushTypeCode;					///< Переменная с кодом пакета.
public:
	/// Конструктор.
	ProtocolStorage();
	/// Инициализация хранилища.
	void Init(int iSize);
												///< \param[in] iSize Размер объекта в байтах.
	/// Освобождение хранилища.
	void Release();
	/// Деструктор.
	~ProtocolStorage();
};

#endif // PROTO_UTIL_H
