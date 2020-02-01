#ifndef MAINHUBDEFS_H
#define MAINHUBDEFS_H

//== КОНСТАНТЫ.
const QString cstrStatusReady = "Готов к работе";
const QString cstrStatusShutdown = "Отключение...";

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
#define SetStatusBarText(qstring)				p_QLabelStatusBarText->setText(qstring);			\
												p_ui->statusBar->repaint()

#endif // MAINHUBDEFS_H
