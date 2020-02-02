//== ВКЛЮЧЕНИЯ.
#include <QScrollBar>
#include "main-window.h"
#include "ui_main-window.h"
#include "Dialogs/message-dialog.h"

//== МАКРОСЫ.
#define LOG_NAME				"main-window"
#define LOG_DIR_PATH			"../Z-Hub/logs/"

//== ДЕКЛАРАЦИИ СТАТИЧЕСКИХ ПЕРЕМЕННЫХ.
// Основное.
LOGDECL_INIT_INCLASS(MainWindow)
LOGDECL_INIT_PTHRD_INCLASS_OWN_ADD(MainWindow)
int MainWindow::iInitRes;
const char* MainWindow::cp_chUISettingsName = H_MAINWINDOW_UI_CONF_PATH;
Ui::MainWindow* MainWindow::p_ui = new Ui::MainWindow;
QSettings* MainWindow::p_UISettings = nullptr;
QLabel* MainWindow::p_QLabelStatusBarText;
Server* MainWindow::p_Server = nullptr;
vector<Server::IPBanUnit> MainWindow::vec_IPBanUnits;
NetHub::IPPortPassword MainWindow::oIPPortPassword;
char MainWindow::m_chServerName[SERVER_NAME_STR_LEN];
char MainWindow::m_chIP[IP_STR_LEN];
char MainWindow::m_chPort[PORT_STR_LEN];
char MainWindow::m_chPassword[AUTH_PASSWORD_STR_LEN];

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
	p_QLabelStatusBarText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	p_ui->statusBar->addWidget(p_QLabelStatusBarText);
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
	if(!LoadBansCatalogue(vec_IPBanUnits)) goto gEI;
	if(!LoadServerConfig(oIPPortPassword, m_chServerName)) goto gEI;
	p_Server = new Server(LOG_MUTEX, &vec_IPBanUnits);
	p_Server->SetClientStatusChangedCB(ClientStatusChangedCallback);
	p_Server->SetClientDataArrivedCB(ClientDataArrivedCallback);
	p_Server->SetClientRequestArrivedCB(ClientRequestArrivedCallback);
	if(!ServerStartProcedures(oIPPortPassword, m_chServerName))
	{
gEI:	iInitRes = RETVAL_ERR;
		RETVAL_SET(RETVAL_ERR);
		return;
	}
}

// Деструктор.
MainWindow::~MainWindow()
{
	if(RETVAL == RETVAL_OK)
	{
		LOG_P_0(LOG_CAT_I, "EXIT.")
	}
	else
	{
		LOG_P_0(LOG_CAT_E, "EXIT WITH ERROR: " << RETVAL);
	}
	LOG_CLOSE;
	delete p_ui;
}

// Процедуры при закрытии окна приложения.
void MainWindow::closeEvent(QCloseEvent *event)
{
	SetStatusBarText(cstrStatusShutdown);
	if(p_Server)
	{
		LCHECK_BOOL(ServerStopProcedures());
		delete p_Server;
	}
	// Main.
	p_UISettings->setValue("Geometry", saveGeometry());
	p_UISettings->setValue("WindowState", saveState());
	// Splitters.
	QMainWindow::closeEvent(event);
}

// Загрузка каталога банов.
bool MainWindow::LoadBansCatalogue(vector<Server::IPBanUnit>& o_vec_IPBanUnits)
{
	XMLError eResult;
	tinyxml2::XMLDocument xmlDocBans;
	Server::IPBanUnit oIPBanUnit;
	QString strHelper;
	list<XMLNode*> l_pIPBans;
	//
	eResult = xmlDocBans.LoadFile(S_BANS_CAT_PATH);
	if (eResult != XML_SUCCESS)
	{
		LOG_P_0(LOG_CAT_E, "Can`t open bans catalogue file: " << S_BANS_CAT_PATH);
		return false;
	}
	LOG_P_1(LOG_CAT_I, "Bans catalogue is loaded.");
	if(!FindChildNodes(xmlDocBans.LastChild(), l_pIPBans,
					   "IPBans", FCN_ONE_LEVEL, FCN_FIRST_ONLY))
	{
		LOG_P_0(LOG_CAT_E, "Bans catalogue file is corrupt. 'IPBans' node is absend.");
		return false;
	}
	PARSE_CHILDLIST(l_pIPBans.front(), p_ListIPs, "IP",
					FCN_ONE_LEVEL, p_NodeIP)
	{
		strHelper = QString(p_NodeIP->FirstChild()->Value());
		if(strHelper.isEmpty())
		{
			LOG_P_2(LOG_CAT_I, "Bans catalogue file is corrupt. 'IP' node is empty.");
			return false;
		}
		memcpy(oIPBanUnit.m_chIP,
			   strHelper.toStdString().c_str(), SizeOfChars(strHelper.toStdString().length() + 1));
		o_vec_IPBanUnits.push_back(oIPBanUnit);
	} PARSE_CHILDLIST_END(p_ListIPs);
	return true;
}

// Загрузка конфигурации сервера.
bool MainWindow::LoadServerConfig(NetHub::IPPortPassword& o_IPPortPassword, char* p_chServerName)
{
	tinyxml2::XMLDocument xmlDocSConf;
	list <XMLNode*> l_pNet;
	list <XMLNode*> l_pName;
	XMLError eResult;
	//
	o_IPPortPassword.p_chIPNameBuffer = nullptr;
	o_IPPortPassword.p_chPortNameBuffer = nullptr;
	o_IPPortPassword.p_chPasswordNameBuffer = nullptr;
	eResult = xmlDocSConf.LoadFile(S_CONF_PATH);
	if (eResult != XML_SUCCESS)
	{
		LOG_P_0(LOG_CAT_E, "Can`t open configuration file: " << S_CONF_PATH);
		return false;
	}
	LOG_P_1(LOG_CAT_I, "Configuration is loaded.");
	if(!FindChildNodes(xmlDocSConf.LastChild(), l_pName,
					   "Name", FCN_ONE_LEVEL, FCN_FIRST_ONLY))
	{
		LOG_P_0(LOG_CAT_E, "Configuration file is corrupt! No 'Name' node.");
		return false;
	}
	CopyStrArray((char*)l_pName.front()->FirstChild()->Value(), p_chServerName, SERVER_NAME_STR_LEN);
	LOG_P_1(LOG_CAT_I, "Server name: " << m_chServerName);
	if(!FindChildNodes(xmlDocSConf.LastChild(), l_pNet,
					   "Net", FCN_ONE_LEVEL, FCN_FIRST_ONLY))
	{
		LOG_P_0(LOG_CAT_E, "Configuration file is corrupt! No 'Net' node.");
		return false;
	}
	FIND_IN_CHILDLIST(l_pNet.front(), p_ListServerIP, "IP",
					  FCN_ONE_LEVEL, p_NodeServerIP)
	{
		o_IPPortPassword.p_chIPNameBuffer = (char*)p_NodeServerIP->FirstChild()->Value();
	} FIND_IN_CHILDLIST_END(p_ListServerIP);
	if(o_IPPortPassword.p_chIPNameBuffer != nullptr)
	{
		CopyStrArray(oIPPortPassword.p_chIPNameBuffer, m_chIP, IP_STR_LEN);
		oIPPortPassword.p_chIPNameBuffer = m_chIP;
		if(NetHub::CheckIPv4(o_IPPortPassword.p_chIPNameBuffer))
		{
			LOG_P_1(LOG_CAT_I, "Server IP: " << o_IPPortPassword.p_chIPNameBuffer);
		}
		else
		{
			LOG_P_1(LOG_CAT_I, "Server IP: [" << o_IPPortPassword.p_chIPNameBuffer << "]");
		}
	}
	else
	{
		LOG_P_0(LOG_CAT_E, "Configuration file is corrupt! No '(Net)IP' node.");
		return false;
	}
	FIND_IN_CHILDLIST(l_pNet.front(), p_ListPort, "Port",
					  FCN_ONE_LEVEL, p_NodePort)
	{
		o_IPPortPassword.p_chPortNameBuffer = (char*)p_NodePort->FirstChild()->Value();
	} FIND_IN_CHILDLIST_END(p_ListPort);
	if(o_IPPortPassword.p_chPortNameBuffer != nullptr)
	{
		CopyStrArray(oIPPortPassword.p_chPortNameBuffer, m_chPort, PORT_STR_LEN);
		oIPPortPassword.p_chPortNameBuffer = m_chPort;
		LOG_P_1(LOG_CAT_I, "Server port: " << o_IPPortPassword.p_chPortNameBuffer)
	}
	else
	{
		LOG_P_0(LOG_CAT_E, "Configuration file is corrupt! No '(Net)Port' node.");
		return false;
	}
	FIND_IN_CHILDLIST(l_pNet.front(), p_ListPassword, "Password",
					  FCN_ONE_LEVEL, p_NodePassword)
	{
		o_IPPortPassword.p_chPasswordNameBuffer = (char*)p_NodePassword->FirstChild()->Value();
	} FIND_IN_CHILDLIST_END(p_ListPassword);
	if(o_IPPortPassword.p_chPasswordNameBuffer == nullptr)
	{
		LOG_P_0(LOG_CAT_E, "Configuration file is corrupt! No 'Password' node.");
		return false;
	}
	else
	{
		CopyStrArray(oIPPortPassword.p_chPasswordNameBuffer, m_chPassword, AUTH_PASSWORD_STR_LEN);
		oIPPortPassword.p_chPasswordNameBuffer = m_chPassword;
	}
	return true;
}


// Кэлбэк обработки отслеживания статута клиентов.
void MainWindow::ClientStatusChangedCallback(int iConnection, bool bConnected)
{
	char m_chIPNameBuffer[IP_STR_LEN];
	char m_chPortNameBuffer[PORT_STR_LEN];
	NetHub::ConnectionData oConnectionDataInt;
	//
	LOG_P_0(LOG_CAT_I, "ID: " << iConnection << " have status: " << bConnected);
	oConnectionDataInt = p_Server->GetConnectionData(iConnection, false);
	if(oConnectionDataInt.iStatus != NO_CONNECTION)
	{
		p_Server->FillIPAndPortNames(oConnectionDataInt, m_chIPNameBuffer, m_chPortNameBuffer, false);
		if(NetHub::CheckIPv4(m_chIPNameBuffer))
		{
			LOG_P_0(LOG_CAT_I, "IP: " << m_chIPNameBuffer << " Port: " << m_chPortNameBuffer);
		}
		else
		{
			LOG_P_0(LOG_CAT_I, "IP: [" << m_chIPNameBuffer << "] Port: " << m_chPortNameBuffer);
		}
	}
	else
	{
		LOG_P_0(LOG_CAT_E, "Can`t get connection data from server storage.");
		RETVAL_SET(RETVAL_ERR);
	}
}

// Кэлбэк обработки приходящих пакетов данных.
void MainWindow::ClientDataArrivedCallback(int iConnection, unsigned short ushType, void* p_ReceivedData, int iPocket)
{
	//
//	switch(ushType)
//	{
//		//======== Раздел PROTO_O_SCH_ELEMENT_BASE. ========
//		case PROTO_O_SCH_ELEMENT_BASE:
//		{
//			Element* p_Element;
//			//
//			if(!CheckAuthority(iConnection))
//			{
//				goto gLEx;
//			}
//			p_PSchElementBase = ((PSchElementBase*)p_ReceivedData);
//			LOG_P_2(LOG_CAT_I, "{In} Element [" << QString(p_PSchElementBase->m_chName).toStdString()
//					<< "] base from ID: " << QString::number(iConnection).toStdString());
//			AppendToPBExternal(Element, p_Element = new Element(*p_PSchElementBase), S_World);
//			p_Element->m_bNew[iConnection] = false;
//			for(int iF = 0; iF < MAX_CONN; iF++)
//			{
//				p_Element->m_chTouchedBits[iF] = 0;
//				if(iF != iConnection)
//				{
//					//p_Element->m_chTouchedBits[iF] = TOUCHED_GEOMETRY;
//					if(p_PSchElementBase->oPSchElementVars.ullIDGroup != 0)
//					{
//						p_Element->m_chTouchedBits[iF] |= TOUCHED_GROUP;
//					}
//				}
//			}
//		}
//			//======== Следующий раздел... ========
//	}
}

// Кэлбэк обработки приходящих запросов.
void MainWindow::ClientRequestArrivedCallback(int iConnection, unsigned short ushRequest)
{
	//
	LOG_P_2(LOG_CAT_I, "Client: " << iConnection << " request: " << ushRequest);
}

// Процедуры запуска сервера.
bool MainWindow::ServerStartProcedures(NetHub::IPPortPassword& o_IPPortPassword, char* p_chServerName)
{
	Message_Dialog* p_Message_Dialog;
	//
	SetStatusBarText("Запуск сервера...");
	if(!p_Server->Start(&o_IPPortPassword, p_chServerName))
	{
		goto gSS;
	}
	for(unsigned char uchAtt = 0; uchAtt != SERVER_WAITING_ATTEMPTS; uchAtt++)
	{
		if(p_Server->CheckServerAlive())
		{
			SetStatusBarText(cstrStatusWorking);
			return true;
		}
		MSleep(USER_RESPONSE_MS);
	}
gSS:LOG_P_0(LOG_CAT_E, "Can`t start server.");
	p_Message_Dialog = new Message_Dialog(QString("Error").toStdString().c_str(), QString("Failed to start server").toStdString().c_str());
	p_Message_Dialog->exec();
	p_Message_Dialog->deleteLater();
	return false;
}

// Процедуры остановки сервера.
bool MainWindow::ServerStopProcedures()
{
	Message_Dialog* p_Message_Dialog;
	//
	if(p_Server->CheckServerAlive())
	{
		SetStatusBarText("Остановка сервера...");
		if(!p_Server->Stop())
		{
			goto gSB;
		}
		for(unsigned char uchAtt = 0; uchAtt != SERVER_WAITING_ATTEMPTS; uchAtt++)
		{
			if(!p_Server->CheckServerAlive())
			{
				SetStatusBarText(cstrStatusReady);
				return true;
			}
			MSleep(USER_RESPONSE_MS);
		}
	gSB:LOG_P_0(LOG_CAT_E, "Can`t stop server.");
		p_Message_Dialog = new Message_Dialog(QString("Error").toStdString().c_str(), QString("Failed to stop server").toStdString().c_str());
		p_Message_Dialog->exec();
		p_Message_Dialog->deleteLater();
		SetStatusBarText(cstrStatusWorking);
		return false;
	}
	else return true;
}

// Установка текста строки статуса.
void MainWindow::SetStatusBarText(QString strMsg)
{
	p_QLabelStatusBarText->setText(strMsg);
	p_ui->statusBar->repaint();
}
