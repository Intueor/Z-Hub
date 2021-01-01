//== ВКЛЮЧЕНИЯ.
#include "proto-parser.h"
#include "string.h"

//== ФУНКЦИИ КЛАССОВ.
//== Класс парсера протокола.
// Парсинг пакета в соответствующий член хранилища класса парсера.
ProtoParser::ParseResult ProtoParser::ParsePocket(char* p_chData, int iLength,
												  ProtocolStorage& aProtocolStorage, bool bDoNotStore)
{
	unsigned short* p_ushCurrPos;
	unsigned int* p_uiCurrPos;
	ParseResult oParseResult;
	int iSizeOfStructure = 0;
	//
	iSizeOfStructure = iSizeOfStructure; // Заглушка.
	oParseResult.iRes = PROTOPARSER_UNKNOWN_COMMAND;
	oParseResult.bStored = false;
	bDoNotStore = bDoNotStore;
	int iCurrentLength;
	bool bOutOfRange;
	int iSizeOfHeader = sizeof(unsigned int) + sizeof(unsigned short);
	//
	bOutOfRange = false;
	p_uiCurrPos = (unsigned int*)p_chData;
	if(*p_uiCurrPos != (unsigned int)PROTOCOL_CODE)
	{
		oParseResult.iRes = PROTOPARSER_WRONG_FORMAT;
		oParseResult.p_chExtraData = nullptr;
		oParseResult.iExtraDataLength = 0;
		return oParseResult;
	}
	p_uiCurrPos++;
	p_ushCurrPos = (unsigned short*)p_uiCurrPos;
	oParseResult.ushTypeCode = *p_ushCurrPos;
	p_ushCurrPos++;
	iCurrentLength = iLength - iSizeOfHeader; // Длина всего остального, кроме кодов.
	switch(oParseResult.ushTypeCode)
	{
		// Обработка команд.
		CommandPocketTypesHub;
		// Обработка служебных пакетов.
		UtilPocketTypesHub;
		// Обработка пакетов.
		PocketTypesHub;
	}
	if(oParseResult.bStored == true) aProtocolStorage.ushTypeCode = oParseResult.ushTypeCode;
	// Если зашкалило...
	if(bOutOfRange == true)
	{
		p_chData += iCurrentLength;
		p_chData += iSizeOfHeader;
		oParseResult.p_chExtraData = p_chData;
		oParseResult.iExtraDataLength = iLength - iCurrentLength - iSizeOfHeader;
	}
	else
	{
		oParseResult.p_chExtraData = nullptr;
		oParseResult.iExtraDataLength = 0;
	}
	return oParseResult;
}
