#ifndef PROTO_PARSER_H
#define PROTO_PARSER_H

//== ВКЛЮЧЕНИЯ.
#include "protocol.h"

//== МАКРОСЫ.
#define PROTOPARSER_OK					0x00
#define PROTOPARSER_UNKNOWN_COMMAND		_NMG-4 // См. protocol.h для занятия нового свободного номера.
#define PROTOPARSER_WRONG_FORMAT		_NMG-6 // См. protocol.h для занятия нового свободного номера.
// Сообщения.
#define MSG_POCKET_OUT_OF_RANGE			"Pocket is out of range."
#define MSG_UNKNOWN_COMMAND				"Unknown command"
#define MSG_WRONG_FORMAT				" reports wrong format of pocket."

//== КЛАССЫ.
/// Класс парсера протокола.
class ProtoParser
{
public:
	/// Структура возвратных результатов парсинга.
	struct ParseResult
	{
		bool bStored; ///< Признак сохранения в новую структуру.
		int iRes; ///< Результат операции.
		unsigned short ushTypeCode; ///< Код принятого пакета (вне зависимости от статуса сохранения).
		int iExtraDataLength; ///< Длина оставшегося пакета в байтах после возможного 'слипания' пакетов.
		char* p_chExtraData; ///< При 'слипании' пакетов - указатель на дополнительные данные, иначе - 0.
	};
public:
	/// Парсинг пакета в соответствующий член хранилища.
	ParseResult ParsePocket(char* p_chData, int iLength, ProtocolStorage& aProtocolStorage, bool bDoNotStore = false);
											///< \param[in] p_chData Указатель на пакет.
											///< \param[in] iLength Длина пакета в байтах.
											///< \param[out] aProtocolStorage Ссылка на структуру для заполнения.
											///< \param[in] bDoNotStore Флаг необходимости игнорировать заполнение хранилища.
											///< \return Результат парсинга в соотв. структуре.
};

#endif // PROTO_PARSER_H
