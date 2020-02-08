//== ВКЛЮЧЕНИЯ.
#include <QMap>
#include <QColor>
#include "environment.h"

//== МАКРОСЫ.
#define LOG_NAME				"environment"
#define LOG_DIR_PATH			"../Z-Hub/logs/"

//== ДЕКЛАРАЦИИ СТАТИЧЕСКИХ ПЕРЕМЕННЫХ.
// Основное.
LOGDECL_INIT_INCLASS(Environment)
LOGDECL_INIT_PTHRD_INCLASS_EXT_ADD(Environment)
// Буферы.
StaticPBSourceInit(Element,, Environment, MAX_ELEMENTS)
StaticPBSourceInit(Link,, Environment, MAX_LINKS)
StaticPBSourceInit(Group,, Environment, MAX_GROUPS)
//
char* Environment::p_chEnvNameInt = nullptr;
bool Environment::bEnvLoaded = false;
QString Environment::strEnvPath;
QString Environment::strEnvFilename;

//== ФУНКЦИИ КЛАССОВ.
//== Класс среды.
// Конструктор.
Environment::Environment(pthread_mutex_t ptLogMutex, char* p_chEnvName)
{
	LOG_CTRL_BIND_EXT_MUTEX(ptLogMutex);
	LOG_CTRL_INIT;
	p_chEnvNameInt = p_chEnvName;
	bEnvLoaded = false;
}

// Деструктор.
Environment::~Environment()
{
	ReleasePB(Group);
	ReleasePB(Link);
	ReleasePB(Element);
	LOG_CLOSE;
}

// Загрузка среды.
bool Environment::LoadEnv()
{
	PSchGroupBase oPSchGroupBase;
	PSchElementBase oPSchElementBase;
	PSchLinkBase oPSchLinkBase;
	XMLError eResult;
	tinyxml2::XMLDocument xmlDocEnv;
	QStringList lstrHelper;
	bool bPresent;
	double dbZ = 0;
	QMap<double, XMLNode*> oQMapNums;
	QMap<double, XMLNode*>::Iterator itNums;
	XMLNode* p_NodeZHelper;
	QString strHelper;
	list<XMLNode*> l_pGroups;
	list<XMLNode*> l_pElements;
	list<XMLNode*> l_pLinks;
	list<XMLNode*> l_pZs;
	//
	strEnvPath.clear();
	strEnvFilename.clear();
	strEnvFilename = p_chEnvNameInt;
	strEnvFilename += m_chXML;
	strEnvPath = ENVS_DIR;
	strEnvPath += strEnvFilename;
	LOG_P_0(LOG_CAT_I, "Loading environment from: " << strEnvFilename.toStdString());
	eResult = xmlDocEnv.LoadFile(strEnvPath.toStdString().c_str());
	if (eResult != XML_SUCCESS)
	{
		LOG_P_0(LOG_CAT_E, "Can`t open environment file: " << strEnvFilename.toStdString());
		return false;
	}
	else
	{
		LOG_P_1(LOG_CAT_I, "Environment file loaded: " << strEnvFilename.toStdString());
	}
	//
	FindChildNodes(xmlDocEnv.LastChild(), l_pZs, "Z", FCN_MULT_LEVELS, FCN_ALL);
	while(!l_pZs.empty())
	{
		p_NodeZHelper = l_pZs.front();
		l_pZs.pop_front();
		strHelper = QString(p_NodeZHelper->FirstChild()->Value());
		oQMapNums.insert(strHelper.toDouble(), p_NodeZHelper);
	}
	for(itNums = oQMapNums.begin(); itNums != oQMapNums.end(); itNums++)
	{
		p_NodeZHelper = itNums.value();
		p_NodeZHelper->FirstChild()->SetValue(strHelper.setNum(dbZ).toStdString().c_str());
		dbZ += SCH_NEXT_Z_SHIFT;
	}
	//
	if(!FindChildNodes(xmlDocEnv.LastChild(), l_pGroups,
					   "Groups", FCN_ONE_LEVEL, FCN_FIRST_ONLY))
	{
		LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << "'Groups'" << m_chLogEnvNodeAbsend);
		return false;
	}
	if(!FindChildNodes(xmlDocEnv.LastChild(), l_pElements,
					   "Elements", FCN_ONE_LEVEL, FCN_FIRST_ONLY))
	{
		LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << "'Elements'" << m_chLogEnvNodeAbsend);
		return false;
	}
	if(!FindChildNodes(xmlDocEnv.LastChild(), l_pLinks,
					   "Links", FCN_ONE_LEVEL, FCN_FIRST_ONLY))
	{
		LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << "'Links'" << m_chLogEnvNodeAbsend);
		return false;
	}
	// ГРУППЫ.
	PARSE_CHILDLIST(l_pGroups.front(), p_ListGroups, "Group",
					FCN_ONE_LEVEL, p_NodeGroup)
	{
		memset(&oPSchGroupBase, 0, sizeof(oPSchGroupBase));
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeGroup, p_ListIDs,
						  "ID", FCN_ONE_LEVEL, p_NodeID)
		{
			strHelper = QString(p_NodeID->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << "'Group' node format incorrect - wrong 'ID' node.");
				return false;
			}
			oPSchGroupBase.oPSchGroupVars.ullIDInt = strHelper.toULongLong();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListIDs);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << "'Group' node format incorrect - missing 'ID' node.");
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeGroup, p_ListNames,
						  "Name", FCN_ONE_LEVEL, p_NodeName)
		{
			strHelper = QString(p_NodeName->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << "'Group' node format incorrect - wrong 'Name' node.");
				return false;
			}
			memcpy(oPSchGroupBase.m_chName,
				   strHelper.toStdString().c_str(), SizeOfChars(strHelper.toStdString().length() + 1));
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListNames);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << "'Group' node format incorrect - missing 'Name' node.");
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeGroup, p_ListBkgColors,
						  "BkgColor", FCN_ONE_LEVEL, p_NodeBkgColor)
		{
			strHelper = QString(p_NodeBkgColor->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << "'Group' node format incorrect - wrong 'BkgColor' node.");
				return false;
			}
			lstrHelper = strHelper.split(',');
			if(lstrHelper.count() != 3)
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << "'Group' node format incorrect - wrong 'BkgColor' node.");
				return false;
			}
			oPSchGroupBase.oPSchGroupVars.oSchGroupGraph.uiObjectBkgColor =
					QColor(lstrHelper.at(0).toUInt(),
						   lstrHelper.at(1).toUInt(),
						   lstrHelper.at(2).toUInt()).rgb();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListBkgColors);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << "'Group' node format incorrect - missing 'BkgColor' node.");
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeGroup, p_ListFrames,
						  "Frame", FCN_ONE_LEVEL, p_NodeFrame)
		{
			strHelper = QString(p_NodeFrame->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << "'Group' node format incorrect - wrong 'Frame' node.");
				return false;
			}
			lstrHelper = strHelper.split(',');
			if(lstrHelper.count() != 4)
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << "'Group' node format incorrect - wrong 'Frame' node.");
				return false;
			}
			oPSchGroupBase.oPSchGroupVars.oSchGroupGraph.oDbObjectFrame.dbX = lstrHelper.at(0).toDouble();
			oPSchGroupBase.oPSchGroupVars.oSchGroupGraph.oDbObjectFrame.dbY = lstrHelper.at(1).toDouble();
			oPSchGroupBase.oPSchGroupVars.oSchGroupGraph.oDbObjectFrame.dbW = lstrHelper.at(2).toDouble();
			oPSchGroupBase.oPSchGroupVars.oSchGroupGraph.oDbObjectFrame.dbH = lstrHelper.at(3).toDouble();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListFrames);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << "'Group' node format incorrect - missing 'Frame' node.");
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeGroup, p_ListZs,
						  "Z", FCN_ONE_LEVEL, p_NodeZ)
		{
			strHelper = QString(p_NodeZ->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << "'Group' node format incorrect - wrong 'Z' node.");
				return false;
			}
			oPSchGroupBase.oPSchGroupVars.oSchGroupGraph.dbObjectZPos = strHelper.toDouble();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListZs);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << "'Group' node format incorrect - missing 'Z' node.");
			return false;
		}
		AppendToPB(Group, new Group(oPSchGroupBase));
	} PARSE_CHILDLIST_END(p_ListGroups);
	// ЭЛЕМЕНТЫ.
	PARSE_CHILDLIST(l_pElements.front(), p_ListElements, "Element",
					FCN_ONE_LEVEL, p_NodeElement)
	{
		memset(&oPSchElementBase, 0, sizeof(oPSchElementBase));
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeElement, p_ListIDs,
						  "ID", FCN_ONE_LEVEL, p_NodeID)
		{
			strHelper = QString(p_NodeID->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << "'Element' node format incorrect - wrong 'ID' node.");
				return false;
			}
			oPSchElementBase.oPSchElementVars.ullIDInt = strHelper.toULongLong();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListIDs);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << "'Element' node format incorrect - missing 'ID' node.");
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeElement, p_ListNames,
						  "Name", FCN_ONE_LEVEL, p_NodeName)
		{
			strHelper = QString(p_NodeName->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << "'Element' node format incorrect - wrong 'Name' node.");
				return false;
			}
			memcpy(oPSchElementBase.m_chName,
				   strHelper.toStdString().c_str(), SizeOfChars(strHelper.toStdString().length() + 1));
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListNames);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << "'Element' node format incorrect - missing 'Name' node.");
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeElement, p_ListBkgColors,
						  "BkgColor", FCN_ONE_LEVEL, p_NodeBkgColor)
		{
			strHelper = QString(p_NodeBkgColor->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << "'Element' node format incorrect - wrong 'BkgColor' node.");
				return false;
			}
			lstrHelper = strHelper.split(',');
			if(lstrHelper.count() != 3)
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << "'Element' node format incorrect - wrong 'BkgColor' node.");
				return false;
			}
			oPSchElementBase.oPSchElementVars.oSchElementGraph.uiObjectBkgColor =
					QColor(lstrHelper.at(0).toUInt(),
						   lstrHelper.at(1).toUInt(),
						   lstrHelper.at(2).toUInt()).rgb();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListBkgColors);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << "'Element' node format incorrect - missing 'BkgColor' node.");
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeElement, p_ListFrames,
						  "Frame", FCN_ONE_LEVEL, p_NodeFrame)
		{
			strHelper = QString(p_NodeFrame->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << "'Element' node format incorrect - wrong 'Frame' node.");
				return false;
			}
			lstrHelper = strHelper.split(',');
			if(lstrHelper.count() != 4)
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << "'Element' node format incorrect - wrong 'Frame' node.");
				return false;
			}
			oPSchElementBase.oPSchElementVars.oSchElementGraph.oDbObjectFrame.dbX = lstrHelper.at(0).toDouble();
			oPSchElementBase.oPSchElementVars.oSchElementGraph.oDbObjectFrame.dbY = lstrHelper.at(1).toDouble();
			oPSchElementBase.oPSchElementVars.oSchElementGraph.oDbObjectFrame.dbW = lstrHelper.at(2).toDouble();
			oPSchElementBase.oPSchElementVars.oSchElementGraph.oDbObjectFrame.dbH = lstrHelper.at(3).toDouble();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListFrames);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << "'Element' node format incorrect - missing 'Frame' node.");
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeElement, p_ListZs,
						  "Z", FCN_ONE_LEVEL, p_NodeZ)
		{
			strHelper = QString(p_NodeZ->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << "'Element' node format incorrect - wrong 'Z' node.");
				return false;
			}
			oPSchElementBase.oPSchElementVars.oSchElementGraph.dbObjectZPos = strHelper.toDouble();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListZs);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << "'Element' node format incorrect - missing 'Z' node.");
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeElement, p_ListPoss,
						  "Pos", FCN_ONE_LEVEL, p_NodePos)
		{
			strHelper = QString(p_NodePos->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << "'Element' node format incorrect - wrong 'Pos' node.");
				return false;
			}
			lstrHelper = strHelper.split(',');
			if(lstrHelper.count() != 2)
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << "'Element' node format incorrect - wrong 'Pos' node.");
				return false;
			}
			oPSchElementBase.oPSchElementVars.oSchElementGraph.oDbObjectPos.dbX = lstrHelper.at(0).toDouble();
			oPSchElementBase.oPSchElementVars.oSchElementGraph.oDbObjectPos.dbY = lstrHelper.at(1).toDouble();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListPoss);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << "'Element' node format incorrect - missing 'Pos' node.");
			return false;
		}
		FIND_IN_CHILDLIST(p_NodeElement, p_ListGroupIDs,
						  "GroupID", FCN_ONE_LEVEL, p_NodeGroupID)
		{
			strHelper = QString(p_NodeGroupID->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << "'Element' node format incorrect - wrong 'GroupID' node.");
				return false;
			}
			oPSchElementBase.oPSchElementVars.ullIDGroup = strHelper.toULongLong();
		} FIND_IN_CHILDLIST_END(p_ListGroupIDs);
		AppendToPB(Element, new Element(oPSchElementBase));
	} PARSE_CHILDLIST_END(p_ListElements);
	// ЛИНКИ.
	PARSE_CHILDLIST(l_pLinks.front(), p_ListLinks, "Link",
					FCN_ONE_LEVEL, p_NodeLink)
	{
		memset(&oPSchLinkBase, 0, sizeof(oPSchLinkBase));
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeLink, p_ListSrcIDs,
						  "SrcID", FCN_ONE_LEVEL, p_NodeSrcID)
		{
			strHelper = QString(p_NodeSrcID->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << "'Link' node format incorrect - wrong 'SrcID' node.");
				return false;
			}
			oPSchLinkBase.oPSchLinkVars.ullIDSrc = strHelper.toULongLong();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListSrcIDs);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << "'Link' node format incorrect - missing 'SrcID' node.");
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeLink, p_ListSrcPortIDs,
						  "SrcPortID", FCN_ONE_LEVEL, p_NodeSrcPortID)
		{
			strHelper = QString(p_NodeSrcPortID->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << "'Link' node format incorrect - wrong 'SrcPortID' node.");
				return false;
			}
			oPSchLinkBase.oPSchLinkVars.ushiSrcPort = strHelper.toUShort();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListSrcPortIDs);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << "'Link' node format incorrect - missing 'SrcPortID' node.");
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeLink, p_ListSrcPortPoss,
						  "SrcPortPos", FCN_ONE_LEVEL, p_NodeSrcPortPos)
		{
			strHelper = QString(p_NodeSrcPortPos->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << "'Element' node format incorrect - wrong 'SrcPortPos' node.");
				return false;
			}
			lstrHelper = strHelper.split(',');
			if(lstrHelper.count() != 2)
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << "'Element' node format incorrect - wrong 'SrcPortPos' node.");
				return false;
			}
			oPSchLinkBase.oPSchLinkVars.oSchLinkGraph.oDbSrcPortGraphPos.dbX = lstrHelper.at(0).toDouble();
			oPSchLinkBase.oPSchLinkVars.oSchLinkGraph.oDbSrcPortGraphPos.dbY = lstrHelper.at(1).toDouble();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListSrcPortPoss);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << "'Element' node format incorrect - missing 'SrcPortPos' node.");
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeLink, p_ListDstIDs,
						  "DstID", FCN_ONE_LEVEL, p_NodeDstID)
		{
			strHelper = QString(p_NodeDstID->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << "'Link' node format incorrect - wrong 'DstID' node.");
				return false;
			}
			oPSchLinkBase.oPSchLinkVars.ullIDDst = strHelper.toULongLong();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListDstIDs);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << "'Link' node format incorrect - missing 'DstID' node.");
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeLink, p_ListDstPortIDs,
						  "DstPortID", FCN_ONE_LEVEL, p_NodeDstPortID)
		{
			strHelper = QString(p_NodeDstPortID->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << "'Link' node format incorrect - wrong 'DstPortID' node.");
				return false;
			}
			oPSchLinkBase.oPSchLinkVars.ushiDstPort = strHelper.toUShort();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListDstPortIDs);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << "'Link' node format incorrect - missing 'DstPortID' node.");
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeLink, p_ListDstPortPoss,
						  "DstPortPos", FCN_ONE_LEVEL, p_NodeDstPortPos)
		{
			strHelper = QString(p_NodeDstPortPos->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << "'Element' node format incorrect - wrong 'DstPortPos' node.");
				return false;
			}
			lstrHelper = strHelper.split(',');
			if(lstrHelper.count() != 2)
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << "'Element' node format incorrect - wrong 'DstPortPos' node.");
				return false;
			}
			oPSchLinkBase.oPSchLinkVars.oSchLinkGraph.oDbDstPortGraphPos.dbX = lstrHelper.at(0).toDouble();
			oPSchLinkBase.oPSchLinkVars.oSchLinkGraph.oDbDstPortGraphPos.dbY = lstrHelper.at(1).toDouble();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListDstPortPoss);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << "'Element' node format incorrect - missing 'DstPortPos' node.");
			return false;
		}
		AppendToPB(Link, new Link(oPSchLinkBase));
	} PARSE_CHILDLIST_END(p_ListLinks);
	//
	LOG_P_1(LOG_CAT_I, "Environment has been initialized.");
	bEnvLoaded = true;
	return true;
}

// Сохранение среды.
bool Environment::SaveEnv()
{
	LOG_P_0(LOG_CAT_I, "Saving environment to: " << strEnvFilename.toStdString());
	return true;
}

// Запуск среды.
bool Environment::Start()
{
	if(!bEnvLoaded)
	{
		if(!LoadEnv()) return false;
	}
	LOG_P_0(LOG_CAT_I, "Start environment.");
	return true;
}

// Остановка среды.
void Environment::Stop()
{
	LOG_P_0(LOG_CAT_I, "Stop environment.");
}

// Проверка инициализированности среды.
bool Environment::CheckInitialized()
{
	return bEnvLoaded;
}
