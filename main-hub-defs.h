#ifndef MAINHUBDEFS_H
#define MAINHUBDEFS_H

//== КОНСТАНТЫ.
const QString cstrStatusReady = "Ready";
const QString cstrStatusShutdown = "Shutdown...";

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
#define ThrUiAccessV(element,func,type,value)   QMetaObject::invokeMethod(p_ui->element, #func, Qt::QueuedConnection, Q_ARG(type, value))
#define ThrUiAccessE(element,func)              QMetaObject::invokeMethod(p_ui->element, #func, Qt::QueuedConnection)
#define ThrUiAccessVT(element,func,type,value)  QMetaObject::invokeMethod(element, #func, Qt::QueuedConnection, Q_ARG(type, value))
#define ThrUiAccessET(element,func)             QMetaObject::invokeMethod(element, #func, Qt::QueuedConnection)

#endif // MAINHUBDEFS_H
