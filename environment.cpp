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
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvGroup << m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogName << m_chLogNode);
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
			if(lstrHelper.count() != 3)
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvGroup << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogBkgColor << m_chLogNode);
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
			if(lstrHelper.count() != 3)
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogBkgColor << m_chLogNode);
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
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogZ << m_chLogNode);
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeElement, p_ListPoss,
						  m_chPos, FCN_ONE_LEVEL, p_NodePos)
		{
			strHelper = QString(p_NodePos->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogPos << m_chLogNode);
				return false;
			}
			lstrHelper = strHelper.split(',');
			if(lstrHelper.count() != 2)
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogPos << m_chLogNode);
				return false;
			}
			oPSchElementBase.oPSchElementVars.oSchElementGraph.oDbObjectPos.dbX = lstrHelper.at(0).toDouble();
			oPSchElementBase.oPSchElementVars.oSchElementGraph.oDbObjectPos.dbY = lstrHelper.at(1).toDouble();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListPoss);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvElement <<
					m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogPos << m_chLogNode);
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
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvLink << m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogSrcID << m_chLogNode);
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
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvLink << m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogDstID << m_chLogNode);
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
						m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogDstPortPos << m_chLogNode);
				return false;
			}
			lstrHelper = strHelper.split(',');
			if(lstrHelper.count() != 2)
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect << m_chLogWrong << m_chLogDstPortPos << m_chLogNode);
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
	int iR, iG, iB;
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
	XMLNode* p_NodePos;
	XMLNode* p_NodeGroupID;
	XMLNode* p_NodeSrcID;
	XMLNode* p_NodeSrcPortID;
	XMLNode* p_NodeSrcPortPos;
	XMLNode* p_NodeDstID;
	XMLNode* p_NodeDstPortID;
	XMLNode* p_NodeDstPortPos;
	//
	LOG_P_0(LOG_CAT_I, "Saving environment to: " << strEnvFilename.toStdString());
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
		oQColor = QColor(PBAccess(Group,iF)->oPSchGroupBase.oPSchGroupVars.oSchGroupGraph.uiObjectBkgColor);
		oQColor.getRgb(&iR, &iG, &iB);
		p_NodeBkgColor->ToElement()->
				SetText((strHOne.setNum(iR) + "," + strHTwo.setNum(iG) + "," + strHThree.setNum(iB)).toStdString().c_str());
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
		oQColor = QColor(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.oSchElementGraph.uiObjectBkgColor);
		oQColor.getRgb(&iR, &iG, &iB);
		p_NodeBkgColor->ToElement()->
				SetText((strHOne.setNum(iR) + "," + strHTwo.setNum(iG) + "," + strHThree.setNum(iB)).toStdString().c_str());
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
		p_NodePos = p_NodeElement->InsertEndChild(xmlEnv.NewElement(m_chPos));
		p_NodePos->ToElement()->
				SetText((strHOne.setNum(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.
										oSchElementGraph.oDbObjectPos.dbX) + "," +
						 strHTwo.setNum(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.
										oSchElementGraph.oDbObjectPos.dbY)).
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
