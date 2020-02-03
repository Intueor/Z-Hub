#ifndef MAINHUBDEFS_H
#define MAINHUBDEFS_H

//== КОНСТАНТЫ.
// Многократно используемые строки.
const QString cstrStatusReady = "Готов к работе";
const QString cstrStatusShutdown = "Отключение...";
const QString cstrMsgError = "Ошибка";
const QString cstrLogCorruptConf = "Configuration file is corrupt! ";

//== МАКРОСЫ.
// Общее.
#define DEF_CHAR_PTH(def)               &(_chpPH = def)
#define CHAR_PTH                        char _chpPH
// Разное.
#define QRealToDbFrame(qreal, dbframe)                                                              \
	dbframe.dbX = qreal.x();                                                                        \
	dbframe.dbY = qreal.y();                                                                        \
	dbframe.dbH = qreal.height();                                                                   \
	dbframe.dbW = qreal.width();
// Диалоги.
#define DIALOGS_ACCEPT                  0
#define DIALOGS_REJECT                  -1

#endif // MAINHUBDEFS_H
