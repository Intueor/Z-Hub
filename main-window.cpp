//== ВКЛЮЧЕНИЯ.
#include <QScrollBar>
#include "main-window.h"
#include "ui_main-window.h"

//== МАКРОСЫ.
#define LOG_NAME				"main-window"

//== ДЕКЛАРАЦИИ СТАТИЧЕСКИХ ПЕРЕМЕННЫХ.
LOGDECL_INIT_INCLASS(MainWindow)
LOGDECL_INIT_PTHRD_INCLASS_OWN_ADD(MainWindow)
int MainWindow::iInitRes;
const char* MainWindow::cp_chUISettingsName = H_MAINWINDOW_UI_CONF_PATH;
Ui::MainWindow* MainWindow::p_ui = new Ui::MainWindow;
QSettings* MainWindow::p_UISettings = nullptr;
QLabel* MainWindow::p_QLabelStatusBarText;

//== ФУНКЦИИ КЛАССОВ.
//== Класс главного окна.
// Конструктор.
MainWindow::MainWindow(QWidget* p_parent) :
	QMainWindow(p_parent)
{
	// Для избежания ошибки при доступе из другого потока.
	qRegisterMetaType<QVector<int>>("QVector<int>");
	//
	LOG_CTRL_INIT;
	LOG_P_0(LOG_CAT_I, "START.");
	iInitRes = RETVAL_OK;
	p_UISettings = new QSettings(cp_chUISettingsName, QSettings::IniFormat);
	p_ui->setupUi(this);
	p_QLabelStatusBarText = new QLabel(this);
	p_ui->statusBar->addWidget(p_QLabelStatusBarText);
	p_QLabelStatusBarText->setText(tr("Инициализация."));
	if(IsFileExists((char*)cp_chUISettingsName))
	{
		LOG_P_2(LOG_CAT_I, "Restore UI states.");
		// Splitters.

		// MainWidow.
		if(!restoreGeometry(p_UISettings->value("Geometry").toByteArray()))
		{
			LOG_P_1(LOG_CAT_E, "Can`t restore Geometry UI state.");
			RETVAL_SET(RETVAL_ERR);
		}
		if(!restoreState(p_UISettings->value("WindowState").toByteArray()))
		{
			LOG_P_1(LOG_CAT_E, "Can`t restore WindowState UI state.");
			RETVAL_SET(RETVAL_ERR);
		}
		// Other.
	}
	else
	{
		LOG_P_0(LOG_CAT_W, "mainwidow_ui.ini is missing and will be created by default at the exit from program.");
	}
	p_QLabelStatusBarText->setText(tr("Готов к работе."));
}

// Деструктор.
MainWindow::~MainWindow()
{
	LOG_CLOSE;
	delete p_ui;
}

// Процедуры при закрытии окна приложения.
void MainWindow::closeEvent(QCloseEvent *event)
{
	p_QLabelStatusBarText->setText(tr("Выключение."));
	p_ui->statusBar->repaint();
	// Main.
	p_UISettings->setValue("Geometry", saveGeometry());
	p_UISettings->setValue("WindowState", saveState());
	// Splitters.
	QMainWindow::closeEvent(event);
}
