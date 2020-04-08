//== ВКЛЮЧЕНИЯ.
#include <QMap>
#include <QColor>
#include "environment.h"
#include "main-window.h"

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
bool Environment::bEnvThreadAlive = false;
bool Environment::bStopEnvUpdate = false;
pthread_t Environment::thrEnv;
bool Environment::bRequested = false;
PSchReadyFrame Environment::oPSchReadyFrame;

//== ФУНКЦИИ КЛАССОВ.
//== Класс среды.
// Конструктор.
Environment::Environment(pthread_mutex_t ptLogMutex, char* p_chEnvName)
{
	LOG_CTRL_BIND_EXT_MUTEX(ptLogMutex);
	LOG_CTRL_INIT;
	p_chEnvNameInt = p_chEnvName;
	bEnvLoaded = false;
	bEnvThreadAlive = false;
	bStopEnvUpdate = false;
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
	LOG_P_1(LOG_CAT_I, "Loading environment from: " << strEnvFilename.toStdString());
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
	FindChildNodes(xmlDocEnv.LastChild(), l_pZs, m_chZ, FCN_MULT_LEVELS, FCN_ALL);
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
					   m_chGroups, FCN_ONE_LEVEL, FCN_FIRST_ONLY))
	{
		LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << "'Groups'" << m_chLogEnvNodeAbsend);
		return false;
	}
	if(!FindChildNodes(xmlDocEnv.LastChild(), l_pElements,
					   m_chElements, FCN_ONE_LEVEL, FCN_FIRST_ONLY))
	{
		LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << "'Elements'" << m_chLogEnvNodeAbsend);
		return false;
	}
	if(!FindChildNodes(xmlDocEnv.LastChild(), l_pLinks,
					   m_chLinks, FCN_ONE_LEVEL, FCN_FIRST_ONLY))
	{
		LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << "'Links'" << m_chLogEnvNodeAbsend);
		return false;
	}
	// ГРУППЫ.
	PARSE_CHILDLIST(l_pGroups.front(), p_ListGroups, m_chGroup,
					FCN_ONE_LEVEL, p_NodeGroup)
	{
		memset(&oPSchGroupBase, 0, sizeof(oPSchGroupBase));
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeGroup, p_ListIDs,
						  m_chID, FCN_ONE_LEVEL, p_NodeID)
		{
			strHelper = QString(p_NodeID->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvGroup << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogID << m_chLogNode);
				return false;
			}
			oPSchGroupBase.oPSchGroupVars.ullIDInt = strHelper.toULongLong();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListIDs);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvGroup << m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogID << m_chLogNode);
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeGroup, p_ListNames,
						  m_chName, FCN_ONE_LEVEL, p_NodeName)
		{
			strHelper = QString(p_NodeName->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvGroup << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogName << m_chLogNode);
				return false;
			}
			memcpy(oPSchGroupBase.m_chName,
				   strHelper.toStdString().c_str(), SizeOfChars(strHelper.toStdString().length() + 1));
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListNames);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvGroup <<
					m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogName << m_chLogNode);
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeGroup, p_ListBkgColors,
						  m_chBkgColor, FCN_ONE_LEVEL, p_NodeBkgColor)
		{
			strHelper = QString(p_NodeBkgColor->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvGroup << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogBkgColor << m_chLogNode);
				return false;
			}
			lstrHelper = strHelper.split(',');
			if(lstrHelper.count() != 4)
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvGroup << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogBkgColor << m_chLogNode);
				return false;
			}
			oPSchGroupBase.oPSchGroupVars.oSchGroupGraph.uiObjectBkgColor =
					QColor(lstrHelper.at(0).toUInt(),
						   lstrHelper.at(1).toUInt(),
						   lstrHelper.at(2).toUInt(),
						   lstrHelper.at(3).toUInt()).rgba();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListBkgColors);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvGroup <<
					m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogBkgColor << m_chLogNode);
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeGroup, p_ListFrames,
						  m_chFrame, FCN_ONE_LEVEL, p_NodeFrame)
		{
			strHelper = QString(p_NodeFrame->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvGroup << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogFrame << m_chLogNode);
				return false;
			}
			lstrHelper = strHelper.split(',');
			if(lstrHelper.count() != 4)
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvGroup << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogFrame << m_chLogNode);
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
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvGroup <<
					m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogFrame << m_chLogNode);
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeGroup, p_ListZs,
						  m_chZ, FCN_ONE_LEVEL, p_NodeZ)
		{
			strHelper = QString(p_NodeZ->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvGroup << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogZ << m_chLogNode);
				return false;
			}
			oPSchGroupBase.oPSchGroupVars.oSchGroupGraph.dbObjectZPos = strHelper.toDouble();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListZs);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvGroup << m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogZ << m_chLogNode);
			return false;
		}
		FIND_IN_CHILDLIST(p_NodeGroup, p_ListGroupIDs,
						  m_chGroupID, FCN_ONE_LEVEL, p_NodeGroupID)
		{
			strHelper = QString(p_NodeGroupID->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvGroup << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << "'GroupID'" << m_chLogNode);
				return false;
			}
			oPSchGroupBase.oPSchGroupVars.ullIDGroup = strHelper.toULongLong();
		} FIND_IN_CHILDLIST_END(p_ListGroupIDs);
		AppendToPB(Group, new Group(oPSchGroupBase));
	} PARSE_CHILDLIST_END(p_ListGroups);
	// ЭЛЕМЕНТЫ.
	PARSE_CHILDLIST(l_pElements.front(), p_ListElements, m_chElement,
					FCN_ONE_LEVEL, p_NodeElement)
	{
		memset(&oPSchElementBase, 0, sizeof(oPSchElementBase));
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeElement, p_ListIDs,
						  m_chID, FCN_ONE_LEVEL, p_NodeID)
		{
			strHelper = QString(p_NodeID->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogID << m_chLogNode);
				return false;
			}
			oPSchElementBase.oPSchElementVars.ullIDInt = strHelper.toULongLong();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListIDs);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvElement <<
					m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogID << m_chLogNode);
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeElement, p_ListNames,
						  m_chName, FCN_ONE_LEVEL, p_NodeName)
		{
			strHelper = QString(p_NodeName->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogName << m_chLogNode);
				return false;
			}
			memcpy(oPSchElementBase.m_chName,
				   strHelper.toStdString().c_str(), SizeOfChars(strHelper.toStdString().length() + 1));
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListNames);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvElement
					<< m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogName << m_chLogNode);
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeElement, p_ListBkgColors,
						  m_chBkgColor, FCN_ONE_LEVEL, p_NodeBkgColor)
		{
			strHelper = QString(p_NodeBkgColor->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogBkgColor << m_chLogNode);
				return false;
			}
			lstrHelper = strHelper.split(',');
			if(lstrHelper.count() != 4)
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogBkgColor << m_chLogNode);
				return false;
			}
			oPSchElementBase.oPSchElementVars.oSchElementGraph.uiObjectBkgColor =
					QColor(lstrHelper.at(0).toUInt(),
						   lstrHelper.at(1).toUInt(),
						   lstrHelper.at(2).toUInt(),
						   lstrHelper.at(3).toUInt()).rgba();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListBkgColors);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt <<
					m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogBkgColor << m_chLogNode);
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeElement, p_ListFrames,
						  m_chFrame, FCN_ONE_LEVEL, p_NodeFrame)
		{
			strHelper = QString(p_NodeFrame->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogFrame << m_chLogNode);
				return false;
			}
			lstrHelper = strHelper.split(',');
			if(lstrHelper.count() != 4)
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogFrame << m_chLogNode);
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
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvElement <<
					m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogFrame << m_chLogNode);
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeElement, p_ListZs,
						  m_chZ, FCN_ONE_LEVEL, p_NodeZ)
		{
			strHelper = QString(p_NodeZ->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogZ << m_chLogNode);
				return false;
			}
			oPSchElementBase.oPSchElementVars.oSchElementGraph.dbObjectZPos = strHelper.toDouble();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListZs);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvElement <<
					m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogZ << m_chLogNode);
			return false;
		}
		FIND_IN_CHILDLIST(p_NodeElement, p_ListGroupIDs,
						  m_chGroupID, FCN_ONE_LEVEL, p_NodeGroupID)
		{
			strHelper = QString(p_NodeGroupID->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << "'GroupID'" << m_chLogNode);
				return false;
			}
			oPSchElementBase.oPSchElementVars.ullIDGroup = strHelper.toULongLong();
		} FIND_IN_CHILDLIST_END(p_ListGroupIDs);
		AppendToPB(Element, new Element(oPSchElementBase));
	} PARSE_CHILDLIST_END(p_ListElements);
	// ЛИНКИ.
	PARSE_CHILDLIST(l_pLinks.front(), p_ListLinks, m_chLink,
					FCN_ONE_LEVEL, p_NodeLink)
	{
		memset(&oPSchLinkBase, 0, sizeof(oPSchLinkBase));
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeLink, p_ListSrcIDs,
						  m_chSrcID, FCN_ONE_LEVEL, p_NodeSrcID)
		{
			strHelper = QString(p_NodeSrcID->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvLink << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogSrcID << m_chLogNode);
				return false;
			}
			oPSchLinkBase.oPSchLinkVars.ullIDSrc = strHelper.toULongLong();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListSrcIDs);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvLink <<
					m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogSrcID << m_chLogNode);
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeLink, p_ListSrcPortIDs,
						  m_chSrcPortID, FCN_ONE_LEVEL, p_NodeSrcPortID)
		{
			strHelper = QString(p_NodeSrcPortID->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvLink << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogSrcPortID << m_chLogNode);
				return false;
			}
			oPSchLinkBase.oPSchLinkVars.ushiSrcPort = strHelper.toUShort();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListSrcPortIDs);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvLink <<
					m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogSrcPortID << m_chLogNode);
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeLink, p_ListSrcPortPoss,
						  m_chSrcPortPos, FCN_ONE_LEVEL, p_NodeSrcPortPos)
		{
			strHelper = QString(p_NodeSrcPortPos->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogSrcPortPos << m_chLogNode);
				return false;
			}
			lstrHelper = strHelper.split(',');
			if(lstrHelper.count() != 2)
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogSrcPortPos << m_chLogNode);
				return false;
			}
			oPSchLinkBase.oPSchLinkVars.oSchLinkGraph.oDbSrcPortGraphPos.dbX = lstrHelper.at(0).toDouble();
			oPSchLinkBase.oPSchLinkVars.oSchLinkGraph.oDbSrcPortGraphPos.dbY = lstrHelper.at(1).toDouble();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListSrcPortPoss);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvElement <<
					m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogSrcPortPos << m_chLogNode);
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeLink, p_ListDstIDs,
						  m_chDstID, FCN_ONE_LEVEL, p_NodeDstID)
		{
			strHelper = QString(p_NodeDstID->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvLink << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogDstID << m_chLogNode);
				return false;
			}
			oPSchLinkBase.oPSchLinkVars.ullIDDst = strHelper.toULongLong();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListDstIDs);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvLink <<
					m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogDstID << m_chLogNode);
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeLink, p_ListDstPortIDs,
						  m_chDstPortID, FCN_ONE_LEVEL, p_NodeDstPortID)
		{
			strHelper = QString(p_NodeDstPortID->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvLink << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogDstPortID << m_chLogNode);
				return false;
			}
			oPSchLinkBase.oPSchLinkVars.ushiDstPort = strHelper.toUShort();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListDstPortIDs);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvLink <<
					m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogDstPortID << m_chLogNode);
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeLink, p_ListDstPortPoss,
						  m_chDstPortPos, FCN_ONE_LEVEL, p_NodeDstPortPos)
		{
			strHelper = QString(p_NodeDstPortPos->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvElement <<
						m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogDstPortPos << m_chLogNode);
				return false;
			}
			lstrHelper = strHelper.split(',');
			if(lstrHelper.count() != 2)
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvElement <<
						m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogDstPortPos << m_chLogNode);
				return false;
			}
			oPSchLinkBase.oPSchLinkVars.oSchLinkGraph.oDbDstPortGraphPos.dbX = lstrHelper.at(0).toDouble();
			oPSchLinkBase.oPSchLinkVars.oSchLinkGraph.oDbDstPortGraphPos.dbY = lstrHelper.at(1).toDouble();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListDstPortPoss);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvElement <<
					m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogDstPortPos << m_chLogNode);
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
	XMLError eResult;
	tinyxml2::XMLDocument xmlEnv;
	QString strHOne, strHTwo, strHThree, strHFour;
	int iR, iG, iB, iA;
	QColor oQColor;
	XMLNode* p_NodeRoot;
	XMLNode* p_NodeGroups;
	XMLNode* p_NodeGroup;
	XMLNode* p_NodeElements;
	XMLNode* p_NodeElement;
	XMLNode* p_NodeLinks;
	XMLNode* p_NodeLink;
	XMLNode* p_NodeID;
	XMLNode* p_NodeName;
	XMLNode* p_NodeBkgColor;
	XMLNode* p_NodeFrame;
	XMLNode* p_NodeZ;
	XMLNode* p_NodeGroupID;
	XMLNode* p_NodeSrcID;
	XMLNode* p_NodeSrcPortID;
	XMLNode* p_NodeSrcPortPos;
	XMLNode* p_NodeDstID;
	XMLNode* p_NodeDstPortID;
	XMLNode* p_NodeDstPortPos;
	//
	LOG_P_1(LOG_CAT_I, "Saving environment to: " << strEnvFilename.toStdString());
	xmlEnv.InsertEndChild(xmlEnv.NewDeclaration());
	p_NodeRoot = xmlEnv.InsertEndChild(xmlEnv.NewElement("Root"));
	p_NodeGroups = p_NodeRoot->InsertEndChild(xmlEnv.NewElement(m_chGroups));
	for(unsigned int iF = 0; iF != PBCount(Group); iF++)
	{
		p_NodeGroup = p_NodeGroups->InsertEndChild(xmlEnv.NewElement(m_chGroup));
		p_NodeID = p_NodeGroup->InsertEndChild(xmlEnv.NewElement(m_chID));
		p_NodeID->ToElement()->
				SetText(strHOne.setNum(PBAccess(Group,iF)->oPSchGroupBase.oPSchGroupVars.ullIDInt).toStdString().c_str());
		p_NodeName = p_NodeGroup->InsertEndChild(xmlEnv.NewElement(m_chName));
		p_NodeName->ToElement()->
				SetText(PBAccess(Group,iF)->oPSchGroupBase.m_chName);
		p_NodeBkgColor = p_NodeGroup->InsertEndChild(xmlEnv.NewElement(m_chBkgColor));
		oQColor = QColor();
		oQColor.setRgba(PBAccess(Group,iF)->oPSchGroupBase.oPSchGroupVars.oSchGroupGraph.uiObjectBkgColor);
		oQColor.getRgb(&iR, &iG, &iB, &iA);
		p_NodeBkgColor->ToElement()->
				SetText((strHOne.setNum(iR) + "," + strHTwo.setNum(iG) + "," + strHThree.setNum(iB) + "," + strHFour.setNum(iA)).toStdString().c_str());
		p_NodeFrame = p_NodeGroup->InsertEndChild(xmlEnv.NewElement(m_chFrame));
		p_NodeFrame->ToElement()->
				SetText((strHOne.setNum(PBAccess(Group,iF)->oPSchGroupBase.oPSchGroupVars.oSchGroupGraph.oDbObjectFrame.dbX) + "," +
						 strHTwo.setNum(PBAccess(Group,iF)->oPSchGroupBase.oPSchGroupVars.oSchGroupGraph.oDbObjectFrame.dbY) + "," +
						 strHThree.setNum(PBAccess(Group,iF)->oPSchGroupBase.oPSchGroupVars.oSchGroupGraph.oDbObjectFrame.dbW) + "," +
						 strHFour.setNum(PBAccess(Group,iF)->oPSchGroupBase.oPSchGroupVars.oSchGroupGraph.oDbObjectFrame.dbH)).
						toStdString().c_str());
		p_NodeZ = p_NodeGroup->InsertEndChild(xmlEnv.NewElement(m_chZ));
		p_NodeZ->ToElement()->
				SetText(strHOne.setNum(PBAccess(Group,iF)->oPSchGroupBase.oPSchGroupVars.oSchGroupGraph.dbObjectZPos).
						toStdString().c_str());
		if(PBAccess(Group,iF)->oPSchGroupBase.oPSchGroupVars.ullIDGroup != 0)
		{
			p_NodeGroupID = p_NodeGroup->InsertEndChild(xmlEnv.NewElement(m_chGroupID));
			p_NodeGroupID->ToElement()->
					SetText(strHOne.setNum(PBAccess(Group,iF)->oPSchGroupBase.oPSchGroupVars.ullIDGroup).toStdString().c_str());
		}
	}
	p_NodeElements = p_NodeRoot->InsertEndChild(xmlEnv.NewElement(m_chElements));
	for(unsigned int iF = 0; iF != PBCount(Element); iF++)
	{
		p_NodeElement = p_NodeElements->InsertEndChild(xmlEnv.NewElement(m_chElement));
		p_NodeID = p_NodeElement->InsertEndChild(xmlEnv.NewElement(m_chID));
		p_NodeID->ToElement()->
				SetText(strHOne.setNum(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.ullIDInt).toStdString().c_str());
		p_NodeName = p_NodeElement->InsertEndChild(xmlEnv.NewElement(m_chName));
		p_NodeName->ToElement()->
				SetText(PBAccess(Element,iF)->oPSchElementBase.m_chName);
		p_NodeBkgColor = p_NodeElement->InsertEndChild(xmlEnv.NewElement(m_chBkgColor));
		oQColor = QColor();
		oQColor.setRgba(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.oSchElementGraph.uiObjectBkgColor);
		oQColor.getRgb(&iR, &iG, &iB, &iA);
		p_NodeBkgColor->ToElement()->
				SetText((strHOne.setNum(iR) + "," + strHTwo.setNum(iG) + "," + strHThree.setNum(iB) + "," + strHFour.setNum(iA)).toStdString().c_str());
		p_NodeFrame = p_NodeElement->InsertEndChild(xmlEnv.NewElement(m_chFrame));
		p_NodeFrame->ToElement()->
				SetText((strHOne.setNum(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.
										oSchElementGraph.oDbObjectFrame.dbX) + "," +
						 strHTwo.setNum(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.
										oSchElementGraph.oDbObjectFrame.dbY) + "," +
						 strHThree.setNum(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.
										  oSchElementGraph.oDbObjectFrame.dbW) + "," +
						 strHFour.setNum(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.
										 oSchElementGraph.oDbObjectFrame.dbH)).
						toStdString().c_str());
		p_NodeZ = p_NodeElement->InsertEndChild(xmlEnv.NewElement(m_chZ));
		p_NodeZ->ToElement()->
				SetText(strHOne.setNum(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.oSchElementGraph.dbObjectZPos).
						toStdString().c_str());
		if(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.ullIDGroup != 0)
		{
			p_NodeGroupID = p_NodeElement->InsertEndChild(xmlEnv.NewElement(m_chGroupID));
			p_NodeGroupID->ToElement()->
					SetText(strHOne.setNum(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.ullIDGroup).toStdString().c_str());
		}
	}
	p_NodeLinks = p_NodeRoot->InsertEndChild(xmlEnv.NewElement(m_chLinks));
	for(unsigned int iF = 0; iF != PBCount(Link); iF++)
	{
		p_NodeLink = p_NodeLinks->InsertEndChild(xmlEnv.NewElement(m_chLink));
		p_NodeSrcID = p_NodeLink->InsertEndChild(xmlEnv.NewElement(m_chSrcID));
		p_NodeSrcID->ToElement()->
				SetText(strHOne.setNum(PBAccess(Link,iF)->oPSchLinkBase.oPSchLinkVars.ullIDSrc).toStdString().c_str());
		p_NodeSrcPortID = p_NodeLink->InsertEndChild(xmlEnv.NewElement(m_chSrcPortID));
		p_NodeSrcPortID->ToElement()->
				SetText(strHOne.setNum(PBAccess(Link,iF)->oPSchLinkBase.oPSchLinkVars.ushiSrcPort).toStdString().c_str());
		p_NodeSrcPortPos = p_NodeLink->InsertEndChild(xmlEnv.NewElement(m_chSrcPortPos));
		p_NodeSrcPortPos->ToElement()->
				SetText((strHOne.setNum(PBAccess(Link,iF)->oPSchLinkBase.oPSchLinkVars.oSchLinkGraph.oDbSrcPortGraphPos.dbX) + "," +
						 strHTwo.setNum(PBAccess(Link,iF)->oPSchLinkBase.oPSchLinkVars.oSchLinkGraph.oDbSrcPortGraphPos.dbY)).
						toStdString().c_str());
		p_NodeDstID = p_NodeLink->InsertEndChild(xmlEnv.NewElement(m_chDstID));
		p_NodeDstID->ToElement()->
				SetText(strHOne.setNum(PBAccess(Link,iF)->oPSchLinkBase.oPSchLinkVars.ullIDDst).toStdString().c_str());
		p_NodeDstPortID = p_NodeLink->InsertEndChild(xmlEnv.NewElement(m_chDstPortID));
		p_NodeDstPortID->ToElement()->
				SetText(strHOne.setNum(PBAccess(Link,iF)->oPSchLinkBase.oPSchLinkVars.ushiDstPort).toStdString().c_str());
		p_NodeDstPortPos = p_NodeLink->InsertEndChild(xmlEnv.NewElement(m_chDstPortPos));
		p_NodeDstPortPos->ToElement()->
				SetText((strHOne.setNum(PBAccess(Link,iF)->oPSchLinkBase.oPSchLinkVars.oSchLinkGraph.oDbDstPortGraphPos.dbX) + "," +
						 strHTwo.setNum(PBAccess(Link,iF)->oPSchLinkBase.oPSchLinkVars.oSchLinkGraph.oDbDstPortGraphPos.dbY)).
						toStdString().c_str());
	}
	eResult = xmlEnv.SaveFile(strEnvPath.toStdString().c_str());
	if (eResult != XML_SUCCESS)
	{
		LOG_P_0(LOG_CAT_E, "Can`t save environment.");
		return false;
	}
	LOG_P_1(LOG_CAT_I, "Environment has been saved.");
	return true;
}

// Запуск среды.
bool Environment::Start()
{
	if(!bEnvLoaded)
	{
		if(!LoadEnv()) return false;
	}
	LOG_P_1(LOG_CAT_I, "Start environment.");
	bRequested = false;
	memset(&oPSchReadyFrame, 0, sizeof(PSchReadyFrame));
	pthread_create(&thrEnv, nullptr, EnvThread, nullptr);
	return true;
}

// Установка всех флагов всех объектов сцены на новые для клиента.
void Environment::SetAllNew()
{
	for(unsigned int uiF = 0; uiF < PBCount(Element); uiF++)
	{
		PBAccess(Element, uiF)->bNew = true;
	}
	for(unsigned int uiF = 0; uiF < PBCount(Group); uiF++)
	{
		PBAccess(Group, uiF)->bNew = true;
	}
	for(unsigned int uiF = 0; uiF < PBCount(Link); uiF++)
	{
		PBAccess(Link, uiF)->bNew = true;
	}
}

// Остановка среды.
void Environment::Stop()
{
	LOG_P_1(LOG_CAT_I, "Stop environment.");
	bStopEnvUpdate = true;
	while(bStopEnvUpdate)
	{
		MSleep(USER_RESPONSE_MS);
	}
	SetAllNew();
}

// Проверка инициализированности среды.
bool Environment::CheckInitialized()
{
	return bEnvLoaded;
}

// Поток шагов среды.
void* Environment::EnvThread(void *p_vPlug)
{
	p_vPlug = p_vPlug;
	bEnvThreadAlive = true;
	LOG_P_1(LOG_CAT_I, "Environment thread alive.");
	while(!bStopEnvUpdate)
	{
		if(MainWindow::p_Server)
		{
			if(MainWindow::p_Server->CheckServerAlive())
			{
				NetOperations();
			}
		}
		MSleep(ENV_STEP_WAITING);
	}
	bEnvThreadAlive = false;
	LOG_P_1(LOG_CAT_I, "Environment thread terminated.");
	bStopEnvUpdate = false;
	RETURN_THREAD
}

// Работа с сетью.
void Environment::NetOperations()
{
	Element* p_Element;
	Link* p_Link;
	Group* p_Group;
	PSchElementVars oPSchElementVars;
	PSchElementName oPSchElementName;
	PSchGroupName oPSchGroupName;
	PSchLinkVars oPSchLinkVars;
	PSchGroupVars oPSchGroupVars;
	QList<Element*> lp_NewElements;
	QList<Element*> lp_ChangedElements;
	QList<Element*> lp_RenamedElements;
	QList<Link*> lp_NewLinks;
	QList<Link*> lp_ChangedLinks;
	QList<Group*> lp_NewGroups;
	QList<Group*> lp_ChangedGroups;
	QList<Group*> lp_RenamedGroups;
	//
	memset(&oPSchElementVars, 0, sizeof(oPSchElementVars));
	memset(&oPSchLinkVars, 0, sizeof(oPSchLinkVars));
	bool bPresent;
	int iEC, iECM;
	int iLC, iLCM;
	unsigned short ushNewsQantity = 1;
	//
	if(MainWindow::p_Server->GetConnectionData(0).iStatus != NO_CONNECTION)
	{
		if(MainWindow::p_Server->IsConnectionSecured(0))
		{
			iEC = PBCount(Element);
			iLC = PBCount(Link);
			if(bRequested) // Если был запрос от клиента...
			{
				//==================== Раздел элементов.
				for(int iE = 0; iE < iEC; iE++) // По всем элементам...
				{
					p_Element = PBAccess(Element, iE);
					if(ushNewsQantity)
					{
						if(p_Element->bNew) // Проверка признака нового элемента для клиента.
						{
							lp_NewElements.push_back(p_Element);
							p_Element->bNew = false; // Уже не новый для клиента.
							bRequested = false;
							ushNewsQantity--;
						}
					}
					if(p_Element->chTouchedBits & TOUCHED_GEOMETRY)
						// Если была затронута геометрия...
					{
						for(int iL = 0; iL < p_Element->vp_LinkedElements.count(); iL++)
						{
							Element* p_DstElement = p_Element->vp_LinkedElements.at(iL);
							// Проверка признака затронутого эл. для клиента.
							if(p_DstElement->chTouchedBits & TOUCHED_GEOMETRY)
							{
								lp_ChangedElements.push_back(p_DstElement);
								p_DstElement->chTouchedBits ^= TOUCHED_GEOMETRY;
							}
						}
						lp_ChangedElements.push_back(p_Element);
						p_Element->chTouchedBits ^= TOUCHED_GEOMETRY;
						bRequested = false;
					} // else if - так как геометрия в приоритете.
					else if(((p_Element->chTouchedBits & TOUCHED_CONTENT) ||
							 (p_Element->chTouchedBits & TOUCHED_NAME)) &&
							(p_Element->oPSchElementBase.oPSchElementVars.oSchElementGraph.oDbObjectFrame.dbX >
							 oPSchReadyFrame.oDbFrame.dbX) &
							(p_Element->oPSchElementBase.oPSchElementVars.oSchElementGraph.oDbObjectFrame.dbY >
							 oPSchReadyFrame.oDbFrame.dbY) &
							(p_Element->oPSchElementBase.oPSchElementVars.oSchElementGraph.oDbObjectFrame.dbX <
							 (oPSchReadyFrame.oDbFrame.dbX +
							  oPSchReadyFrame.oDbFrame.dbW)) &
							(p_Element->oPSchElementBase.oPSchElementVars.oSchElementGraph.oDbObjectFrame.dbY <
							 (oPSchReadyFrame.oDbFrame.dbY +
							  oPSchReadyFrame.oDbFrame.dbH)))
						// Если был затронут контент или имя и входит в окно...
					{
						if(p_Element->chTouchedBits & TOUCHED_CONTENT)
						{
							lp_ChangedElements.push_back(p_Element);
							p_Element->chTouchedBits ^= TOUCHED_CONTENT;
						}
						if(p_Element->chTouchedBits & TOUCHED_NAME)
						{
							lp_RenamedElements.push_back(p_Element);
							p_Element->chTouchedBits ^= TOUCHED_NAME;
						}
						bRequested = false;
					}
				}
				bPresent = false;
				// По новым.
				iEC = lp_NewElements.count();
				iECM = iEC - 1;
				for(int iEP = 0; iEP < iEC; iEP++)
				{
					p_Element = lp_NewElements.at(iEP);
					if(iEP != iECM)
					{
						p_Element->oPSchElementBase.oPSchElementVars.bLastInQueue = false; // Не последний в цепочке.
					}
					else
					{
						p_Element->oPSchElementBase.oPSchElementVars.bLastInQueue = true; // Последний в цепочке.
					}
					if(p_Element->chTouchedBits & TOUCHED_GROUP)
					{
						p_Element->oPSchElementBase.bRequestGroupUpdate = true;
						p_Element->chTouchedBits ^= TOUCHED_GROUP;
					}
					else
					{
						p_Element->oPSchElementBase.bRequestGroupUpdate = false;
					}
					// Отправка полных данных на соединение из запроса.
					LCHECK_BOOL(MainWindow::p_Server->AddPocketToOutputBuffer(0,
																  PROTO_O_SCH_ELEMENT_BASE,
																  (char*)&p_Element->oPSchElementBase,
																  sizeof(PSchElementBase)));
					LOG_P_2(LOG_CAT_I, "{Out} New element [" << QString(p_Element->oPSchElementBase.m_chName).toStdString()
							<< "] has been sent to client.");
					bPresent = true; // Хоть один есть.
				}
				// По изменённым.
				iEC = lp_ChangedElements.count();
				iECM = iEC - 1;
				for(int iEP = 0; iEP < iEC; iEP++)
				{
					p_Element = lp_ChangedElements.at(iEP);
					memcpy(&oPSchElementVars, &p_Element->oPSchElementBase.oPSchElementVars, sizeof(PSchElementVars));
					if(iEP != iECM)
					{
						oPSchElementVars.bLastInQueue = false; // Не последний в цепочке.
					}
					else
					{
						oPSchElementVars.bLastInQueue = true; // Последний в цепочке.
					}
					// Отправка изменений на соединение из запроса.
					LCHECK_BOOL(MainWindow::p_Server->AddPocketToOutputBuffer(0, PROTO_O_SCH_ELEMENT_VARS,
																  (char*)&oPSchElementVars, sizeof(PSchElementVars)));
					LOG_P_2(LOG_CAT_I, "{Out} Changed element [" << QString(p_Element->oPSchElementBase.m_chName).toStdString()
							<< "] has been sent to client.");
					bPresent = true; // Хоть один есть.
				}
				// По переименованным.
				iEC = lp_RenamedElements.count();
				iECM = iEC - 1;
				for(int iEP = 0; iEP < iEC; iEP++)
				{
					p_Element = lp_RenamedElements.at(iEP);
					memcpy(&oPSchElementName.m_chName, &p_Element->oPSchElementBase.m_chName, SCH_OBJ_NAME_STR_LEN);
					oPSchElementName.ullIDInt = p_Element->oPSchElementBase.oPSchElementVars.ullIDInt;
					if(iEP != iECM)
					{
						oPSchElementName.bLastInQueue = false; // Не последний в цепочке.
					}
					else
					{
						oPSchElementName.bLastInQueue = true; // Последний в цепочке.
					}
					// Отправка изменений на соединение из запроса.
					LCHECK_BOOL(MainWindow::p_Server->AddPocketToOutputBuffer(0, PROTO_O_SCH_ELEMENT_NAME,
																  (char*)&oPSchElementName, sizeof(oPSchElementName)));
					LOG_P_2(LOG_CAT_I, "{Out} Changed element`s name [" << QString(p_Element->oPSchElementBase.m_chName).toStdString()
							<< "] has been sent to client.");
					bPresent = true; // Хоть один есть.
				}
				lp_NewElements.clear();
				lp_ChangedElements.clear();
				lp_RenamedElements.clear();
				//==================== Раздел линков.
				for(int iL = 0; iL < iLC; iL++) // По всем линкам...
				{
					p_Link = PBAccess(Link, iL);
					if(ushNewsQantity)
					{
						if(p_Link->bNew) // Проверка признака нового линка для клиента.
						{
							if(CheckLinkForAct(p_Link)) // Проверка на догруженность элементов линка на клиент.
							{
								lp_NewLinks.push_back(p_Link);
								p_Link->bNew = false; // Уже не новый для клиента.
								bRequested = false;
								ushNewsQantity--;
							}
						}
					}
					if(p_Link->chTouchedBits & TOUCHED_GEOMETRY) // Проверка признака затр/ линка для клиента.
					{
						if(CheckLinkForAct(p_Link)) // Проверка на догруженность элементов линка на клиент.
						{
							lp_ChangedLinks.push_back(p_Link);
							p_Link->chTouchedBits = 0; // Уже не затронутый для клиента.
							bRequested = false;
						}
					}
				}
				// По новым.
				iLC = lp_NewLinks.count();
				iLCM = iLC - 1;
				for(int iLP = 0; iLP < iLC; iLP++)
				{
					p_Link = lp_NewLinks.at(iLP);
					if(iLP != iLCM)
					{
						p_Link->oPSchLinkBase.oPSchLinkVars.bLastInQueue = false; // Не последний в цепочке.
					}
					else
					{
						p_Link->oPSchLinkBase.oPSchLinkVars.bLastInQueue = true; // Последний в цепочке.
					}
					// Отправка полных данных на соединение из запроса.
					LCHECK_BOOL(MainWindow::p_Server->AddPocketToOutputBuffer(0, PROTO_O_SCH_LINK_BASE,
																  (char*)&p_Link->oPSchLinkBase, sizeof(PSchLinkBase)));
					LOG_P_2(LOG_CAT_I, "{Out} New link [" << QString(p_Link->p_SrcElement->oPSchElementBase.m_chName).toStdString()
							<< "<>" << QString(p_Link->p_DstElement->oPSchElementBase.m_chName).toStdString()
							<< "] has been sent to client.");
					bPresent = true; // Хоть один есть.
				}
				// По изменённым.
				iLC = lp_ChangedLinks.count();
				iLCM = iLC - 1;
				for(int iLP = 0; iLP < iLC; iLP++)
				{
					p_Link = lp_ChangedLinks.at(iLP);
					memcpy(&oPSchLinkVars, &p_Link->oPSchLinkBase.oPSchLinkVars, sizeof(oPSchLinkVars));
					if(iLP != iLCM)
					{
						oPSchLinkVars.bLastInQueue = false; // Не последний в цепочке.
					}
					else
					{
						oPSchLinkVars.bLastInQueue = true; // Последний в цепочке.
					}
					// Отправка изменений на соединение из запроса.
					LCHECK_BOOL(MainWindow::p_Server->AddPocketToOutputBuffer(0, PROTO_O_SCH_LINK_VARS,
																  (char*)&oPSchLinkVars, sizeof(PSchLinkVars)));
					LOG_P_2(LOG_CAT_I, "{Out} Changed link [" << QString(p_Link->p_SrcElement->oPSchElementBase.m_chName).toStdString()
							<< "<>" << QString(p_Link->p_DstElement->oPSchElementBase.m_chName).toStdString()
							<< "] has been sent to client.");
					bPresent = true; // Хоть один есть.
				}
				lp_NewLinks.clear();
				lp_ChangedLinks.clear();
				//==================== Раздел групп.
				for(uint uiE = 0; uiE < PBCount(Group); uiE++) // По всем группам...
				{
					p_Group = PBAccess(Group, uiE);
					if(ushNewsQantity)
					{
						if(p_Group->bNew) // Проверка признака новой группы для клиента.
						{
							lp_NewGroups.push_back(p_Group);
							p_Group->bNew = false; // Уже не новая для клиента.
							bRequested = false;
							ushNewsQantity--;
						}
					}
					if(p_Group->chTouchedBits & TOUCHED_GEOMETRY)
						// Если была затронута геометрия...
					{
						lp_ChangedGroups.push_back(p_Group);
						p_Group->chTouchedBits = 0; // Уже не затронутая для клиента.
						bRequested = false;
					} // else if - так как геометрия в приоритете.
					else if(((p_Group->chTouchedBits & TOUCHED_CONTENT) ||
							 (p_Group->chTouchedBits & TOUCHED_NAME)) &&
							(p_Group->oPSchGroupBase.oPSchGroupVars.oSchGroupGraph.oDbObjectFrame.dbX >
							 oPSchReadyFrame.oDbFrame.dbX) &
							(p_Group->oPSchGroupBase.oPSchGroupVars.oSchGroupGraph.oDbObjectFrame.dbY >
							 oPSchReadyFrame.oDbFrame.dbY) &
							(p_Group->oPSchGroupBase.oPSchGroupVars.oSchGroupGraph.oDbObjectFrame.dbX <
							 (oPSchReadyFrame.oDbFrame.dbX +
							  oPSchReadyFrame.oDbFrame.dbW)) &
							(p_Group->oPSchGroupBase.oPSchGroupVars.oSchGroupGraph.oDbObjectFrame.dbY <
							 (oPSchReadyFrame.oDbFrame.dbY +
							  oPSchReadyFrame.oDbFrame.dbH)))
						// Если был затронут контент или имя и входит в окно...
					{
						if(p_Group->chTouchedBits & TOUCHED_CONTENT)
						{
							lp_ChangedGroups.push_back(p_Group);
						}
						if(p_Group->chTouchedBits & TOUCHED_NAME)
						{
							lp_RenamedGroups.push_back(p_Group);
						}
						bRequested = false;
						p_Group->chTouchedBits = 0; // Уже не затронутая для клиента.
					}
				}
				// По новым.
				iEC = lp_NewGroups.count();
				iECM = iEC - 1;
				for(int iEP = 0; iEP < iEC; iEP++)
				{
					p_Group = lp_NewGroups.at(iEP);
					if(iEP != iECM)
					{
						p_Group->oPSchGroupBase.oPSchGroupVars.bLastInQueue = false; // Не последний в цепочке.
					}
					else
					{
						p_Group->oPSchGroupBase.oPSchGroupVars.bLastInQueue = true; // Последний в цепочке.
					}
					// Отправка полных данных на соединение из запроса.
					LCHECK_BOOL(MainWindow::p_Server->AddPocketToOutputBuffer(0, PROTO_O_SCH_GROUP_BASE,
																  (char*)&p_Group->oPSchGroupBase, sizeof(PSchGroupBase)));
					LOG_P_2(LOG_CAT_I, "{Out} New group [" << QString(p_Group->oPSchGroupBase.m_chName).toStdString()
							<< "] has been sent to client.");
					bPresent = true; // Хоть один есть.
				}
				// По изменённым.
				iEC = lp_ChangedGroups.count();
				iECM = iEC - 1;
				for(int iEP = 0; iEP < iEC; iEP++)
				{
					p_Group = lp_ChangedGroups.at(iEP);
					memcpy(&oPSchGroupVars, &p_Group->oPSchGroupBase.oPSchGroupVars, sizeof(PSchGroupVars));
					if(iEP != iECM)
					{
						oPSchGroupVars.bLastInQueue = false; // Не последний в цепочке.
					}
					else
					{
						oPSchGroupVars.bLastInQueue = true; // Последний в цепочке.
					}
					// Отправка изменений на соединение из запроса.
					LCHECK_BOOL(MainWindow::p_Server->AddPocketToOutputBuffer(0, PROTO_O_SCH_GROUP_VARS,
																  (char*)&oPSchGroupVars, sizeof(PSchGroupVars)));
					LOG_P_2(LOG_CAT_I, "{Out} Changed group [" << QString(p_Group->oPSchGroupBase.m_chName).toStdString()
							<< "] has been sent to client.");
					bPresent = true; // Хоть один есть.
				}
				// По переименованным.
				iEC = lp_RenamedGroups.count();
				iECM = iEC - 1;
				for(int iEP = 0; iEP < iEC; iEP++)
				{
					p_Group = lp_RenamedGroups.at(iEP);
					memcpy(&oPSchGroupName.m_chName, &p_Group->oPSchGroupBase.m_chName, SCH_OBJ_NAME_STR_LEN);
					oPSchGroupName.ullIDInt = p_Group->oPSchGroupBase.oPSchGroupVars.ullIDInt;
					if(iEP != iECM)
					{
						oPSchGroupName.bLastInQueue = false; // Не последний в цепочке.
					}
					else
					{
						oPSchGroupName.bLastInQueue = true; // Последний в цепочке.
					}
					// Отправка изменений на соединение из запроса.
					LCHECK_BOOL(MainWindow::p_Server->AddPocketToOutputBuffer(0, PROTO_O_SCH_GROUP_NAME,
																  (char*)&oPSchGroupName, sizeof(oPSchGroupName)));
					LOG_P_2(LOG_CAT_I, "{Out} Changed group`s name [" << QString(p_Group->oPSchGroupBase.m_chName).toStdString()
							<< "] has been sent to client.");
					bPresent = true; // Хоть один есть.
				}
				lp_NewGroups.clear();
				lp_ChangedGroups.clear();
				lp_RenamedGroups.clear();
				//
				if(bPresent)
				{
					LCHECK_BOOL(MainWindow::p_Server->SendBufferToClient(0, true));
				}
			}
		}
	}
}

// Проверка линка на актуальность по представленным элементам.
bool Environment::CheckLinkForAct(Link* p_Link)
{
	Element* p_Element;
	//
	for(uint uiE = 0; uiE < PBCount(Element); uiE++)
	{
		p_Element = PBAccess(Element, uiE);
		if(p_Element->oPSchElementBase.oPSchElementVars.ullIDInt == p_Link->oPSchLinkBase.oPSchLinkVars.ullIDSrc)
		{
			if(p_Element->bNew) return false;
		}
		if(p_Element->oPSchElementBase.oPSchElementVars.ullIDInt == p_Link->oPSchLinkBase.oPSchLinkVars.ullIDDst)
		{
			if(p_Element->bNew) return false;
		}
	}
	return true;
}

// Удаление линка в позиции и обнуление указателя на него.
void Environment::EraseLinkAt(int iPos)
{
	Link* p_Link = PBAccess(Link, iPos);
	p_Link->p_SrcElement->vp_LinkedElements.removeOne(p_Link->p_DstElement);
	p_Link->p_DstElement->vp_LinkedElements.removeOne(p_Link->p_SrcElement);
	RemoveObjectFromPBByPos(Link, iPos);
}

// Удаление линков для элемента.
void Environment::EraseLinksForElement(Element* p_Element)
{
	for(int iL = 0; iL < (int)PBCount(Link); iL++)
	{
		bool bPresent = false;
		//
		if(PBAccess(Link, iL)->p_DstElement == p_Element)
		{
			PBAccess(Link, iL)->p_SrcElement->vp_LinkedElements.removeOne(p_Element);
			bPresent = true;
		}
		else if(PBAccess(Link, iL)->p_SrcElement == p_Element)
		{
			PBAccess(Link, iL)->p_DstElement->vp_LinkedElements.removeOne(p_Element);
			bPresent = true;
		}
		if(bPresent)
		{
			RemoveObjectFromPBByPos(Link, iL);
			iL--;
		}
	}
}

// Удаление элемента в позиции и обнуление указателя на него.
void Environment::EraseElementAt(int iPos)
{
	Element* p_Element = PBAccess(Element, iPos);
	// Работа с группой.
	if(p_Element->oPSchElementBase.oPSchElementVars.ullIDGroup != 0)
	{
		Group* p_Group = p_Element->p_Group;
		//
		if(p_Group != nullptr)
		{
			if(!p_Group->vp_ConnectedElements.isEmpty())
			{
				p_Group->vp_ConnectedElements.removeOne(p_Element);
				if(p_Group->vp_ConnectedElements.isEmpty())
				{
					RemoveObjectFromPBByPointer(Group, p_Group);
				}
			}
		}
	}
	EraseLinksForElement(p_Element);
	RemoveObjectFromPBByPos(Element, iPos);
}

// Удаление элементов из группы.
void Environment::EraseElementsFromGroup(Group* p_Group)
{
	while(!p_Group->vp_ConnectedElements.isEmpty())
	{
		Element* p_Element = p_Group->vp_ConnectedElements.at(0);
		//
		p_Group->vp_ConnectedElements.removeOne(p_Element);
		EraseLinksForElement(p_Element);
		RemoveObjectFromPBByPointer(Element, p_Element);
	}
}

// Удаление групп из группы.
void Environment::EraseGroupsFromGroup(Group* p_Group)
{
	while(!p_Group->vp_ConnectedGroups.isEmpty())
	{
		Group* p_GroupInt = p_Group->vp_ConnectedGroups.at(0);
		//
		p_Group->vp_ConnectedGroups.removeOne(p_GroupInt);
		EraseGroup(p_GroupInt); // Рекурсия.
	}
}

// Удаление группы в позиции и обнуление указателя на неё.
void Environment::EraseGroupAt(int iPos)
{
	EraseGroup(PBAccess(Group, iPos));
}

// Удаление группы по указателю.
void Environment::EraseGroup(Group* p_Group)
{
	EraseElementsFromGroup(p_Group);
	EraseGroupsFromGroup(p_Group);
	// Удаление группы.
	RemoveObjectFromPBByPointer(Group, p_Group);
}
