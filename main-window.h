#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//== ВКЛЮЧЕНИЯ.
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#endif
#include <QMainWindow>
#include <QSettings>
#include <QLabel>
#include <QVector>
#include "logger.h"
#include "parser-ext.h"
#include "main-hub.h"

//== МАКРОСЫ.
#define LOG_DIR_PATH			"../Z-Hub/logs/"

//== ПРОСТРАНСТВА ИМЁН.
namespace Ui {
	class MainWindow;
}

// Для избежания ошибки при доступе из другого потока.
Q_DECLARE_METATYPE(QVector<int>)
//

//== ПРЕД-ДЕКЛАРАЦИИ.

//== КЛАССЫ.
/// Класс главного окна.
class MainWindow : public QMainWindow
{
	Q_OBJECT

private:

public:
	static int iInitRes; ///< Результат инициализации.

public:
	/// Конструктор.
	explicit MainWindow(QWidget* p_parent = nullptr);
							///< \param[in] p_parent Указатель на родительский виджет.
	/// Деструктор.
	~MainWindow();
	/// Процедуры при закрытии окна приложения.
	void closeEvent(QCloseEvent* event);
							///< \param[in] event Указатель на событие.

private:

private slots:

private:
	LOGDECL
	LOGDECL_PTHRD_INCLASS_ADD
	static Ui::MainWindow *p_ui; ///< Указатель на UI.
	static const char* cp_chUISettingsName; ///< Указатель на имя файла с установками UI.
	static QSettings* p_UISettings; ///< Указатель на строку установок UI.
	static QLabel* p_QLabelStatusBarText; ///< Указатель на объект метки статуса.
};

#endif // MAINWINDOW_H

