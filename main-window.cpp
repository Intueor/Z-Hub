//== ВКЛЮЧЕНИЯ.
#include <QScrollBar>
#include "main-window.h"
#include "ui_main-window.h"
#include "Dialogs/message-dialog.h"
#include "Dialogs/set_proposed_string_dialog.h"
#include "Dialogs/set-server-dialog.h"
#include "Dialogs/set_proposed_bool_dialog.h"

//== МАКРОСЫ.
#define LOG_NAME				"main-window"
#define LOG_DIR_PATH			"../Z-Hub/logs/"

//== ДЕКЛАРАЦИИ СТАТИЧЕСКИХ ПЕРЕМЕННЫХ.
// Основное.
LOGDECL_INIT_INCLASS(MainWindow)
LOGDECL_INIT_PTHRD_INCLASS_OWN_ADD(MainWindow)
unsigned char MainWindow::uchInitRes = RETVAL_OK;
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
char MainWindow::m_chEnvName[ENV_NAME_LEN];
Environment* MainWindow::p_Environment = nullptr;

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
	LOG_P_0(LOG_CAT_I, m_chLogStart);
	p_UISettings = new QSettings(cp_chUISettingsName, QSettings::IniFormat);
	p_ui->setupUi(this);
	p_QLabelStatusBarText = new QLabel(this);
	p_QLabelStatusBarText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	p_ui->statusBar->addWidget(p_QLabelStatusBarText);
	if(IsFileExists((char*)cp_chUISettingsName))
	{
		LOG_P_2(LOG_CAT_I, m_chLogRestoreUI);
		// Splitters.

		// MainWidow.
		if(!restoreGeometry(p_UISettings->value("Geometry").toByteArray()))
		{
			LOG_P_1(LOG_CAT_E, m_chLogNoGeometryState);
			RETVAL_SET(RETVAL_ERR);
		}
		if(!restoreState(p_UISettings->value("WindowState").toByteArray()))
		{
			LOG_P_1(LOG_CAT_E, m_chLogNoWindowState);
			RETVAL_SET(RETVAL_ERR);
		}
	}
	else
	{
		LOG_P_0(LOG_CAT_W, m_chLogMainWindowIniAbsent);
	}
	if(!LoadBansCatalogue(vec_IPBanUnits)) goto gEI;
	if(!LoadServerConfig(oIPPortPassword, m_chServerName))
	{
gEI:	uchInitRes = RETVAL_ERR;
		RETVAL_SET(RETVAL_ERR);
		return;
	}
	if(!LoadEnvConfig(m_chEnvName)) goto gEI;
	p_Server = new Server(LOG_MUTEX, &vec_IPBanUnits);
	p_Server->SetClientStatusChangedCB(ClientStatusChangedCallback);
	p_Server->SetClientDataArrivedCB(ClientDataArrivedCallback);
	p_Server->SetClientRequestArrivedCB(ClientRequestArrivedCallback);
	p_Environment = new Environment(LOG_MUTEX, m_chEnvName);
	if(p_UISettings->value("AutostartServer").toBool())
	{
		p_ui->action_StartOnLaunchApp->setChecked(true);
		p_ui->action_StartStopServer->setChecked(true);
		LCHECK_BOOL(ServerStartProcedures(oIPPortPassword));
	}
	else
	{
		SetStatusBarText(m_chStatusReady);
	}
	if(p_UISettings->value("AutostartEnv").toBool())
	{
		p_ui->action_StartOnLaunchServer->setChecked(true);
		p_ui->action_StartStopEnv->setChecked(true);
		LCHECK_BOOL(EnvStartProcedures());
	}
	p_ui->action_Autosave->setChecked(p_UISettings->value("Autosave").toBool());
}

// Деструктор.
MainWindow::~MainWindow()
{
	if(RETVAL == RETVAL_OK)
	{
		LOG_P_0(LOG_CAT_I, m_chLogExit)
	}
	else
	{
		LOG_P_0(LOG_CAT_E, m_chLogErrorExit << RETVAL);
	}
	LOG_CLOSE;
	delete p_ui;
}

// Процедуры при закрытии окна приложения.
void MainWindow::closeEvent(QCloseEvent *event)
{
	Set_Proposed_Bool_Dialog* p_Set_Proposed_Bool_Dialog;
	//
	SetStatusBarText(m_chStatusShutdown);
	if(p_Environment)
	{
		if(p_ui->action_StartStopEnv->isChecked())
		{
			LCHECK_BOOL(EnvStopProcedures());
		}
		if(!p_ui->action_Autosave->isChecked())
		{
			if(!p_Environment->CheckInitialized()) goto gNI;
			p_Set_Proposed_Bool_Dialog = new Set_Proposed_Bool_Dialog((char*)m_chMsgWarning,
																	  (char*)"Все несохранённые данные среды будут утеряны. Сохранить?");
			if(p_Set_Proposed_Bool_Dialog->exec() == DIALOGS_ACCEPT)
			{
				goto gS;
			}
		}
		else
		{
gS:			LCHECK_BOOL(p_Environment->SaveEnv());
		}
gNI:	delete p_Environment;
	}
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
		LOG_P_0(LOG_CAT_E, m_chLogCantOpen << "bans catalogue file: " << S_BANS_CAT_PATH);
		return false;
	}
	LOG_P_1(LOG_CAT_I, "Bans catalogue" << m_chLogIsLoaded);
	if(!FindChildNodes(xmlDocBans.LastChild(), l_pIPBans,
					   "IPBans", FCN_ONE_LEVEL, FCN_FIRST_ONLY))
	{
		LOG_P_0(LOG_CAT_E, m_chLogBansCorrupt << "'IPBans' node is absend.");
		return false;
	}
	PARSE_CHILDLIST(l_pIPBans.front(), p_ListIPs, "IP",
					FCN_ONE_LEVEL, p_NodeIP)
	{
		strHelper = QString(p_NodeIP->FirstChild()->Value());
		if(strHelper.isEmpty())
		{
			LOG_P_2(LOG_CAT_I, m_chLogBansCorrupt << "'IP' node is empty.");
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
		LOG_P_0(LOG_CAT_E, m_chLogCantOpenConfig << S_CONF_PATH);
		return false;
	}
	LOG_P_1(LOG_CAT_I, "Server configuration" << m_chLogIsLoaded);
	if(!FindChildNodes(xmlDocSConf.LastChild(), l_pName,
					   "Name", FCN_ONE_LEVEL, FCN_FIRST_ONLY))
	{
		LOG_P_0(LOG_CAT_E, m_chLogCorruptConf << "No 'Name' node.");
		return false;
	}
	CopyStrArray((char*)l_pName.front()->FirstChild()->Value(), p_chServerName, SERVER_NAME_STR_LEN);
	LOG_P_1(LOG_CAT_I, "Server name: " << m_chServerName);
	if(!FindChildNodes(xmlDocSConf.LastChild(), l_pNet,
					   "Net", FCN_ONE_LEVEL, FCN_FIRST_ONLY))
	{
		LOG_P_0(LOG_CAT_E, m_chLogCorruptConf << "No 'Net' node.");
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
		LOG_P_0(LOG_CAT_E, m_chLogCorruptConf << "No '(Net)IP' node.");
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
		LOG_P_0(LOG_CAT_E, m_chLogCorruptConf << "No '(Net)Port' node.");
		return false;
	}
	FIND_IN_CHILDLIST(l_pNet.front(), p_ListPassword, "Password",
					  FCN_ONE_LEVEL, p_NodePassword)
	{
		o_IPPortPassword.p_chPasswordNameBuffer = (char*)p_NodePassword->FirstChild()->Value();
	} FIND_IN_CHILDLIST_END(p_ListPassword);
	if(o_IPPortPassword.p_chPasswordNameBuffer == nullptr)
	{
		LOG_P_0(LOG_CAT_E, m_chLogCorruptConf << "No 'Password' node.");
		return false;
	}
	else
	{
		CopyStrArray(oIPPortPassword.p_chPasswordNameBuffer, m_chPassword, AUTH_PASSWORD_STR_LEN);
		oIPPortPassword.p_chPasswordNameBuffer = m_chPassword;
	}
	return true;
}

// Загрузка конфигурации среды.
bool MainWindow::LoadEnvConfig(char* p_chName)
{
	tinyxml2::XMLDocument xmlDocEConf;
	list <XMLNode*> l_pName;
	XMLError eResult;
	//
	eResult = xmlDocEConf.LoadFile(E_CONF_PATH);
	if (eResult != XML_SUCCESS)
	{
		LOG_P_0(LOG_CAT_E, m_chLogCantOpenConfig << E_CONF_PATH);
		return false;
	}
	LOG_P_1(LOG_CAT_I, "Environment configuration" << m_chLogIsLoaded);
	if(!FindChildNodes(xmlDocEConf.LastChild(), l_pName,
					   "Name", FCN_ONE_LEVEL, FCN_FIRST_ONLY))
	{
		LOG_P_0(LOG_CAT_E, m_chLogCorruptConf << "No 'Name' node.");
		return false;
	}
	CopyStrArray((char*)l_pName.front()->FirstChild()->Value(), p_chName, ENV_NAME_LEN);
	LOG_P_1(LOG_CAT_I, (char*)m_chLogCurrentEnv << p_chName);
	return true;
}

// Сохранение конфигурации сервера.
bool MainWindow::SaveServerConfig()
{
	XMLError eResult;
	tinyxml2::XMLDocument xmlServerConf;
	XMLNode* p_NodeRoot;
	XMLNode* p_NodeName;
	XMLNode* p_NodeNet;
	XMLNode* p_NodeIP;
	XMLNode* p_NodePort;
	XMLNode* p_NodePassword;
	//
	xmlServerConf.InsertEndChild(xmlServerConf.NewDeclaration());
	p_NodeRoot = xmlServerConf.InsertEndChild(xmlServerConf.NewElement("Root"));
	p_NodeName = p_NodeRoot->InsertEndChild(xmlServerConf.NewElement("Name"));
	p_NodeName->ToElement()->SetText(m_chServerName);
	p_NodeNet = p_NodeRoot->InsertEndChild(xmlServerConf.NewElement("Net"));
	p_NodeIP = p_NodeNet->InsertEndChild(xmlServerConf.NewElement("IP"));
	p_NodeIP->ToElement()->SetText(oIPPortPassword.p_chIPNameBuffer);
	p_NodePort = p_NodeNet->InsertEndChild(xmlServerConf.NewElement("Port"));
	p_NodePort->ToElement()->SetText(oIPPortPassword.p_chPortNameBuffer);
	p_NodePassword = p_NodeNet->InsertEndChild(xmlServerConf.NewElement("Password"));
	p_NodePassword->ToElement()->SetText(oIPPortPassword.p_chPasswordNameBuffer);
	eResult = xmlServerConf.SaveFile(S_CONF_PATH);
	if (eResult != XML_SUCCESS)
	{
		LOG_P_0(LOG_CAT_E, m_chLogCantSave << "server configuration.");
		return false;
	}
	return true;
}

// Сохранение конфигурации сервера.
bool MainWindow::SaveEnvConfig()
{
	XMLError eResult;
	tinyxml2::XMLDocument xmlEnvConf;
	XMLNode* p_NodeRoot;
	XMLNode* p_NodeName;
	//
	xmlEnvConf.InsertEndChild(xmlEnvConf.NewDeclaration());
	p_NodeRoot = xmlEnvConf.InsertEndChild(xmlEnvConf.NewElement("Root"));
	p_NodeName = p_NodeRoot->InsertEndChild(xmlEnvConf.NewElement("Name"));
	p_NodeName->ToElement()->SetText(m_chEnvName);
	eResult = xmlEnvConf.SaveFile(E_CONF_PATH);
	if (eResult != XML_SUCCESS)
	{
		LOG_P_1(LOG_CAT_E, m_chLogCantSave << "environment configuration.");
		return false;
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
	LOG_P_1(LOG_CAT_I, "ID: " << iConnection << " have status: " << bConnected);
	oConnectionDataInt = p_Server->GetConnectionData(iConnection, false);
	if(oConnectionDataInt.iStatus != NO_CONNECTION)
	{
		p_Server->FillIPAndPortNames(oConnectionDataInt, m_chIPNameBuffer, m_chPortNameBuffer, false);
		if(NetHub::CheckIPv4(m_chIPNameBuffer))
		{
			LOG_P_1(LOG_CAT_I, "IP: " << m_chIPNameBuffer << " Port: " << m_chPortNameBuffer);
		}
		else
		{
			LOG_P_1(LOG_CAT_I, "IP: [" << m_chIPNameBuffer << "] Port: " << m_chPortNameBuffer);
		}
		if(bConnected)
		{
			p_ui->label_ConnectedClient->setText(QString(m_chIPNameBuffer) + ":" + QString(m_chPortNameBuffer));
		}
		else
		{
			p_ui->label_ConnectedClient->setText(m_chClientLabelWaiting);
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
	PSchElementEraser* p_PSchElementEraser;
	PSchGroupEraser* p_PSchGroupEraser;
	PSchElementName* p_PSchElementName;
	PSchElementBase* p_PSchElementBase;
	PSchLinkVars* p_PSchLinkVars;
	PSchGroupName* p_PSchGroupName;
	PSchElementVars* p_PSchElementVars;
	PSchGroupVars* p_PSchGroupVars;
	//
	switch(ushType)
	{
		//======== Раздел PROTO_C_SCH_READY. ========
		case PROTO_C_SCH_READY:
		{
			Environment::bRequested = true;
			Environment::oPSchReadyFrame = *((PSchReadyFrame*)p_ReceivedData);
gLEx:		if(p_Server->ReleaseDataInPosition(iConnection, (uint)iPocket, false) != RETVAL_OK)
			{
				RETVAL_SET(RETVAL_ERR);
			}
			break;
		}
		//======== Раздел PROTO_O_SCH_GROUP_ERASE. ========
		case PROTO_O_SCH_GROUP_ERASE:
		{
			int iGC;
			//
			LOG_P_2(LOG_CAT_I, "{In} Group for erase from client.");
			iGC = PBCountExternal(Group, Environment);
			p_PSchGroupEraser = ((PSchGroupEraser*)p_ReceivedData);
			for(int iG = 0; iG < iGC; iG++) // По всем группам...
			{
				Group* p_Group;
				//
				p_Group = PBAccessExternal(Group, iG, Environment);
				if(p_Group->oPSchGroupBase.oPSchGroupVars.ullIDInt == p_PSchGroupEraser->ullIDInt) // При совп. с запрошенным...
				{
					//Environment::EraseGroupAt(iG);
					LOG_P_2(LOG_CAT_I, "Group [" << QString(p_Group->oPSchGroupBase.m_chName).toStdString() << "] erased.");
					goto gLEx;
				}
			}
			LOG_P_0(LOG_CAT_W, "Wrong group number for erase from client.");
			goto gLEx;
		}
		//======== Раздел PROTO_O_SCH_ELEMENT_ERASE. ========
		case PROTO_O_SCH_ELEMENT_ERASE:
		{
			int iEC;
			//
			LOG_P_2(LOG_CAT_I, "{In} Element for erase from client.");
			iEC = PBCountExternal(Element, Environment);
			p_PSchElementEraser = ((PSchElementEraser*)p_ReceivedData);
			for(int iE = 0; iE < iEC; iE++) // По всем элементам...
			{
				Element* p_Element;
				//
				p_Element = PBAccessExternal(Element, iE, Environment);
				if(p_Element->oPSchElementBase.oPSchElementVars.ullIDInt == p_PSchElementEraser->ullIDInt) // При совп. с запрошенным...
				{
					//Environment::EraseElementAt(iE);
					LOG_P_2(LOG_CAT_I, "Element [" << QString(p_Element->oPSchElementBase.m_chName).toStdString() << "] erased.");
					goto gLEx;
				}
			}
			LOG_P_0(LOG_CAT_W, "Wrong element number for erase from client.");
			goto gLEx;
		}
		//======== Раздел PROTO_O_SCH_ELEMENT_VARS. ========
		case PROTO_O_SCH_ELEMENT_VARS:
		{
			int iEC;
			//
			LOG_P_2(LOG_CAT_I, "{In} Element vars from client.");
			iEC = PBCountExternal(Element, Environment);
			p_PSchElementVars = ((PSchElementVars*)p_ReceivedData);
			for(int iE = 0; iE < iEC; iE++) // По всем элементам...
			{
				Element* p_Element;
				//
				p_Element = PBAccessExternal(Element, iE, Environment);
				if(p_Element->oPSchElementBase.oPSchElementVars.ullIDInt == p_PSchElementVars->ullIDInt) // При совп. с запрошенным...
				{
					if(p_PSchElementVars->oSchElementGraph.uchChangesBits & SCH_ELEMENT_BIT_ZPOS)
					{
						p_Element->oPSchElementBase.oPSchElementVars.oSchElementGraph.dbObjectZPos = p_PSchElementVars->oSchElementGraph.dbObjectZPos;
						LOG_P_2(LOG_CAT_I, "Element [" << QString(p_Element->oPSchElementBase.m_chName).toStdString()
								<< "] z-pos is: " << QString::number((int)p_PSchElementVars->oSchElementGraph.dbObjectZPos).toStdString());
					}
					if(p_PSchElementVars->oSchElementGraph.uchChangesBits & SCH_ELEMENT_BIT_BUSY)
					{
						p_Element->oPSchElementBase.oPSchElementVars.oSchElementGraph.bBusy =
								p_PSchElementVars->oSchElementGraph.bBusy;
						if(p_PSchElementVars->oSchElementGraph.bBusy)
						{
							LOG_P_2(LOG_CAT_I, "Element [" << QString(p_Element->oPSchElementBase.m_chName).toStdString()
									<< "] is busy by client.");
						}
						else
						{
							LOG_P_2(LOG_CAT_I, "Element [" << QString(p_Element->oPSchElementBase.m_chName).toStdString()
									<< "] is free.");
						}
					}
					if(p_PSchElementVars->oSchElementGraph.uchChangesBits & SCH_ELEMENT_BIT_POS)
					{
						p_Element->oPSchElementBase.oPSchElementVars.oSchElementGraph.oDbObjectPos =
								p_PSchElementVars->oSchElementGraph.oDbObjectPos;
						LOG_P_2(LOG_CAT_I, "Element [" << QString(p_Element->oPSchElementBase.m_chName).toStdString()
								<< "] position.");
					}
					goto gLEx;
				}
			}
			LOG_P_0(LOG_CAT_W, "Wrong element number from client.");
			goto gLEx;
		}
		//======== Раздел PROTO_O_SCH_ELEMENT_NAME. ========
		case PROTO_O_SCH_ELEMENT_NAME:
		{
			int iEC;
			//
			LOG_P_2(LOG_CAT_I, "{In} Element name from client.");
			iEC = PBCountExternal(Element, Environment);
			p_PSchElementName = ((PSchElementName*)p_ReceivedData);
			for(int iE = 0; iE < iEC; iE++) // По всем элементам...
			{
				Element* p_Element;
				//
				p_Element = PBAccessExternal(Element, iE, Environment);
				if(p_Element->oPSchElementBase.oPSchElementVars.ullIDInt == p_PSchElementName->ullIDInt) // При совп. с запрошенным...
				{
					CopyStrArray(p_PSchElementName->m_chName, p_Element->oPSchElementBase.m_chName, SCH_OBJ_NAME_STR_LEN);
					goto gLEx;
				}
			}
			LOG_P_0(LOG_CAT_W, "Wrong element number from client.");
			goto gLEx;
		}
		//======== Раздел PROTO_O_SCH_LINK_VARS. ========
		case PROTO_O_SCH_LINK_VARS:
		{
			int iLC;
			//
			LOG_P_2(LOG_CAT_I, "{In} Link vars from client.");
			iLC = PBCountExternal(Link, Environment);
			p_PSchLinkVars = ((PSchLinkVars*)p_ReceivedData);
			for(int iL = 0; iL < iLC; iL++) // По всем линкам...
			{
				Link* p_Link;
				//
				p_Link = PBAccessExternal(Link, iL, Environment);
				if((p_Link->oPSchLinkBase.oPSchLinkVars.ullIDSrc == p_PSchLinkVars->ullIDSrc) &&
						(p_Link->oPSchLinkBase.oPSchLinkVars.ullIDDst == p_PSchLinkVars->ullIDDst) &&
						(p_Link->oPSchLinkBase.oPSchLinkVars.ushiSrcPort == p_PSchLinkVars->ushiSrcPort) &&
						(p_Link->oPSchLinkBase.oPSchLinkVars.ushiDstPort == p_PSchLinkVars->ushiDstPort))
					// При совпадении с запрошенным...
				{
					p_Link->oPSchLinkBase.oPSchLinkVars.oSchLinkGraph.uchChangesBits |= p_PSchLinkVars->oSchLinkGraph.uchChangesBits;
					if(p_Link->oPSchLinkBase.oPSchLinkVars.oSchLinkGraph.uchChangesBits & SCH_LINK_BIT_SCR_PORT_POS)
					{
						p_Link->oPSchLinkBase.oPSchLinkVars.oSchLinkGraph.oDbSrcPortGraphPos =
								p_PSchLinkVars->oSchLinkGraph.oDbSrcPortGraphPos;
						LOG_P_2(LOG_CAT_I, "Link [" << QString(p_Link->p_SrcElement->oPSchElementBase.m_chName).toStdString() << "<>" <<
								QString(p_Link->p_DstElement->oPSchElementBase.m_chName).toStdString()
								<< "] vars - src port position.");
					}
					if(p_Link->oPSchLinkBase.oPSchLinkVars.oSchLinkGraph.uchChangesBits & SCH_LINK_BIT_DST_PORT_POS)
					{
						p_Link->oPSchLinkBase.oPSchLinkVars.oSchLinkGraph.oDbDstPortGraphPos =
								p_PSchLinkVars->oSchLinkGraph.oDbDstPortGraphPos;
						LOG_P_2(LOG_CAT_I, "Link [" << QString(p_Link->p_SrcElement->oPSchElementBase.m_chName).toStdString() << "<>" <<
								QString(p_Link->p_DstElement->oPSchElementBase.m_chName).toStdString()
								<< "] vars - dst port position.");
					}
					goto gLEx;
				}
			}
			LOG_P_0(LOG_CAT_W, "Wrong link number from client.");
			goto gLEx;
		}
		//======== Раздел PROTO_O_SCH_GROUP_VARS. ========
		case PROTO_O_SCH_GROUP_VARS:
		{
			int iGC;
			//
			LOG_P_2(LOG_CAT_I, "{In} Group vars from client.");
			iGC = PBCountExternal(Group, Environment);
			p_PSchGroupVars = ((PSchGroupVars*)p_ReceivedData);
			for(int iE = 0; iE < iGC; iE++) // По всем группам...
			{
				Group* p_Group;
				//
				p_Group = PBAccessExternal(Group, iE, Environment);
				if(p_Group->oPSchGroupBase.oPSchGroupVars.ullIDInt == p_PSchGroupVars->ullIDInt) // При совп. с запрошенным...
				{
					if(p_PSchGroupVars->oSchGroupGraph.uchChangesBits & SCH_GROUP_BIT_FRAME)
					{
						p_Group->oPSchGroupBase.oPSchGroupVars.oSchGroupGraph.oDbObjectFrame =
								p_PSchGroupVars->oSchGroupGraph.oDbObjectFrame;
						LOG_P_2(LOG_CAT_I, "Group [" << QString(p_Group->oPSchGroupBase.m_chName).toStdString()
								<< "] frame.");
					}
					if(p_PSchGroupVars->oSchGroupGraph.uchChangesBits & SCH_GROUP_BIT_ZPOS)
					{
						p_Group->oPSchGroupBase.oPSchGroupVars.oSchGroupGraph.dbObjectZPos =
								p_PSchGroupVars->oSchGroupGraph.dbObjectZPos;
						LOG_P_2(LOG_CAT_I, "Group [" << QString(p_Group->oPSchGroupBase.m_chName).toStdString()
								<< "] z-pos is: " << QString::number((int)p_PSchGroupVars->oSchGroupGraph.dbObjectZPos).toStdString());
					}
					goto gLEx;
				}
			}
			LOG_P_0(LOG_CAT_W, "Wrong group number from client.");
			goto gLEx;
		}
		//======== Раздел PROTO_O_SCH_GROUP_NAME. ========
		case PROTO_O_SCH_GROUP_NAME:
		{
			int iEC;
			//
			LOG_P_2(LOG_CAT_I, "{In} Group name from client.");
			iEC = PBCountExternal(Group, Environment);
			p_PSchGroupName = ((PSchGroupName*)p_ReceivedData);
			for(int iE = 0; iE < iEC; iE++) // По всем группам...
			{
				Group* p_Group;
				//
				p_Group = PBAccessExternal(Group, iE, Environment);
				if(p_Group->oPSchGroupBase.oPSchGroupVars.ullIDInt == p_PSchGroupName->ullIDInt) // При совп. с запрошенным...
				{
					CopyStrArray(p_PSchGroupName->m_chName, p_Group->oPSchGroupBase.m_chName, SCH_OBJ_NAME_STR_LEN);
					goto gLEx;
				}
			}
			LOG_P_0(LOG_CAT_W, "Wrong group number from client.");
			goto gLEx;
		}
		//======== Раздел PROTO_O_SCH_ELEMENT_BASE. ========
		case PROTO_O_SCH_ELEMENT_BASE:
		{
			Element* p_Element;
			//
			p_PSchElementBase = ((PSchElementBase*)p_ReceivedData);
			LOG_P_2(LOG_CAT_I, "{In} Element [" << QString(p_PSchElementBase->m_chName).toStdString()
					<< "] base from client.");
			AppendToPBExternal(Element, p_Element = new Element(*p_PSchElementBase), Environment);
			p_Element->bNew = false;
			p_Element->chTouchedBits = 0;
		}
		//======== Следующий раздел... ========
	}
}

// Кэлбэк обработки приходящих запросов.
void MainWindow::ClientRequestArrivedCallback(int iConnection, unsigned short ushRequest)
{
	//
	LOG_P_2(LOG_CAT_I, "Client: " << iConnection << " request: " << ushRequest);
}

// Процедуры запуска сервера.
bool MainWindow::ServerStartProcedures(NetHub::IPPortPassword& o_IPPortPassword)
{
	Message_Dialog* p_Message_Dialog;
	//
	p_ui->action_ServerName->setDisabled(true);
	p_ui->action_ServerSettings->setDisabled(true);
	SetStatusBarText("Запуск сервера...");
	if(!p_Server->Start(&o_IPPortPassword, m_chServerName))
	{
		goto gSS;
	}
	for(unsigned char uchAtt = 0; uchAtt != SERVER_WAITING_ATTEMPTS; uchAtt++)
	{
		if(p_Server->CheckServerAlive())
		{
			SetStatusBarText(m_chStatusWorking);
			p_ui->label_ConnectedClient->setText(m_chClientLabelWaiting);
			return true;
		}
		MSleep(USER_RESPONSE_MS);
	}
gSS:LOG_P_0(LOG_CAT_E, m_chLogCantStart << "server.");
	p_Message_Dialog = new Message_Dialog(m_chMsgError, "Невозможно запустить сервер");
	p_Message_Dialog->exec();
	p_Message_Dialog->deleteLater();
	SetStatusBarText(m_chStatusReady);
	p_ui->action_StartStopServer->setChecked(false);
	p_ui->action_ServerName->setDisabled(false);
	p_ui->action_ServerSettings->setDisabled(false);
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
				SetStatusBarText(m_chStatusReady);
				p_ui->action_ServerName->setDisabled(false);
				p_ui->action_ServerSettings->setDisabled(false);
				p_ui->label_ConnectedClient->clear();
				return true;
			}
			MSleep(USER_RESPONSE_MS);
		}
	gSB:LOG_P_0(LOG_CAT_E, m_chLogCantStop << "server.");
		p_Message_Dialog = new Message_Dialog(m_chMsgError, "Невозможно остановить сервер");
		p_Message_Dialog->exec();
		p_Message_Dialog->deleteLater();
		SetStatusBarText(m_chStatusWorking);
		return false;
	}
	else return true;
}

// Процедуры запуска среды.
bool MainWindow::EnvStartProcedures()
{
	if(!p_Environment->Start())
	{
		p_ui->action_StartStopEnv->setChecked(false);
		return false;
	}
	p_ui->action_ChangeEnv->setDisabled(true);
	p_ui->action_SaveCurrent->setDisabled(true);
	return true;
}

// Процедуры остановки среды.
bool MainWindow::EnvStopProcedures()
{
	p_Environment->Stop();
	p_ui->action_ChangeEnv->setDisabled(false);
	p_ui->action_SaveCurrent->setDisabled(false);
	return true;
}

// Установка текста строки статуса.
void MainWindow::SetStatusBarText(QString strMsg)
{
	p_QLabelStatusBarText->setText(strMsg);
	p_ui->statusBar->repaint();
}

// При переключении кнопки 'Запуск \ остановка сервера'.
void MainWindow::on_action_StartStopServer_triggered(bool checked)
{
	if(checked)
	{
		LCHECK_BOOL(ServerStartProcedures(oIPPortPassword));
	}
	else
	{
		LCHECK_BOOL(ServerStopProcedures());
	}
}

// При переключении кнопки 'Запуск при входе в приложение'.
void MainWindow::on_action_StartOnLaunchApp_triggered(bool checked)
{
	 p_UISettings->setValue("AutostartServer", checked);
}

// При нажатии кнопки 'Имя сервера'.
void MainWindow::on_action_ServerName_triggered()
{
	Set_Proposed_String_Dialog* p_Set_Proposed_String_Dialog;
	//
	p_Set_Proposed_String_Dialog = new Set_Proposed_String_Dialog((char*)"Имя сервера", m_chServerName, SERVER_NAME_STR_LEN);
	if(p_Set_Proposed_String_Dialog->exec() == DIALOGS_ACCEPT)
	{
		LCHECK_BOOL(SaveServerConfig());
		LOG_P_0(LOG_CAT_I, m_chLogServerUpdated);
		LOG_P_1(LOG_CAT_I, "Server name: " << m_chServerName);
	}
}

// При нажатии кнопки 'Основные параметры сервера'.
void MainWindow::on_action_ServerSettings_triggered()
{
	Set_Server_Dialog* p_Set_Server_Dialog;
	Message_Dialog* p_Message_Dialog;
	NumericAddress oNumericAddress;
	//
gA: p_Set_Server_Dialog = new Set_Server_Dialog(m_chIP, m_chPort, m_chPassword);
	p_Set_Server_Dialog->deleteLater();
	if(p_Set_Server_Dialog->exec() == DIALOGS_ACCEPT)
	{
		FillNumericStructWithIPPortStrs(oNumericAddress, Set_Server_Dialog::oIPPortPasswordStrings.strIP,
										Set_Server_Dialog::oIPPortPasswordStrings.strPort);
		if(!oNumericAddress.bIsCorrect)
		{
			p_Message_Dialog = new Message_Dialog(m_chMsgError, m_chMsgWrongIPPort);
			p_Message_Dialog->exec();
			p_Message_Dialog->deleteLater();
			goto gA;
		}
		CopyStrArray((char*)Set_Server_Dialog::oIPPortPasswordStrings.strIP.toStdString().c_str(), m_chIP, IP_STR_LEN);
		CopyStrArray((char*)Set_Server_Dialog::oIPPortPasswordStrings.strPort.toStdString().c_str(), m_chPort, PORT_STR_LEN);
		CopyStrArray((char*)Set_Server_Dialog::oIPPortPasswordStrings.strPassword.toStdString().c_str(), m_chPassword, AUTH_PASSWORD_STR_LEN);
		LCHECK_BOOL(SaveServerConfig());
		LOG_P_0(LOG_CAT_I, m_chLogServerUpdated);
		LOG_P_1(LOG_CAT_I, "Server IP: " << oIPPortPassword.p_chIPNameBuffer);
		LOG_P_1(LOG_CAT_I, "Server port: " << oIPPortPassword.p_chPortNameBuffer);
	}
}

// При нажатии кнопки 'Старт \ стоп среды'.
void MainWindow::on_action_StartStopEnv_triggered(bool checked)
{
	if(checked)
	{
		LCHECK_BOOL(EnvStartProcedures());
	}
	else
	{
		LCHECK_BOOL(EnvStopProcedures());
	}
}

// При нажатии кнопки 'Старт при запуске сервера'.
void MainWindow::on_action_StartOnLaunchServer_triggered(bool checked)
{
	p_UISettings->setValue("AutostartEnv", checked);
}

// При нажатии кнопки 'Сменить среду'.
void MainWindow::on_action_ChangeEnv_triggered()
{
	Set_Proposed_String_Dialog* p_Set_Proposed_String_Dialog;
	Set_Proposed_Bool_Dialog* p_Set_Proposed_Bool_Dialog;
	char m_chEnvNameOld[ENV_NAME_LEN];
	//
	CopyStrArray(m_chEnvName, m_chEnvNameOld, ENV_NAME_LEN);
	p_Set_Proposed_String_Dialog = new Set_Proposed_String_Dialog((char*)"Смена среды", m_chEnvName, ENV_NAME_LEN);
	if(p_Set_Proposed_String_Dialog->exec() == DIALOGS_ACCEPT)
	{
		if(!p_Environment->CheckInitialized()) goto gN;
		p_Set_Proposed_Bool_Dialog = new Set_Proposed_Bool_Dialog((char*)m_chMsgWarning,
																  (char*)"Все несохранённые данные среды будут утеряны. Продолжить?");
		if(p_Set_Proposed_Bool_Dialog->exec() == DIALOGS_ACCEPT)
		{
gN:			LCHECK_BOOL(SaveEnvConfig());
			LOG_P_0(LOG_CAT_I, m_chLogEnvUpdated);
			LOG_P_1(LOG_CAT_I, (char*)m_chLogCurrentEnv << m_chEnvName);
			delete p_Environment;
			p_Environment = new Environment(LOG_MUTEX, m_chEnvName);
		}
		else
		{
			CopyStrArray(m_chEnvNameOld, m_chEnvName, ENV_NAME_LEN);
		}
	}
}

// При нажатии кнопки 'Сохранение при выходе из приложения'.
void MainWindow::on_action_Autosave_triggered(bool checked)
{
	p_UISettings->setValue("Autosave", checked);
}

// При нажатии кнопки 'Сохранение текущей среды'
void MainWindow::on_action_SaveCurrent_triggered()
{
	LCHECK_BOOL(p_Environment->SaveEnv());
}
