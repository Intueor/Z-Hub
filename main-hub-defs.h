#ifndef MAINHUBDEFS_H
#define MAINHUBDEFS_H

//== КОНСТАНТЫ.
// Многократно используемые строки.
const char m_chStatusReady[] = "Готов к работе";
const char m_chStatusShutdown[] = "Отключение...";
const char m_chMsgError[] = "Ошибка";
const char m_chMsgWarning[] = "Внимание";
const char m_chMsgWrongIPPort[] = "Неверные данные адрес/порт.";
const char m_chLogStart[] = "START.";
const char m_chLogExit[] = "EXIT.";
const char m_chLogErrorExit[] = "EXIT WITH ERROR: ";
const char m_chLogRestoreUI[] = "Restore UI states.";
const char m_chLogNoGeometryState[] = "Can`t restore Geometry UI state.";
const char m_chLogNoWindowState[] = "Can`t restore WindowState UI state.";
const char m_chLogMainWindowIniAbsent[] = "mainwidow_ui.ini is missing and will be created by default at the exit from program.";
const char m_chLogCorruptConf[] = "Configuration file is corrupt! ";
const char m_chLogCantOpenConfig[] = "Can`t open configuration file: ";
const char m_chLogIsLoaded[] = " is loaded.";
const char m_chLogNodeInList[] = " node in servers list.";
const char m_chLogCantSave[] = "Can`t save ";
const char m_chLogCantStart[] = "Can`t start ";
const char m_chLogCantStop[] = "Can`t stop ";
const char m_chLogCantOpen[] = "Can`t open ";

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
