//== ВКЛЮЧЕНИЯ.
#include <QApplication>
#include "main-window.h"

//== ФУНКЦИИ.
// Точка входа в приложение.
int main(int argc, char *argv[])
							///< \param[in] argc Заглушка.
							///< \param[in] argv Заглушка.
							///< \return Общий результат работы.
{
	int iExecResult;
	QApplication oApplication(argc, argv);
	oApplication.setAttribute(Qt::AA_DisableWindowContextHelpButton);
	MainWindow wMainWindow;
	//
	if(wMainWindow.iInitRes == RETVAL_OK)
	{
		setlocale(LC_NUMERIC, "en_US.UTF-8");
		wMainWindow.show();
		iExecResult = oApplication.exec();
	}
	else iExecResult = RETVAL_ERR;
	return iExecResult;
}
