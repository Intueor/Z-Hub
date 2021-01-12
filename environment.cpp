//== ВКЛЮЧЕНИЯ.
#include <QMap>
#include <QColor>
#include "environment.h"
#include "main-window.h"

//== МАКРОСЫ.
#define LOG_NAME										"environment"
#define LOG_DIR_PATH									"../Z-Hub/logs/"
#define _SQ_Util(Type,Struct)							oQueueSegment.uchType = uiCurrentSegNumber;								\
														uiCurrentSegNumber++;													\
														oQueueSegment.uchType = Type;											\
														oQueueSegment.p_vUnitObject = (void*)(new Struct);						\
														oQueueSegment.bDirectionOut = bDirectionOut;							\
														*static_cast<Struct*>(oQueueSegment.p_vUnitObject) = a##Struct;			\
														l_Queue.append(oQueueSegment);
#define	DRNodeMinimize									p_NodeMinimize = p_NodeMinimize
//== ДЕКЛАРАЦИИ СТАТИЧЕСКИХ ПЕРЕМЕННЫХ.
// Основное.
LOGDECL_INIT_INCLASS(Environment)
LOGDECL_INIT_PTHRD_INCLASS_EXT_ADD(Environment)
// Буферы.
StaticPBSourceInit(Element,, Environment, MAX_ELEMENTS)
StaticPBSourceInit(Link,, Environment, MAX_LINKS)
StaticPBSourceInit(Group,, Environment, MAX_GROUPS)
StaticPBSourceInit(Pseudonym,, Environment, MAX_PSEUDONYMS)
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
QList<Environment::EventsQueue::QueueSegment> Environment::EventsQueue::l_Queue;
Environment::EventsQueue::QueueSegment Environment::EventsQueue::oQueueSegment;
Environment::EventsQueue* Environment::p_EventsQueue = nullptr;
pthread_mutex_t Environment::ptQueueMutex = PTHREAD_MUTEX_INITIALIZER;
unsigned int Environment::EventsQueue::uiCurrentSegNumber;
int Environment::iLastFetchingSegNumber = 0;
//== ФУНКЦИИ КЛАССОВ.
//== Класс очереди событий. Методы вызывать только из-под мьютекса.
// Конструктор.
Environment::EventsQueue::EventsQueue()
{
	uiCurrentSegNumber = 0;
}
// Деструктор.
Environment::EventsQueue::~EventsQueue()
{
	Clear();
}
/////////////////////////////////// ЭЛЕМЕНТ ////////////////////////////////////
// Добавление нового элемента.
void Environment::EventsQueue::AddNewElement(PSchElementBase& aPSchElementBase, bool bDirectionOut)
{
	_SQ_Util(QUEUE_NEW_ELEMENT, PSchElementBase);
}
// Добавление изменений элемента.
void Environment::EventsQueue::AddElementChanges(PSchElementVars& aPSchElementVars, bool bDirectionOut)
{
	_SQ_Util(QUEUE_CHANGED_ELEMENT, PSchElementVars);
}
// Добавление изменения имени элемента и очистка аналогов в очереди.
void Environment::EventsQueue::AddElementRenameAndFlush(PSchElementName& aPSchElementName, bool bDirectionOut)
{
	// Удаление всех предыдущих переименований в цепочке (ни на что не отразится).
	int iQ = l_Queue.count();
	int iN = 0;
	//
gF:	while(iN < iQ)
	{
		const QueueSegment* pc_QueueSegmentStored = &l_Queue.at(iN);
		//
		if(pc_QueueSegmentStored->uchType == QUEUE_RENAMED_ELEMENT)
		{
			PSchElementName* p_PSchElementNameStored = static_cast<PSchElementName*>(pc_QueueSegmentStored->p_vUnitObject);
			//
			if(p_PSchElementNameStored->ullIDInt == aPSchElementName.ullIDInt)
			{
				l_Queue.removeAt(iN);
				iQ--;
				goto gF;
			}
		}
		iN++;
	}
	_SQ_Util(QUEUE_RENAMED_ELEMENT, PSchElementName);
}
// Добавление изменения цвета элемента и очистка аналогов в очереди.
void Environment::EventsQueue::AddElementColorAndFlush(PSchElementColor& aPSchElementColor, bool bDirectionOut)
{
	// Удаление всех предыдущих перекрасов в цепочке (ни на что не отразится).
	int iQ = l_Queue.count();
	int iN = 0;
	//
gF:	while(iN < iQ)
	{
		const QueueSegment* pc_QueueSegmentStored = &l_Queue.at(iN);
		//
		if(pc_QueueSegmentStored->uchType == QUEUE_COLORED_ELEMENT)
		{
			PSchElementColor* p_PSchElementColorStored = static_cast<PSchElementColor*>(pc_QueueSegmentStored->p_vUnitObject);
			//
			if(p_PSchElementColorStored->ullIDInt == aPSchElementColor.ullIDInt)
			{
				l_Queue.removeAt(iN);
				iQ--;
				goto gF;
			}
		}
		iN++;
	}
	_SQ_Util(QUEUE_COLORED_ELEMENT, PSchElementColor);
}
// Добавление удаления элемента.
void Environment::EventsQueue::AddEraseElement(PSchElementEraser& aPSchElementEraser, bool bDirectionOut)
{
	_SQ_Util(QUEUE_ERASED_ELEMENT, PSchElementEraser);
}
/////////////////////////////////// ЛИНК ////////////////////////////////////
// Добавление нового линка.
void Environment::EventsQueue::AddNewLink(PSchLinkBase& aPSchLinkBase, bool bDirectionOut)
{
	_SQ_Util(QUEUE_NEW_LINK, PSchLinkBase);
}
// Добавление изменений линка.
void Environment::EventsQueue::AddLinkChanges(PSchLinkVars& aPSchLinkVars, bool bDirectionOut)
{
	_SQ_Util(QUEUE_CHANGED_LINK, PSchLinkVars);
}
// Добавление удаления линка.
void Environment::EventsQueue::AddEraseLink(PSchLinkEraser& aPSchLinkEraser, bool bDirectionOut)
{
	_SQ_Util(QUEUE_ERASED_LINK, PSchLinkEraser);
}
/////////////////////////////////// ГРУППА ////////////////////////////////////
// Добавление новой группы.
void Environment::EventsQueue::AddNewGroup(PSchGroupBase& aPSchGroupBase, bool bDirectionOut)
{
	_SQ_Util(QUEUE_NEW_GROUP, PSchGroupBase);
}
// Добавление изменений группы.
void Environment::EventsQueue::AddGroupChanges(PSchGroupVars& aPSchGroupVars, bool bDirectionOut)
{
	_SQ_Util(QUEUE_CHANGED_GROUP, PSchGroupVars);
}
// Добавление изменения имени группы.
void Environment::EventsQueue::AddGroupRenameAndFlush(PSchGroupName& aPSchGroupName, bool bDirectionOut)
{
	// Удаление всех предыдущих переименований в цепочке (ни на что не отразится).
	int iQ = l_Queue.count();
	int iN = 0;
	//
gF:	while(iN < iQ)
	{
		const QueueSegment* pc_QueueSegmentStored = &l_Queue.at(iN);
		//
		if(pc_QueueSegmentStored->uchType == QUEUE_RENAMED_GROUP)
		{
			PSchGroupName* p_PSchGroupNameStored = static_cast<PSchGroupName*>(pc_QueueSegmentStored->p_vUnitObject);
			//
			if(p_PSchGroupNameStored->ullIDInt == aPSchGroupName.ullIDInt)
			{
				l_Queue.removeAt(iN);
				iQ--;
				goto gF;
			}
		}
		iN++;
	}
	_SQ_Util(QUEUE_RENAMED_GROUP, PSchGroupName);
}
// Добавление изменения цвета гуппы и очистка аналогов в очереди.
void Environment::EventsQueue::AddGroupColorAndFlush(PSchGroupColor& aPSchGroupColor, bool bDirectionOut)
{
	// Удаление всех предыдущих перекрасов в цепочке (ни на что не отразится).
	int iQ = l_Queue.count();
	int iN = 0;
	//
gF:	while(iN < iQ)
	{
		const QueueSegment* pc_QueueSegmentStored = &l_Queue.at(iN);
		//
		if(pc_QueueSegmentStored->uchType == QUEUE_COLORED_GROUP)
		{
			PSchGroupColor* p_PSchGroupColorStored = static_cast<PSchGroupColor*>(pc_QueueSegmentStored->p_vUnitObject);
			//
			if(p_PSchGroupColorStored->ullIDInt == aPSchGroupColor.ullIDInt)
			{
				l_Queue.removeAt(iN);
				iQ--;
				goto gF;
			}
		}
		iN++;
	}
	_SQ_Util(QUEUE_COLORED_GROUP, PSchGroupColor);
}
// Добавление удаления группы.
void Environment::EventsQueue::AddEraseGroup(PSchGroupEraser& aPSchGroupEraser, bool bDirectionOut)
{
	_SQ_Util(QUEUE_ERASED_GROUP, PSchGroupEraser);
}
/////////////////////////////////// ПСЕВДОНИМ ////////////////////////////////////
// Добавление нового псевдонима и очистка аналогов в очереди.
void Environment::EventsQueue::AddSetPseudonymAndFlush(PSchPseudonym& aPSchPseudonym, bool bDirectionOut)
{
	// Удаление всех предыдущих псевдонимов в цепочке (ни на что не отразится).
	int iQ = l_Queue.count();
	int iN = 0;
	//
gF:	while(iN < iQ)
	{
		const QueueSegment* pc_QueueSegmentStored = &l_Queue.at(iN);
		//
		if(pc_QueueSegmentStored->uchType == QUEUE_SET_PSEUDONYM)
		{
			PSchPseudonym* p_PSchPseudonymStored = static_cast<PSchPseudonym*>(pc_QueueSegmentStored->p_vUnitObject);
			//
			if(p_PSchPseudonymStored->ushiPort == aPSchPseudonym.ushiPort)
			{
				l_Queue.removeAt(iN);
				iQ--;
				goto gF;
			}
		}
		iN++;
	}
	_SQ_Util(QUEUE_SET_PSEUDONYM, PSchPseudonym);
}
// Добавление удаления псевдонима.
void Environment::EventsQueue::AddErasePseudonym(PSchPseudonymEraser& aPSchPseudonymEraser, bool bDirectionOut)
{
	_SQ_Util(QUEUE_ERASED_PSEUDONYM, PSchPseudonymEraser);
}
//////////////////////////////////////////////////////////////////////////////////
// Получение данных из позиции.
const Environment::EventsQueue::QueueSegment* Environment::EventsQueue::Get(int iNum)
{
	if(l_Queue.isEmpty()) return nullptr;
	else return &l_Queue.at(iNum);
}
// Очистка и удаление позиции.
void Environment::EventsQueue::Remove(int iNum)
{
	const QueueSegment* pc_QueueSegment = &l_Queue.at(iNum);
	//
	switch(pc_QueueSegment->uchType)
	{
		case QUEUE_NEW_ELEMENT:{delete static_cast<PSchElementBase*>(pc_QueueSegment->p_vUnitObject); break;}
		case QUEUE_CHANGED_ELEMENT:{delete static_cast<PSchElementVars*>(pc_QueueSegment->p_vUnitObject); break;}
		case QUEUE_RENAMED_ELEMENT:{delete static_cast<PSchElementName*>(pc_QueueSegment->p_vUnitObject); break;}
		case QUEUE_COLORED_ELEMENT:{delete static_cast<PSchElementColor*>(pc_QueueSegment->p_vUnitObject); break;}
		case QUEUE_ERASED_ELEMENT:{delete static_cast<PSchElementEraser*>(pc_QueueSegment->p_vUnitObject); break;}
		case QUEUE_NEW_LINK:{delete static_cast<PSchLinkBase*>(pc_QueueSegment->p_vUnitObject); break;}
		case QUEUE_CHANGED_LINK:{delete static_cast<PSchLinkVars*>(pc_QueueSegment->p_vUnitObject); break;}
		case QUEUE_ERASED_LINK:{delete static_cast<PSchLinkEraser*>(pc_QueueSegment->p_vUnitObject); break;}
		case QUEUE_NEW_GROUP:{delete static_cast<PSchGroupBase*>(pc_QueueSegment->p_vUnitObject); break;}
		case QUEUE_CHANGED_GROUP:{delete static_cast<PSchGroupVars*>(pc_QueueSegment->p_vUnitObject); break;}
		case QUEUE_RENAMED_GROUP:{delete static_cast<PSchGroupName*>(pc_QueueSegment->p_vUnitObject); break;}
		case QUEUE_COLORED_GROUP:{delete static_cast<PSchGroupColor*>(pc_QueueSegment->p_vUnitObject); break;}
		case QUEUE_ERASED_GROUP:{delete static_cast<PSchGroupEraser*>(pc_QueueSegment->p_vUnitObject); break;}
	}
	l_Queue.removeAt(iNum);
}
// Очистка цепочки с удалением содержимого.
void Environment::EventsQueue::Clear()
{
	while(l_Queue.count())
	{
		Remove(0);
	}
	uiCurrentSegNumber = 0;
}
// Получение длины цепочки.
int Environment::EventsQueue::Count()
{
	return l_Queue.count();
}

//== Класс среды.
// Конструктор.
Environment::Environment(pthread_mutex_t ptLogMutex, char* p_chEnvName)
{
	LOG_CTRL_BIND_EXT_MUTEX(ptLogMutex);
	LOG_CTRL_INIT;
	p_EventsQueue = new EventsQueue;
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
	ReleasePB(Pseudonym);
	delete p_EventsQueue;
	LOG_CLOSE;
}

// Прогрузка элемента и его разновидностей.
bool Environment::InitElementForEnv(PSchElementBase& a_oPSchElementBase, XMLNode* p_NodeElement, unsigned char uchElementTypeBits)
{
	bool bPresent;
	QString strHelper;
	QStringList lstrHelper;
	// cppcheck-suppress memsetClassFloat
	memset(&a_oPSchElementBase, 0, sizeof(a_oPSchElementBase));
	bPresent = false;
	FIND_IN_CHILDLIST(p_NodeElement, p_ListIDs,
					  m_chID, FCN_ONE_LEVEL, p_NodeID)
	{
		strHelper = QString(p_NodeID->FirstChild()->Value());
		if(strHelper.isEmpty())
		{
			LOG_P_0(LOG_CAT_E,
					m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect <<
					m_chLogWrong << m_chLogID << m_chLogNode);
			return false;
		}
		a_oPSchElementBase.oPSchElementVars.ullIDInt = strHelper.toULongLong();
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
					m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect <<
					m_chLogWrong << m_chLogName << m_chLogNode);
			return false;
		}
		memcpy(a_oPSchElementBase.m_chName,
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
					m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect <<
					m_chLogWrong << m_chLogBkgColor << m_chLogNode);
			return false;
		}
		lstrHelper = strHelper.split(',');
		if(lstrHelper.count() != 4)
		{
			LOG_P_0(LOG_CAT_E,
					m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect <<
					m_chLogWrong << m_chLogBkgColor << m_chLogNode);
			return false;
		}
		a_oPSchElementBase.uiObjectBkgColor =
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
					m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect <<
					m_chLogWrong << m_chLogFrame << m_chLogNode);
			return false;
		}
		lstrHelper = strHelper.split(',');
		if(lstrHelper.count() != 4)
		{
			LOG_P_0(LOG_CAT_E,
					m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect <<
					m_chLogWrong << m_chLogFrame << m_chLogNode);
			return false;
		}
		a_oPSchElementBase.oPSchElementVars.oSchEGGraph.oDbFrame.dbX = lstrHelper.at(0).toDouble();
		a_oPSchElementBase.oPSchElementVars.oSchEGGraph.oDbFrame.dbY = lstrHelper.at(1).toDouble();
		a_oPSchElementBase.oPSchElementVars.oSchEGGraph.oDbFrame.dbW = lstrHelper.at(2).toDouble();
		a_oPSchElementBase.oPSchElementVars.oSchEGGraph.oDbFrame.dbH = lstrHelper.at(3).toDouble();
		bPresent = true;
	} FIND_IN_CHILDLIST_END(p_ListFrames);
	if(!bPresent)
	{
		LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvElement <<
				m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogFrame << m_chLogNode);
		return false;
	}
	FIND_IN_CHILDLIST(p_NodeElement, p_ListMinimizes,
					  m_chMinimized, FCN_ONE_LEVEL, p_NodeMinimize)
	{
		DRNodeMinimize;
		a_oPSchElementBase.oPSchElementVars.oSchEGGraph.uchSettingsBits |= SCH_SETTINGS_EG_BIT_MIN;
	} FIND_IN_CHILDLIST_END(p_ListMinimizes);
	if(uchElementTypeBits & SCH_SETTINGS_ELEMENT_BIT_EXTENDED)
	{
		FIND_IN_CHILDLIST(p_NodeElement, p_ListExtPorts,
						  m_chExtPort, FCN_ONE_LEVEL, p_NodeExtPort)
		{
			strHelper = QString(p_NodeExtPort->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect <<
						m_chLogWrong << m_chLogExtPort << m_chLogNode);
				return false;
			}
			a_oPSchElementBase.oPSchElementVars.ushiExtPort = strHelper.toUShort();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListExtPorts);
	}
	if(!bPresent)
	{
		LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvElement <<
				m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogExtPort << m_chLogNode);
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
					m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect <<
					m_chLogWrong << m_chLogZ << m_chLogNode);
			return false;
		}
		a_oPSchElementBase.oPSchElementVars.oSchEGGraph.dbObjectZPos = strHelper.toDouble();
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
					m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect <<
					m_chLogWrong << "'GroupID'" << m_chLogNode);
			return false;
		}
		a_oPSchElementBase.oPSchElementVars.ullIDGroup = strHelper.toULongLong();
	} FIND_IN_CHILDLIST_END(p_ListGroupIDs);
	a_oPSchElementBase.oPSchElementVars.oSchEGGraph.uchSettingsBits |= uchElementTypeBits;
	AppendToPB(Element, new Element(a_oPSchElementBase));
	return true;
}

// Загрузка среды.
bool Environment::LoadEnv()
{
	PSchGroupBase oPSchGroupBase;
	PSchElementBase oPSchElementBase;
	PSchLinkBase oPSchLinkBase;
	PSchPseudonym oPSchPseudonym;
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
	list<XMLNode*> l_pPseudonyms;
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
		LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chGroups << m_chLogEnvNodeAbsend);
		return false;
	}
	if(!FindChildNodes(xmlDocEnv.LastChild(), l_pElements,
					   m_chElements, FCN_ONE_LEVEL, FCN_FIRST_ONLY))
	{
		LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chElements << m_chLogEnvNodeAbsend);
		return false;
	}
	if(!FindChildNodes(xmlDocEnv.LastChild(), l_pLinks,
					   m_chLinks, FCN_ONE_LEVEL, FCN_FIRST_ONLY))
	{
		LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLinks << m_chLogEnvNodeAbsend);
		return false;
	}
	if(!FindChildNodes(xmlDocEnv.LastChild(), l_pPseudonyms,
					   m_chPseudonyms, FCN_ONE_LEVEL, FCN_FIRST_ONLY))
	{
		LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chPseudonyms << m_chLogEnvNodeAbsend);
		return false;
	}
	// ГРУППЫ.
	PARSE_CHILDLIST(l_pGroups.front(), p_ListGroups, m_chGroup,
					FCN_ONE_LEVEL, p_NodeGroup)
	{
		// cppcheck-suppress memsetClassFloat
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
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvGroup <<
					m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogID << m_chLogNode);
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
						m_chLogEnvFileCorrupt << m_chLogEnvGroup << m_chLogEnvNodeFormatIncorrect <<
						m_chLogWrong << m_chLogName << m_chLogNode);
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
						m_chLogEnvFileCorrupt << m_chLogEnvGroup << m_chLogEnvNodeFormatIncorrect <<
						m_chLogWrong << m_chLogBkgColor << m_chLogNode);
				return false;
			}
			lstrHelper = strHelper.split(',');
			if(lstrHelper.count() != 4)
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvGroup << m_chLogEnvNodeFormatIncorrect <<
						m_chLogWrong << m_chLogBkgColor << m_chLogNode);
				return false;
			}
			oPSchGroupBase.uiObjectBkgColor =
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
						m_chLogEnvFileCorrupt << m_chLogEnvGroup << m_chLogEnvNodeFormatIncorrect <<
						m_chLogWrong << m_chLogFrame << m_chLogNode);
				return false;
			}
			lstrHelper = strHelper.split(',');
			if(lstrHelper.count() != 4)
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvGroup << m_chLogEnvNodeFormatIncorrect <<
						m_chLogWrong << m_chLogFrame << m_chLogNode);
				return false;
			}
			oPSchGroupBase.oPSchGroupVars.oSchEGGraph.oDbFrame.dbX = lstrHelper.at(0).toDouble();
			oPSchGroupBase.oPSchGroupVars.oSchEGGraph.oDbFrame.dbY = lstrHelper.at(1).toDouble();
			oPSchGroupBase.oPSchGroupVars.oSchEGGraph.oDbFrame.dbW = lstrHelper.at(2).toDouble();
			oPSchGroupBase.oPSchGroupVars.oSchEGGraph.oDbFrame.dbH = lstrHelper.at(3).toDouble();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListFrames);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvGroup <<
					m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogFrame << m_chLogNode);
			return false;
		}
		FIND_IN_CHILDLIST(p_NodeGroup, p_ListMinimizes,
						  m_chMinimized, FCN_ONE_LEVEL, p_NodeMinimize)
		{
			DRNodeMinimize;
			oPSchGroupBase.oPSchGroupVars.oSchEGGraph.uchSettingsBits |= SCH_SETTINGS_EG_BIT_MIN;
		} FIND_IN_CHILDLIST_END(p_ListMinimizes);
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeGroup, p_ListZs,
						  m_chZ, FCN_ONE_LEVEL, p_NodeZ)
		{
			strHelper = QString(p_NodeZ->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvGroup << m_chLogEnvNodeFormatIncorrect <<
						m_chLogWrong << m_chLogZ << m_chLogNode);
				return false;
			}
			oPSchGroupBase.oPSchGroupVars.oSchEGGraph.dbObjectZPos = strHelper.toDouble();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListZs);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvGroup << m_chLogEnvNodeFormatIncorrect <<
					m_chLogMissing << m_chLogZ << m_chLogNode);
			return false;
		}
		FIND_IN_CHILDLIST(p_NodeGroup, p_ListGroupIDs,
						  m_chGroupID, FCN_ONE_LEVEL, p_NodeGroupID)
		{
			strHelper = QString(p_NodeGroupID->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvGroup << m_chLogEnvNodeFormatIncorrect <<
						m_chLogWrong << "'GroupID'" << m_chLogNode);
				return false;
			}
			oPSchGroupBase.oPSchGroupVars.ullIDGroup = strHelper.toULongLong();
		} FIND_IN_CHILDLIST_END(p_ListGroupIDs);
		AppendToPB(Group, new Group(oPSchGroupBase, false));
	} PARSE_CHILDLIST_END(p_ListGroups);
	// Заполнение вложенностей групп.
	for(uint  uiF = 0; uiF != PBCount(Group); uiF++)
	{
		Group* p_Group = PBAccess(Group, uiF);
		//
		p_Group->p_GroupAbove = nullptr;
		// Если не была вписана в группу, а надо...
		if(p_Group->oPSchGroupBase.oPSchGroupVars.ullIDGroup != 0)
		{
			for(uint uiG = 0; uiG != PBCount(Group); uiG++)
			{
				Group* p_GroupInt = PBAccess(Group, uiG);
				//
				if(p_GroupInt != p_Group)
				{
					if(p_GroupInt->oPSchGroupBase.oPSchGroupVars.ullIDInt == p_Group->oPSchGroupBase.oPSchGroupVars.ullIDGroup)
					{
						p_Group->p_GroupAbove = p_GroupInt;
						p_Group->p_GroupAbove->vp_ConnectedGroups.append(p_Group);
						break;
					}
				}
			}
		}
	}
	// ЭЛЕМЕНТЫ.
	PARSE_CHILDLIST(l_pElements.front(), p_ListElements, m_chElement,
					FCN_ONE_LEVEL, p_NodeElement)
	{
		if(!InitElementForEnv(oPSchElementBase, p_NodeElement, 0)) return false;
	} PARSE_CHILDLIST_END(p_ListElements);
	// ТРАНСЛЯТОРЫ.
	PARSE_CHILDLIST(l_pElements.front(), p_ListBroadcasters, m_chBroadcaster,
					FCN_ONE_LEVEL, p_NodeBroadcaster)
	{
		if(!InitElementForEnv(oPSchElementBase, p_NodeBroadcaster, SCH_SETTINGS_ELEMENT_BIT_EXTENDED))
			return false;
	} PARSE_CHILDLIST_END(p_ListBroadcasters);
	// ПРИЁМНИКИ.
	PARSE_CHILDLIST(l_pElements.front(), p_ListReceivers, m_chReceiver,
					FCN_ONE_LEVEL, p_NodeReceiver)
	{
		if(!InitElementForEnv(oPSchElementBase, p_NodeReceiver, SCH_SETTINGS_ELEMENT_BIT_EXTENDED | SCH_SETTINGS_ELEMENT_BIT_RECEIVER))
			return false;
	} PARSE_CHILDLIST_END(p_ListReceivers);
	// ЛИНКИ.
	PARSE_CHILDLIST(l_pLinks.front(), p_ListLinks, m_chLink,
					FCN_ONE_LEVEL, p_NodeLink)
	{
		// cppcheck-suppress memsetClassFloat
		memset(&oPSchLinkBase, 0, sizeof(oPSchLinkBase));
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodeLink, p_ListSrcIDs,
						  m_chSrcID, FCN_ONE_LEVEL, p_NodeSrcID)
		{
			strHelper = QString(p_NodeSrcID->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvLink << m_chLogEnvNodeFormatIncorrect <<
						m_chLogWrong << m_chLogSrcID << m_chLogNode);
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
						m_chLogEnvFileCorrupt << m_chLogEnvLink << m_chLogEnvNodeFormatIncorrect <<
						m_chLogWrong << m_chLogSrcPortID << m_chLogNode);
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
						m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect <<
						m_chLogWrong << m_chLogSrcPortPos << m_chLogNode);
				return false;
			}
			lstrHelper = strHelper.split(',');
			if(lstrHelper.count() != 2)
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect <<
						m_chLogWrong << m_chLogSrcPortPos << m_chLogNode);
				return false;
			}
			oPSchLinkBase.oPSchLinkVars.oSchLGraph.oDbSrcPortGraphPos.dbX = lstrHelper.at(0).toDouble();
			oPSchLinkBase.oPSchLinkVars.oSchLGraph.oDbSrcPortGraphPos.dbY = lstrHelper.at(1).toDouble();
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
						m_chLogEnvFileCorrupt << m_chLogEnvLink << m_chLogEnvNodeFormatIncorrect <<
						m_chLogWrong << m_chLogDstID << m_chLogNode);
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
						m_chLogEnvFileCorrupt << m_chLogEnvLink << m_chLogEnvNodeFormatIncorrect <<
						m_chLogWrong << m_chLogDstPortID << m_chLogNode);
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
			oPSchLinkBase.oPSchLinkVars.oSchLGraph.oDbDstPortGraphPos.dbX = lstrHelper.at(0).toDouble();
			oPSchLinkBase.oPSchLinkVars.oSchLGraph.oDbDstPortGraphPos.dbY = lstrHelper.at(1).toDouble();
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
	// ПСЕВДОНИМЫ.
	PARSE_CHILDLIST(l_pPseudonyms.front(), p_ListPseudonyms, m_chPseudonym,
					FCN_ONE_LEVEL, p_NodePseudonym)
	{
		memset(&oPSchPseudonym, 0, sizeof(oPSchPseudonym));
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodePseudonym, p_ListPortIDs,
						  m_chPortID, FCN_ONE_LEVEL, p_NodePortID)
		{
			strHelper = QString(p_NodePortID->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvPseudonym << m_chLogEnvNodeFormatIncorrect <<
						m_chLogWrong << m_chLogPortID << m_chLogNode);
				return false;
			}
			oPSchPseudonym.ushiPort = strHelper.toUShort();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListPortIDs);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvPseudonym <<
					m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogPortID << m_chLogNode);
			return false;
		}
		bPresent = false;
		FIND_IN_CHILDLIST(p_NodePseudonym, p_ListNames,
						  m_chName, FCN_ONE_LEVEL, p_NodeName)
		{
			strHelper = QString(p_NodeName->FirstChild()->Value());
			if(strHelper.isEmpty())
			{
				LOG_P_0(LOG_CAT_E,
						m_chLogEnvFileCorrupt << m_chLogEnvPseudonym << m_chLogEnvNodeFormatIncorrect <<
						m_chLogWrong << m_chLogName << m_chLogNode);
				return false;
			}
			memcpy(oPSchPseudonym.m_chName,
				   strHelper.toStdString().c_str(), SizeOfChars(strHelper.toStdString().length() + 1));
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListNames);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvPseudonym <<
					m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogName << m_chLogNode);
			return false;
		}
		AppendToPB(Pseudonym, new Pseudonym(oPSchPseudonym));
	} PARSE_CHILDLIST_END(p_ListPseudonyms);
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
	XMLNode* p_NodePseudonyms;
	XMLNode* p_NodePseudonym;
	XMLNode* p_NodeID;
	XMLNode* p_NodeName;
	XMLNode* p_NodeBkgColor;
	XMLNode* p_NodeFrame;
	XMLNode* p_NodeExtPort;
	XMLNode* p_NodeZ;
	XMLNode* p_NodeGroupID;
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
		oQColor.setRgba(PBAccess(Group,iF)->oPSchGroupBase.uiObjectBkgColor);
		oQColor.getRgb(&iR, &iG, &iB, &iA);
		p_NodeBkgColor->ToElement()->
				SetText((strHOne.setNum(iR) + "," + strHTwo.setNum(iG) + "," + strHThree.setNum(iB) +
						 "," + strHFour.setNum(iA)).toStdString().c_str());
		p_NodeFrame = p_NodeGroup->InsertEndChild(xmlEnv.NewElement(m_chFrame));
		p_NodeFrame->ToElement()->
				SetText((strHOne.setNum(PBAccess(Group,iF)->oPSchGroupBase.oPSchGroupVars.oSchEGGraph.oDbFrame.dbX) + "," +
						 strHTwo.setNum(PBAccess(Group,iF)->oPSchGroupBase.oPSchGroupVars.oSchEGGraph.oDbFrame.dbY) + "," +
						 strHThree.setNum(PBAccess(Group,iF)->oPSchGroupBase.oPSchGroupVars.oSchEGGraph.oDbFrame.dbW) + "," +
						 strHFour.setNum(PBAccess(Group,iF)->oPSchGroupBase.oPSchGroupVars.oSchEGGraph.oDbFrame.dbH)).
						toStdString().c_str());
		if(PBAccess(Group,iF)->oPSchGroupBase.oPSchGroupVars.oSchEGGraph.uchSettingsBits & SCH_SETTINGS_EG_BIT_MIN)
		{
			p_NodeGroup->InsertEndChild(xmlEnv.NewElement(m_chMinimized));
		}
		p_NodeZ = p_NodeGroup->InsertEndChild(xmlEnv.NewElement(m_chZ));
		p_NodeZ->ToElement()->
				SetText(strHOne.setNum(PBAccess(Group,iF)->oPSchGroupBase.oPSchGroupVars.oSchEGGraph.dbObjectZPos).
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
		if(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.oSchEGGraph.uchSettingsBits & SCH_SETTINGS_ELEMENT_BIT_EXTENDED)
		{
			if(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.oSchEGGraph.uchSettingsBits & SCH_SETTINGS_ELEMENT_BIT_RECEIVER)
				p_NodeElement = p_NodeElements->InsertEndChild(xmlEnv.NewElement(m_chReceiver));
			else p_NodeElement = p_NodeElements->InsertEndChild(xmlEnv.NewElement(m_chBroadcaster));
		}
		else p_NodeElement = p_NodeElements->InsertEndChild(xmlEnv.NewElement(m_chElement));
		p_NodeID = p_NodeElement->InsertEndChild(xmlEnv.NewElement(m_chID));
		p_NodeID->ToElement()->
				SetText(strHOne.setNum(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.ullIDInt).toStdString().c_str());
		p_NodeName = p_NodeElement->InsertEndChild(xmlEnv.NewElement(m_chName));
		p_NodeName->ToElement()->
				SetText(PBAccess(Element,iF)->oPSchElementBase.m_chName);
		p_NodeBkgColor = p_NodeElement->InsertEndChild(xmlEnv.NewElement(m_chBkgColor));
		oQColor = QColor();
		oQColor.setRgba(PBAccess(Element,iF)->oPSchElementBase.uiObjectBkgColor);
		oQColor.getRgb(&iR, &iG, &iB, &iA);
		p_NodeBkgColor->ToElement()->
				SetText((strHOne.setNum(iR) + "," + strHTwo.setNum(iG) + "," + strHThree.setNum(iB) + "," +
						 strHFour.setNum(iA)).toStdString().c_str());
		p_NodeFrame = p_NodeElement->InsertEndChild(xmlEnv.NewElement(m_chFrame));
		p_NodeFrame->ToElement()->
				SetText((strHOne.setNum(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.
										oSchEGGraph.oDbFrame.dbX) + "," +
						 strHTwo.setNum(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.
										oSchEGGraph.oDbFrame.dbY) + "," +
						 strHThree.setNum(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.
										  oSchEGGraph.oDbFrame.dbW) + "," +
						 strHFour.setNum(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.
										 oSchEGGraph.oDbFrame.dbH)).
						toStdString().c_str());
		if(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.oSchEGGraph.uchSettingsBits & SCH_SETTINGS_EG_BIT_MIN)
		{
			p_NodeElement->InsertEndChild(xmlEnv.NewElement(m_chMinimized));
		}
		if(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.oSchEGGraph.uchSettingsBits & SCH_SETTINGS_ELEMENT_BIT_EXTENDED)
		{
			p_NodeExtPort = p_NodeElement->InsertEndChild(xmlEnv.NewElement(m_chExtPort));
			p_NodeExtPort->ToElement()->
					SetText(strHOne.setNum(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.ushiExtPort).toStdString().c_str());
		}
		p_NodeZ = p_NodeElement->InsertEndChild(xmlEnv.NewElement(m_chZ));
		p_NodeZ->ToElement()->
				SetText(strHOne.setNum(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.oSchEGGraph.dbObjectZPos).
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
		XMLNode* p_NodeSrcID;
		XMLNode* p_NodeSrcPortID;
		XMLNode* p_NodeSrcPortPos;
		XMLNode* p_NodeDstID;
		XMLNode* p_NodeDstPortID;
		XMLNode* p_NodeDstPortPos;
		//
		p_NodeLink = p_NodeLinks->InsertEndChild(xmlEnv.NewElement(m_chLink));
		p_NodeSrcID = p_NodeLink->InsertEndChild(xmlEnv.NewElement(m_chSrcID));
		p_NodeSrcID->ToElement()->
				SetText(strHOne.setNum(PBAccess(Link,iF)->oPSchLinkBase.oPSchLinkVars.ullIDSrc).toStdString().c_str());
		p_NodeSrcPortID = p_NodeLink->InsertEndChild(xmlEnv.NewElement(m_chSrcPortID));
		p_NodeSrcPortID->ToElement()->
				SetText(strHOne.setNum(PBAccess(Link,iF)->oPSchLinkBase.oPSchLinkVars.ushiSrcPort).toStdString().c_str());
		p_NodeSrcPortPos = p_NodeLink->InsertEndChild(xmlEnv.NewElement(m_chSrcPortPos));
		p_NodeSrcPortPos->ToElement()->
				SetText((strHOne.setNum(PBAccess(Link,iF)->oPSchLinkBase.oPSchLinkVars.oSchLGraph.oDbSrcPortGraphPos.dbX) + "," +
						 strHTwo.setNum(PBAccess(Link,iF)->oPSchLinkBase.oPSchLinkVars.oSchLGraph.oDbSrcPortGraphPos.dbY)).
						toStdString().c_str());
		p_NodeDstID = p_NodeLink->InsertEndChild(xmlEnv.NewElement(m_chDstID));
		p_NodeDstID->ToElement()->
				SetText(strHOne.setNum(PBAccess(Link,iF)->oPSchLinkBase.oPSchLinkVars.ullIDDst).toStdString().c_str());
		p_NodeDstPortID = p_NodeLink->InsertEndChild(xmlEnv.NewElement(m_chDstPortID));
		p_NodeDstPortID->ToElement()->
				SetText(strHOne.setNum(PBAccess(Link,iF)->oPSchLinkBase.oPSchLinkVars.ushiDstPort).toStdString().c_str());
		p_NodeDstPortPos = p_NodeLink->InsertEndChild(xmlEnv.NewElement(m_chDstPortPos));
		p_NodeDstPortPos->ToElement()->
				SetText((strHOne.setNum(PBAccess(Link,iF)->oPSchLinkBase.oPSchLinkVars.oSchLGraph.oDbDstPortGraphPos.dbX) + "," +
						 strHTwo.setNum(PBAccess(Link,iF)->oPSchLinkBase.oPSchLinkVars.oSchLGraph.oDbDstPortGraphPos.dbY)).
						toStdString().c_str());
	}
	p_NodePseudonyms = p_NodeRoot->InsertEndChild(xmlEnv.NewElement(m_chPseudonyms));
	for(unsigned int iF = 0; iF != PBCount(Pseudonym); iF++)
	{
		XMLNode* p_NodePortID;
		//
		p_NodePseudonym = p_NodePseudonyms->InsertEndChild(xmlEnv.NewElement(m_chPseudonym));
		p_NodePortID = p_NodePseudonym->InsertEndChild(xmlEnv.NewElement(m_chPortID));
		p_NodePortID->ToElement()->
				SetText(strHOne.setNum(PBAccess(Pseudonym,iF)->oPSchPseudonym.ushiPort).toStdString().c_str());
		p_NodeName = p_NodePseudonym->InsertEndChild(xmlEnv.NewElement(m_chName));
		p_NodeName->ToElement()->
				SetText(PBAccess(Pseudonym,iF)->oPSchPseudonym.m_chName);
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

// Прогрузка цепочки событий для подключившегося клиента.
void Environment::FetchEnvToQueue()
{
	unsigned int uiG = PBCount(Group);
	unsigned int uiE = PBCount(Element);
	unsigned int uiL = PBCount(Link);
	unsigned int uiP = PBCount(Pseudonym);
	//
	while(p_EventsQueue->Count() != 0)
	{
		MSleep(ENV_STEP_WAITING);
	}
	TryMutexInit;
	TryMutexLock(ptQueueMutex);
	if((uiG + uiE + uiL + uiP) == 0)
	{
		iLastFetchingSegNumber = UPLOAD_STATUS_EMPTY;
		goto gEm;
	}
	for(unsigned int iF = 0; iF != uiG; iF++)
	{
		p_EventsQueue->AddNewGroup(PBAccess(Group,iF)->oPSchGroupBase, QUEUE_TO_CLIENT);
	}
	for(unsigned int iF = 0; iF != uiE; iF++)
	{
		p_EventsQueue->AddNewElement(PBAccess(Element,iF)->oPSchElementBase, QUEUE_TO_CLIENT);
	}
	for(unsigned int iF = 0; iF != uiL; iF++)
	{
		p_EventsQueue->AddNewLink(PBAccess(Link,iF)->oPSchLinkBase, QUEUE_TO_CLIENT);
	}
	for(unsigned int iF = 0; iF != uiP; iF++)
	{
		p_EventsQueue->AddSetPseudonymAndFlush(PBAccess(Pseudonym,iF)->oPSchPseudonym, QUEUE_TO_CLIENT);
	}
	iLastFetchingSegNumber = (int)EventsQueue::uiCurrentSegNumber - 1; // Если не грузилось ничего, будет статус UPLOAD_STATUS_INACTIVE автом.
gEm:TryMutexUnlock(ptQueueMutex);
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
	// cppcheck-suppress memsetClassFloat
	memset(&oPSchReadyFrame, 0, sizeof(PSchReadyFrame));
	SafeThreadStart(thrEnv, EnvThread, nullptr);
	return true;
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
}

// Проверка инициализированности среды.
bool Environment::CheckInitialized()
{
	return bEnvLoaded;
}

// Поток шагов среды.
void* Environment::EnvThread(void*)
{
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
	LOG_P_1(LOG_CAT_I, "Finishing client operations...");
	while(NetOperations())
	{
		MSleep(ENV_STEP_WAITING);
	}
	bEnvThreadAlive = false;
	LOG_P_1(LOG_CAT_I, "Environment thread terminated.");
	bStopEnvUpdate = false;
	RETURN_THREAD;
}

// Проверка и установка признака последней новости.
void Environment::CheckLastInQueue(unsigned short ushNewsQantity, bool& abLastInQueue)
{
	if(ushNewsQantity > 1)
	{
		abLastInQueue = false;
	}
	else
	{
		abLastInQueue = true;
	}
}

// Работа с сетью.
bool Environment::NetOperations()
{
	PSchElementBase* p_PSchElementBase;
	PSchElementVars* p_PSchElementVars;
	PSchElementName* p_PSchElementName;
	PSchElementColor* p_PSchElementColor;
	PSchElementEraser* p_PSchElementEraser;
	PSchLinkBase* p_PSchLinkBase;
	PSchLinkVars* p_PSchLinkVars;
	PSchLinkEraser* p_PSchLinkEraser;
	PSchGroupBase* p_PSchGroupBase;
	PSchGroupVars* p_PSchGroupVars;
	PSchGroupName* p_PSchGroupName;
	PSchGroupColor* p_PSchGroupColor;
	PSchGroupEraser* p_PSchGroupEraser;
	PSchPseudonym* p_PSchPseudonym;
	PSchPseudonymEraser* p_PSchPseudonymEraser;
	//
	bool bPresent = false;
	unsigned short ushNewsQantity = 2;
	bool bAllowToClient = false;
	//
	if(MainWindow::p_Server->GetConnectionData(0, DONT_TRY_LOCK).iStatus != NO_CONNECTION)
	{
		if(MainWindow::p_Server->IsConnectionSecured(0))
		{
			bAllowToClient = true;
		}
	}
	if(bRequested) // Если был запрос сессии от клиента...
	{
		TryMutexInit;
		TryMutexLock(ptQueueMutex);
		// Цикл по всей очереди событий до конца или исчерпания лимита новостей (если новосте меньше, чем очередь).
		if(ushNewsQantity > p_EventsQueue->Count()) ushNewsQantity = p_EventsQueue->Count();
		while(p_EventsQueue->Count())
		{
			if(ushNewsQantity > 0)
			{
				const EventsQueue::QueueSegment* pc_QueueSegment = p_EventsQueue->Get(0);
				//
				switch(pc_QueueSegment->uchType)
				{
					case QUEUE_NEW_ELEMENT:
					{
						p_PSchElementBase = static_cast<PSchElementBase*>(pc_QueueSegment->p_vUnitObject);
						if((pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT) && bAllowToClient)
						{
							CheckLastInQueue(ushNewsQantity, p_PSchElementBase->oPSchElementVars.bLastInQueue);
							LCHECK_BOOL(MainWindow::p_Server->AddPocketToOutputBuffer(0,
																					  PROTO_O_SCH_ELEMENT_BASE,
																					  (char*)p_PSchElementBase,
																					  sizeof(PSchElementBase)));
							LOG_P_2(LOG_CAT_I, "{Out} New element [" << QString(p_PSchElementBase->m_chName).toStdString()
									<< m_chLogSentToClient);
							bPresent = true;
						}
						else
						{
							Element* p_Element;
							//
							LOG_P_2(LOG_CAT_I, "{In} Element [" << QString(p_PSchElementBase->m_chName).toStdString()
									<< "] base from client.");
							AppendToPB(Element, p_Element = new Element(*p_PSchElementBase));
						}
						break;
					}
					case QUEUE_CHANGED_ELEMENT:
					{
						p_PSchElementVars = static_cast<PSchElementVars*>(pc_QueueSegment->p_vUnitObject);
						if((pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT) && bAllowToClient)
						{
							CheckLastInQueue(ushNewsQantity, p_PSchElementVars->bLastInQueue);
							LCHECK_BOOL(MainWindow::p_Server->AddPocketToOutputBuffer(0,
																					  PROTO_O_SCH_ELEMENT_VARS,
																					  (char*)p_PSchElementVars,
																					  sizeof(PSchElementVars)));
							LOG_P_2(LOG_CAT_I, "{Out} Changed element [" <<
									QString::number(p_PSchElementVars->ullIDInt).toStdString()
									<< m_chLogSentToClient);
							bPresent = true;
						}
						else
						{
							int iEC;
							//
							LOG_P_2(LOG_CAT_I, "{In} Element vars from client.");
							iEC = PBCount(Element);
							for(int iE = 0; iE < iEC; iE++) // По всем элементам...
							{
								Element* p_Element;
								//
								p_Element = PBAccess(Element, iE);
								if(p_Element->oPSchElementBase.oPSchElementVars.ullIDInt ==
								   p_PSchElementVars->ullIDInt) // При совп. с запрошенным...
								{
									if(p_PSchElementVars->oSchEGGraph.uchChangesBits & SCH_CHANGES_ELEMENT_BIT_ZPOS)
									{
										p_Element->oPSchElementBase.oPSchElementVars.oSchEGGraph.dbObjectZPos =
												p_PSchElementVars->oSchEGGraph.dbObjectZPos;
										LOG_P_2(LOG_CAT_I, "Element [" << QString(p_Element->oPSchElementBase.m_chName).toStdString()
												<< "] z-pos is: " <<
												QString::number((int)p_PSchElementVars->oSchEGGraph.dbObjectZPos).toStdString());
									}
									if(p_PSchElementVars->oSchEGGraph.uchChangesBits & SCH_CHANGES_ELEMENT_BIT_BUSY)
									{
										CopyBits(p_PSchElementVars->oSchEGGraph.uchSettingsBits,
												 p_Element->oPSchElementBase.oPSchElementVars.oSchEGGraph.uchSettingsBits,
												 SCH_SETTINGS_EG_BIT_BUSY);
										if(p_Element->oPSchElementBase.oPSchElementVars.oSchEGGraph.uchSettingsBits &
										   SCH_SETTINGS_EG_BIT_BUSY)
										{
											LOG_P_2(LOG_CAT_I, "Element [" <<
													QString(p_Element->oPSchElementBase.m_chName).toStdString()
													<< "] is busy by client.");
										}
										else
										{
											LOG_P_2(LOG_CAT_I, "Element [" <<
													QString(p_Element->oPSchElementBase.m_chName).toStdString()
													<< "] is free.");
										}
									}
									if(p_PSchElementVars->oSchEGGraph.uchChangesBits & SCH_CHANGES_ELEMENT_BIT_FRAME)
									{
										p_Element->oPSchElementBase.oPSchElementVars.oSchEGGraph.oDbFrame =
												p_PSchElementVars->oSchEGGraph.oDbFrame;
										LOG_P_2(LOG_CAT_I, "Element [" << QString(p_Element->oPSchElementBase.m_chName).toStdString()
												<< "] frame.");
									}
									if(p_PSchElementVars->oSchEGGraph.uchChangesBits & SCH_CHANGES_ELEMENT_BIT_MIN)
									{
										CopyBits(p_PSchElementVars->oSchEGGraph.uchSettingsBits,
												 p_Element->oPSchElementBase.oPSchElementVars.oSchEGGraph.uchSettingsBits,
												 SCH_SETTINGS_EG_BIT_MIN);
										if(p_Element->oPSchElementBase.oPSchElementVars.oSchEGGraph.uchSettingsBits &
										   SCH_SETTINGS_EG_BIT_MIN)
										{
											LOG_P_2(LOG_CAT_I, "Element [" <<
													QString(p_Element->oPSchElementBase.m_chName).toStdString()
													<< "] minimized.");
										}
										else
										{
											LOG_P_2(LOG_CAT_I, "Element [" <<
													QString(p_Element->oPSchElementBase.m_chName).toStdString()
													<< "] restored.");
										}
									}
									if(p_PSchElementVars->oSchEGGraph.uchChangesBits & SCH_CHANGES_ELEMENT_BIT_EXTPORT)
									{
										p_Element->oPSchElementBase.oPSchElementVars.ushiExtPort = p_PSchElementVars->ushiExtPort;
										if(p_Element->oPSchElementBase.oPSchElementVars.oSchEGGraph.uchSettingsBits &
										   SCH_SETTINGS_ELEMENT_BIT_RECEIVER)
										{
											LOG_P_2(LOG_CAT_I, "Receiver [" <<
													QString(p_Element->oPSchElementBase.m_chName).toStdString()
													<< "] external port changed.");
										}
										else
										{
											LOG_P_2(LOG_CAT_I, "Broadcaster [" <<
													QString(p_Element->oPSchElementBase.m_chName).toStdString()
													<< "] external port changed.");
										}
									}
									if(p_PSchElementVars->oSchEGGraph.uchChangesBits & SCH_CHANGES_ELEMENT_BIT_GROUP)
									{
										if(p_PSchElementVars->ullIDGroup == 0) // Обработка отсоединения от группы.
										{
											if(p_Element->p_Group != nullptr)
											{
												if(p_Element->p_Group->vp_ConnectedElements.contains(p_Element))
												{
													p_Element->p_Group->vp_ConnectedElements.removeOne(p_Element);
													LOG_P_2(LOG_CAT_I, "Element [" <<
															QString(p_Element->oPSchElementBase.m_chName).toStdString()
															<< "] group - detach.");
													if(p_Element->p_Group->vp_ConnectedElements.isEmpty() &
													   p_Element->p_Group->vp_ConnectedGroups.isEmpty())
													{
														LOG_P_2(LOG_CAT_I, "Group is empty - erase.");
														Environment::EraseGroup(p_Element->p_Group);
													}
													p_Element->p_Group = nullptr;
													p_Element->oPSchElementBase.oPSchElementVars.ullIDGroup = 0;
													goto gEOK;
												}
												else
												{
gGEx:														LOG_P_0(LOG_CAT_E, "Error element detaching from group.");
													goto gEOK;
												}
											}
											else goto gGEx;
										}
										// Обработка включения в группу.
										for(int iG = 0; iG < (int)PBCount(Group); iG++)
										{
											if(PBAccess(Group, iG)->
											   oPSchGroupBase.oPSchGroupVars.ullIDInt == p_PSchElementVars->ullIDGroup)
											{
												p_Element->p_Group = PBAccess(Group, iG);
												if(p_Element->oPSchElementBase.oPSchElementVars.ullIDGroup !=
												   p_PSchElementVars->ullIDGroup)
												{
													p_Element->oPSchElementBase.oPSchElementVars.ullIDGroup =
															p_PSchElementVars->ullIDGroup;
													LOG_P_2(LOG_CAT_I, "Element [" <<
															QString(p_Element->oPSchElementBase.m_chName).toStdString()
															<< "] group - attach.");
													p_Element->p_Group->vp_ConnectedElements.append(p_Element);
												}
												goto gEOK;
											}
										}
										LOG_P_0(LOG_CAT_W, "Wrong group number for element");
									}
									goto gEOK;
								}
							}
							LOG_P_0(LOG_CAT_W, "Wrong element number from client.");
						}
						break;
					}
					case QUEUE_RENAMED_ELEMENT:
					{
						p_PSchElementName = static_cast<PSchElementName*>(pc_QueueSegment->p_vUnitObject);
						if((pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT) && bAllowToClient)
						{
							CheckLastInQueue(ushNewsQantity, p_PSchElementName->bLastInQueue);
							LCHECK_BOOL(MainWindow::p_Server->
										AddPocketToOutputBuffer(0,
																PROTO_O_SCH_ELEMENT_NAME,
																(char*)p_PSchElementName, sizeof(PSchElementName)));
							LOG_P_2(LOG_CAT_I, "{Out} Renamed element [" << QString(p_PSchElementName->m_chName).toStdString()
									<< m_chLogSentToClient);
							bPresent = true;
						}
						else
						{
							int iEC;
							//
							LOG_P_2(LOG_CAT_I, "{In} Element name from client.");
							iEC = PBCount(Element);
							for(int iE = 0; iE < iEC; iE++) // По всем элементам...
							{
								Element* p_Element;
								//
								p_Element = PBAccess(Element, iE);
								if(p_Element->oPSchElementBase.oPSchElementVars.ullIDInt ==
								   p_PSchElementName->ullIDInt) // При совп. с запрошенным...
								{
									CopyStrArray(p_PSchElementName->m_chName,
												 p_Element->oPSchElementBase.m_chName, SCH_OBJ_NAME_STR_LEN);
									goto gEOK;
								}
							}
							LOG_P_0(LOG_CAT_W, "Wrong element number from client.");
						}
						break;
					}
					case QUEUE_COLORED_ELEMENT:
					{
						p_PSchElementColor = static_cast<PSchElementColor*>(pc_QueueSegment->p_vUnitObject);
						if((pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT) && bAllowToClient)
						{
							CheckLastInQueue(ushNewsQantity, p_PSchElementColor->bLastInQueue);
							LCHECK_BOOL(MainWindow::p_Server->AddPocketToOutputBuffer(0,
																					  PROTO_O_SCH_ELEMENT_COLOR,
																					  (char*)p_PSchElementColor,
																					  sizeof(PSchElementColor)));
							LOG_P_2(LOG_CAT_I, "{Out} Recolored element [" <<
									QString::number(p_PSchElementColor->ullIDInt).toStdString()
									<< m_chLogSentToClient);
							bPresent = true;
						}
						else
						{
							int iEC;
							//
							LOG_P_2(LOG_CAT_I, "{In} Element color change from client.");
							iEC = PBCount(Element);
							for(int iE = 0; iE < iEC; iE++) // По всем элементам...
							{
								Element* p_Element;
								//
								p_Element = PBAccess(Element, iE);
								if(p_Element->oPSchElementBase.oPSchElementVars.ullIDInt ==
								   p_PSchElementColor->ullIDInt) // При совп. с запрошенным...
								{
									p_Element->oPSchElementBase.uiObjectBkgColor = p_PSchElementColor->uiObjectBkgColor;
									LOG_P_2(LOG_CAT_I, "Element [" <<
											QString(p_Element->oPSchElementBase.m_chName).toStdString() << "] color changed.");
									goto gEOK;
								}
							}
							LOG_P_0(LOG_CAT_W, "Wrong element number for change color from client.");
						}
						break;
					}
					case QUEUE_ERASED_ELEMENT:
					{
						p_PSchElementEraser = static_cast<PSchElementEraser*>(pc_QueueSegment->p_vUnitObject);
						if((pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT) && bAllowToClient)
						{
							CheckLastInQueue(ushNewsQantity, p_PSchElementEraser->bLastInQueue);
							LCHECK_BOOL(MainWindow::p_Server->AddPocketToOutputBuffer(0,
																					  PROTO_O_SCH_ELEMENT_ERASE,
																					  (char*)p_PSchElementEraser,
																					  sizeof(PSchElementEraser)));
							LOG_P_2(LOG_CAT_I, "{Out} Erased element [" <<
									QString::number(p_PSchElementEraser->ullIDInt).toStdString()
									<< m_chLogSentToClient);
							bPresent = true;
						}
						else
						{
							int iEC;
							//
							LOG_P_2(LOG_CAT_I, "{In} Element for erase from client.");
							iEC = PBCount(Element);
							for(int iE = 0; iE < iEC; iE++) // По всем элементам...
							{
								Element* p_Element;
								//
								p_Element = PBAccess(Element, iE);
								if(p_Element->oPSchElementBase.oPSchElementVars.ullIDInt ==
								   p_PSchElementEraser->ullIDInt) // При совп. с запрошенным...
								{
									LOG_P_2(LOG_CAT_I, "Element [" <<
											QString(p_Element->oPSchElementBase.m_chName).toStdString() << "] erase.");
									EraseElementAt(iE);
									goto gEOK;
								}
							}
							LOG_P_0(LOG_CAT_W, "Wrong element number for erase from client.");
						}
						break;
					}
					case QUEUE_NEW_LINK:
					{
						p_PSchLinkBase = static_cast<PSchLinkBase*>(pc_QueueSegment->p_vUnitObject);
						if((pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT) && bAllowToClient)
						{
							CheckLastInQueue(ushNewsQantity, p_PSchLinkBase->oPSchLinkVars.bLastInQueue);
							LCHECK_BOOL(MainWindow::p_Server->AddPocketToOutputBuffer(0,
																					  PROTO_O_SCH_LINK_BASE,
																					  (char*)p_PSchLinkBase,
																					  sizeof(PSchLinkBase)));
							LOG_P_2(LOG_CAT_I, "{Out} New link [" <<
									QString::number(p_PSchLinkBase->oPSchLinkVars.ullIDSrc).toStdString()
									<< "<>" <<
									QString::number(p_PSchLinkBase->oPSchLinkVars.ullIDDst).toStdString()
									<< m_chLogSentToClient);
							bPresent = true;
						}
						else
						{
							Link* p_Link;
							char* p_chSrc = nullptr;
							char* p_chDst = nullptr;
							//
							for(unsigned int uiF = 0; uiF != PBCount(Element); uiF++)
							{
								Element* p_Element = PBAccess(Element, uiF);
								//
								if(p_PSchLinkBase->oPSchLinkVars.ullIDSrc == p_Element->oPSchElementBase.oPSchElementVars.ullIDInt)
								{
									p_chSrc = p_Element->oPSchElementBase.m_chName;
								}
								if(p_PSchLinkBase->oPSchLinkVars.ullIDDst == p_Element->oPSchElementBase.oPSchElementVars.ullIDInt)
								{
									p_chDst = p_Element->oPSchElementBase.m_chName;
								}
								if((p_chSrc != nullptr) & (p_chDst != nullptr)) goto gLO;
							}
							LOG_P_0(LOG_CAT_W, "Wrong link elements from client.");
							goto gEOK;
gLO:								LOG_P_2(LOG_CAT_I, "{In} Link [" << p_chSrc << "<>" << p_chDst << "] base from client.");
							AppendToPB(Link, p_Link = new Link(*p_PSchLinkBase));
						}
						break;
					}
					case QUEUE_CHANGED_LINK:
					{
						p_PSchLinkVars = static_cast<PSchLinkVars*>(pc_QueueSegment->p_vUnitObject);
						if((pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT) && bAllowToClient)
						{
							CheckLastInQueue(ushNewsQantity, p_PSchLinkVars->bLastInQueue);
							LCHECK_BOOL(MainWindow::p_Server->AddPocketToOutputBuffer(0,
																					  PROTO_O_SCH_LINK_VARS,
																					  (char*)p_PSchLinkVars,
																					  sizeof(PSchLinkVars)));
							LOG_P_2(LOG_CAT_I, "{Out} Changed link [" <<
									QString::number(p_PSchLinkVars->ullIDSrc).toStdString()
									<< "<>" <<
									QString::number(p_PSchLinkVars->ullIDDst).toStdString()
									<< m_chLogSentToClient);
							bPresent = true;
						}
						else
						{
							int iLC;
							//
							LOG_P_2(LOG_CAT_I, "{In} Link vars from client.");
							iLC = PBCount(Link);
							for(int iL = 0; iL < iLC; iL++) // По всем линкам...
							{
								Link* p_Link;
								//
								p_Link = PBAccess(Link, iL);
								if((p_Link->oPSchLinkBase.oPSchLinkVars.ullIDSrc == p_PSchLinkVars->ullIDSrc) &&
								   (p_Link->oPSchLinkBase.oPSchLinkVars.ullIDDst == p_PSchLinkVars->ullIDDst) &&
								   (p_Link->oPSchLinkBase.oPSchLinkVars.ushiSrcPort == p_PSchLinkVars->ushiSrcPort) &&
								   (p_Link->oPSchLinkBase.oPSchLinkVars.ushiDstPort == p_PSchLinkVars->ushiDstPort))
									// При совпадении с запрошенным...
								{
									if(p_PSchLinkVars->oSchLGraph.uchChangesBits & SCH_CHANGES_LINK_BIT_SCR_PORT_POS)
									{
										p_Link->oPSchLinkBase.oPSchLinkVars.oSchLGraph.oDbSrcPortGraphPos =
												p_PSchLinkVars->oSchLGraph.oDbSrcPortGraphPos;
										LOG_P_2(LOG_CAT_I, "Link vars - src port position.");
									}
									if(p_PSchLinkVars->oSchLGraph.uchChangesBits & SCH_CHANGES_LINK_BIT_DST_PORT_POS)
									{
										p_Link->oPSchLinkBase.oPSchLinkVars.oSchLGraph.oDbDstPortGraphPos =
												p_PSchLinkVars->oSchLGraph.oDbDstPortGraphPos;
										LOG_P_2(LOG_CAT_I, "Link vars - dst port position.");
									}
									goto gEOK;
								}
							}
							LOG_P_0(LOG_CAT_W, "Wrong link number from client.");
						}
						break;
					}
					case QUEUE_ERASED_LINK:
					{
						p_PSchLinkEraser = static_cast<PSchLinkEraser*>(pc_QueueSegment->p_vUnitObject);
						if((pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT) && bAllowToClient)
						{
							CheckLastInQueue(ushNewsQantity, p_PSchLinkEraser->bLastInQueue);
							LCHECK_BOOL(MainWindow::p_Server->AddPocketToOutputBuffer(0,
																					  PROTO_O_SCH_LINK_ERASE,
																					  (char*)p_PSchLinkEraser,
																					  sizeof(PSchLinkEraser)));
							LOG_P_2(LOG_CAT_I, "{Out} Erased link [" <<
									QString::number(p_PSchLinkEraser->ullIDSrc).toStdString()
									<< "<>" <<
									QString::number(p_PSchLinkEraser->ullIDDst).toStdString()
									<< m_chLogSentToClient);
							bPresent = true;
						}
						else
						{
							int iLC;
							//
							LOG_P_2(LOG_CAT_I, "{In} Link for erase from client.");
							iLC = PBCount(Link);
							for(int iL = 0; iL < iLC; iL++) // По всем линкам...
							{
								Link* p_Link;
								//
								p_Link = PBAccess(Link, iL);
								if((p_Link->oPSchLinkBase.oPSchLinkVars.ullIDDst == p_PSchLinkEraser->ullIDDst) &
								   (p_Link->oPSchLinkBase.oPSchLinkVars.ullIDSrc == p_PSchLinkEraser->ullIDSrc) &
								   (p_Link->oPSchLinkBase.oPSchLinkVars.ushiDstPort ==
									p_PSchLinkEraser->ushiDstPort) &
								   (p_Link->oPSchLinkBase.oPSchLinkVars.ushiSrcPort ==
									p_PSchLinkEraser->ushiSrcPort)) // При совп. с запрошенным...
								{
									LOG_P_2(LOG_CAT_I, "Link erase.");
									EraseLinkAt(iL);
									goto gEOK;
								}
							}
							LOG_P_0(LOG_CAT_W, "Wrong link for erase from client.");
						}
						break;
					}
					case QUEUE_NEW_GROUP:
					{
						p_PSchGroupBase = static_cast<PSchGroupBase*>(pc_QueueSegment->p_vUnitObject);
						if((pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT) && bAllowToClient)
						{
							CheckLastInQueue(ushNewsQantity, p_PSchGroupBase->oPSchGroupVars.bLastInQueue);
							LCHECK_BOOL(MainWindow::p_Server->AddPocketToOutputBuffer(0,
																					  PROTO_O_SCH_GROUP_BASE,
																					  (char*)p_PSchGroupBase,
																					  sizeof(PSchGroupBase)));
							LOG_P_2(LOG_CAT_I, "{Out} New group [" << QString(p_PSchGroupBase->m_chName).toStdString()
									<< m_chLogSentToClient);
							bPresent = true;
						}
						else
						{
							Group* p_Group;
							//
							LOG_P_2(LOG_CAT_I, "{In} Group [" << QString(p_PSchGroupBase->m_chName).toStdString()
									<< "] base from client.");
							AppendToPB(Group, p_Group = new Group(*p_PSchGroupBase));
						}
						break;
					}
					case QUEUE_CHANGED_GROUP:
					{
						p_PSchGroupVars = static_cast<PSchGroupVars*>(pc_QueueSegment->p_vUnitObject);
						if((pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT) && bAllowToClient)
						{
							CheckLastInQueue(ushNewsQantity, p_PSchGroupVars->bLastInQueue);
							LCHECK_BOOL(MainWindow::p_Server->AddPocketToOutputBuffer(0,
																					  PROTO_O_SCH_GROUP_VARS,
																					  (char*)p_PSchGroupVars,
																					  sizeof(PSchGroupVars)));
							LOG_P_2(LOG_CAT_I, "{Out} Changed group [" <<
									QString::number(p_PSchGroupVars->ullIDInt).toStdString()
									<< m_chLogSentToClient);
							bPresent = true;
						}
						else
						{
							int iGC;
							//
							LOG_P_2(LOG_CAT_I, "{In} Group vars from client.");
							iGC = PBCount(Group);
							for(int iE = 0; iE < iGC; iE++) // По всем группам...
							{
								Group* p_Group;
								//
								p_Group = PBAccess(Group, iE);
								if(p_Group->oPSchGroupBase.oPSchGroupVars.ullIDInt ==
								   p_PSchGroupVars->ullIDInt) // При совп. с запрошенным...
								{
									if(p_PSchGroupVars->oSchEGGraph.uchChangesBits & SCH_CHANGES_GROUP_BIT_FRAME)
									{
										p_Group->oPSchGroupBase.oPSchGroupVars.oSchEGGraph.oDbFrame =
												p_PSchGroupVars->oSchEGGraph.oDbFrame;
										LOG_P_2(LOG_CAT_I, "Group [" << QString(p_Group->oPSchGroupBase.m_chName).toStdString()
												<< "] frame.");
									}
									if(p_PSchGroupVars->oSchEGGraph.uchChangesBits & SCH_CHANGES_GROUP_BIT_MIN)
									{
										CopyBits(p_PSchGroupVars->oSchEGGraph.uchSettingsBits,
												 p_Group->oPSchGroupBase.oPSchGroupVars.oSchEGGraph.uchSettingsBits,
												 SCH_SETTINGS_EG_BIT_MIN);
										if(p_Group->oPSchGroupBase.oPSchGroupVars.oSchEGGraph.uchSettingsBits &
										   SCH_SETTINGS_EG_BIT_MIN)
										{
											LOG_P_2(LOG_CAT_I, "Group [" <<
													QString(p_Group->oPSchGroupBase.m_chName).toStdString()
													<< "] minimized.");
										}
										else
										{
											LOG_P_2(LOG_CAT_I, "Group [" <<
													QString(p_Group->oPSchGroupBase.m_chName).toStdString()
													<< "] restored.");
										}
									}
									if(p_PSchGroupVars->oSchEGGraph.uchChangesBits & SCH_CHANGES_GROUP_BIT_ZPOS)
									{
										p_Group->oPSchGroupBase.oPSchGroupVars.oSchEGGraph.dbObjectZPos =
												p_PSchGroupVars->oSchEGGraph.dbObjectZPos;
										LOG_P_2(LOG_CAT_I, "Group [" << QString(p_Group->oPSchGroupBase.m_chName).toStdString()
												<< "] z-pos is: " <<
												QString::number((int)p_PSchGroupVars->oSchEGGraph.dbObjectZPos).toStdString());
									}
									if(p_PSchGroupVars->oSchEGGraph.uchChangesBits & SCH_CHANGES_GROUP_BIT_BUSY)
									{
										CopyBits(p_PSchGroupVars->oSchEGGraph.uchSettingsBits,
												 p_Group->oPSchGroupBase.oPSchGroupVars.oSchEGGraph.uchSettingsBits,
												 SCH_SETTINGS_EG_BIT_BUSY);
										if(p_Group->oPSchGroupBase.oPSchGroupVars.oSchEGGraph.uchSettingsBits &
										   SCH_SETTINGS_EG_BIT_BUSY)
										{
											LOG_P_2(LOG_CAT_I, "Group [" << QString(p_Group->oPSchGroupBase.m_chName).toStdString()
													<< "] is busy by client.");
										}
										else
										{
											LOG_P_2(LOG_CAT_I, "Group [" << QString(p_Group->oPSchGroupBase.m_chName).toStdString()
													<< "] is free.");
										}
									}
									if(p_PSchGroupVars->oSchEGGraph.uchChangesBits & SCH_CHANGES_GROUP_BIT_GROUP)
									{
										if(p_PSchGroupVars->ullIDGroup == 0) // Обработка отсоединения от группы.
										{
											if(p_Group->p_GroupAbove != nullptr)
											{
												if(p_Group->p_GroupAbove->vp_ConnectedGroups.contains(p_Group))
												{
													p_Group->p_GroupAbove->vp_ConnectedGroups.removeAll(p_Group);
													LOG_P_2(LOG_CAT_I, "Group [" <<
															QString(p_Group->oPSchGroupBase.m_chName).toStdString()
															<< "] group - detach.");
													if(p_Group->p_GroupAbove->vp_ConnectedGroups.isEmpty() &
													   p_Group->p_GroupAbove->vp_ConnectedElements.isEmpty())
													{
														LOG_P_2(LOG_CAT_I, "Group is empty - erase.");
														EraseGroup(p_Group->p_GroupAbove);
													}
													p_Group->p_GroupAbove = nullptr;
													p_Group->oPSchGroupBase.oPSchGroupVars.ullIDGroup = 0;
													goto gEOK;
												}
												else
												{
gGGEx:												LOG_P_0(LOG_CAT_E, "Error detaching group from group.");
													goto gEOK;
												}
											}
											else goto gGGEx;
										}
										// Обработка включения в группу.
										for(int iG = 0; iG < (int)PBCount(Group); iG++)
										{
											if(PBAccess(Group, iG)->
											   oPSchGroupBase.oPSchGroupVars.ullIDInt == p_PSchGroupVars->ullIDGroup)
											{
												p_Group->p_GroupAbove = PBAccess(Group, iG);
												if(p_Group->oPSchGroupBase.oPSchGroupVars.ullIDGroup != p_PSchGroupVars->ullIDGroup)
												{
													p_Group->oPSchGroupBase.oPSchGroupVars.ullIDGroup = p_PSchGroupVars->ullIDGroup;
													LOG_P_2(LOG_CAT_I, "Group [" <<
															QString(p_Group->oPSchGroupBase.m_chName).toStdString()
															<< "] group - attach.");
													p_Group->p_GroupAbove->vp_ConnectedGroups.append(p_Group);
												}
												goto gEOK;
											}
										}
										LOG_P_0(LOG_CAT_W, "Wrong group number for group");
									}
									goto gEOK;
								}
							}
							LOG_P_0(LOG_CAT_W, "Wrong group number from client.");
						}
						break;
					}
					case QUEUE_RENAMED_GROUP:
					{
						p_PSchGroupName = static_cast<PSchGroupName*>(pc_QueueSegment->p_vUnitObject);
						if((pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT) && bAllowToClient)
						{
							CheckLastInQueue(ushNewsQantity, p_PSchGroupName->bLastInQueue);
							LCHECK_BOOL(MainWindow::p_Server->AddPocketToOutputBuffer(0,
																					  PROTO_O_SCH_GROUP_NAME,
																					  (char*)p_PSchGroupName, sizeof(PSchGroupName)));
							LOG_P_2(LOG_CAT_I, "{Out} Renamed group [" << QString(p_PSchGroupName->m_chName).toStdString()
									<< m_chLogSentToClient);
							bPresent = true;
						}
						else
						{
							int iEC;
							//
							LOG_P_2(LOG_CAT_I, "{In} Group name from client.");
							iEC = PBCount(Group);
							for(int iE = 0; iE < iEC; iE++) // По всем группам...
							{
								Group* p_Group;
								//
								p_Group = PBAccess(Group, iE);
								if(p_Group->oPSchGroupBase.oPSchGroupVars.ullIDInt ==
								   p_PSchGroupName->ullIDInt) // При совп. с запрошенным...
								{
									CopyStrArray(p_PSchGroupName->m_chName, p_Group->oPSchGroupBase.m_chName, SCH_OBJ_NAME_STR_LEN);
									goto gEOK;
								}
							}
							LOG_P_0(LOG_CAT_W, "Wrong group number from client.");
						}
						break;
					}
					case QUEUE_COLORED_GROUP:
					{
						p_PSchGroupColor = static_cast<PSchGroupColor*>(pc_QueueSegment->p_vUnitObject);
						if((pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT) && bAllowToClient)
						{
							CheckLastInQueue(ushNewsQantity, p_PSchGroupColor->bLastInQueue);
							LCHECK_BOOL(MainWindow::p_Server->AddPocketToOutputBuffer(0,
																					  PROTO_O_SCH_GROUP_COLOR,
																					  (char*)p_PSchGroupColor,
																					  sizeof(PSchGroupColor)));
							LOG_P_2(LOG_CAT_I, "{Out} Recolored group [" <<
									QString::number(p_PSchGroupColor->ullIDInt).toStdString()
									<< m_chLogSentToClient);
							bPresent = true;
						}
						else
						{
							int iGC;
							//
							LOG_P_2(LOG_CAT_I, "{In} Group color change from client.");
							iGC = PBCount(Group);
							for(int iG = 0; iG < iGC; iG++) // По всем группам...
							{
								Group* p_Group;
								//
								p_Group = PBAccess(Group, iG);
								if(p_Group->oPSchGroupBase.oPSchGroupVars.ullIDInt ==
								   p_PSchGroupColor->ullIDInt) // При совп. с запрошенным...
								{
									p_Group->oPSchGroupBase.uiObjectBkgColor = p_PSchGroupColor->uiObjectBkgColor;
									LOG_P_2(LOG_CAT_I, "Group [" <<
											QString(p_Group->oPSchGroupBase.m_chName).toStdString() << "] color changed.");
									goto gEOK;
								}
							}
							LOG_P_0(LOG_CAT_W, "Wrong group number for change color from client.");
						}
						break;
					}
					case QUEUE_ERASED_GROUP:
					{
						p_PSchGroupEraser = static_cast<PSchGroupEraser*>(pc_QueueSegment->p_vUnitObject);
						if((pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT) && bAllowToClient)
						{
							CheckLastInQueue(ushNewsQantity, p_PSchGroupEraser->bLastInQueue);
							LCHECK_BOOL(MainWindow::p_Server->AddPocketToOutputBuffer(0,
																					  PROTO_O_SCH_GROUP_ERASE,
																					  (char*)p_PSchGroupEraser,
																					  sizeof(PSchGroupEraser)));
							LOG_P_2(LOG_CAT_I, "{Out} Erased group [" <<
									QString::number(p_PSchGroupEraser->ullIDInt).toStdString()
									<< m_chLogSentToClient);
							bPresent = true;
						}
						else
						{
							int iGC;
							//
							LOG_P_2(LOG_CAT_I, "{In} Group for erase from client.");
							iGC = PBCount(Group);
							for(int iG = 0; iG < iGC; iG++) // По всем группам...
							{
								Group* p_Group;
								//
								p_Group = PBAccess(Group, iG);
								if(p_Group->oPSchGroupBase.oPSchGroupVars.ullIDInt ==
								   p_PSchGroupEraser->ullIDInt) // При совп. с запрошенным...
								{
									LOG_P_2(LOG_CAT_I, "Group [" <<
											QString(p_Group->oPSchGroupBase.m_chName).toStdString() << "] erase.");
									EraseGroupAt(iG);
									goto gEOK;
								}
							}
							LOG_P_0(LOG_CAT_W, "Wrong group number for erase from client.");
						}
						break;
					}
					case QUEUE_SET_PSEUDONYM:
					{
						p_PSchPseudonym = static_cast<PSchPseudonym*>(pc_QueueSegment->p_vUnitObject);
						if((pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT) && bAllowToClient)
						{
							CheckLastInQueue(ushNewsQantity, p_PSchPseudonym->bLastInQueue);
							LCHECK_BOOL(MainWindow::p_Server->AddPocketToOutputBuffer(0,
																					  PROTO_O_SCH_PSEUDONYM,
																					  (char*)p_PSchPseudonym,
																					  sizeof(PSchPseudonym)));
							LOG_P_2(LOG_CAT_I, "{Out} New pseudonym [" << QString(p_PSchPseudonym->m_chName).toStdString()
									<< m_chLogSentToClient);
							bPresent = true;
						}
						else
						{
							Pseudonym* p_Pseudonym;
							int iPC;
							//
							LOG_P_2(LOG_CAT_I, "{In} Pseudonym [" << QString(p_PSchElementBase->m_chName).toStdString()
									<< "] from client.");
							iPC = PBCount(Pseudonym);
							for(int iP = 0; iP < iPC; iP++) // По всем псевдонимам...
							{
								Pseudonym* p_PseudonymOld;
								//
								p_PseudonymOld = PBAccess(Pseudonym, iP);
								if(p_PSchPseudonym->ushiPort ==
								   p_PseudonymOld->oPSchPseudonym.ushiPort) // При совп. с существующим...
								{
									LOG_P_2(LOG_CAT_I, "Pseudonym [" <<
											QString(p_PseudonymOld->oPSchPseudonym.m_chName).toStdString() << "] replacing.");
									RemoveObjectFromPBByPos(Pseudonym, iP);
									break;
								}
							}
							AppendToPB(Pseudonym, p_Pseudonym = new Pseudonym(*p_PSchPseudonym));
						}
						break;
					}
					case QUEUE_ERASED_PSEUDONYM:
					{
						p_PSchPseudonymEraser = static_cast<PSchPseudonymEraser*>(pc_QueueSegment->p_vUnitObject);
						if((pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT) && bAllowToClient)
						{
							CheckLastInQueue(ushNewsQantity, p_PSchPseudonymEraser->bLastInQueue);
							LCHECK_BOOL(MainWindow::p_Server->AddPocketToOutputBuffer(0,
																					  PROTO_O_SCH_PSEUDONYM_ERASE,
																					  (char*)p_PSchPseudonymEraser,
																					  sizeof(PSchPseudonymEraser)));
							LOG_P_2(LOG_CAT_I, "{Out} Erased pseudonym for port [" <<
									QString::number(p_PSchPseudonymEraser->ushiPort).toStdString()
									<< m_chLogSentToClient);
							bPresent = true;
						}
						else
						{
							int iPC;
							//
							LOG_P_2(LOG_CAT_I, "{In} Pseudonym for erase from client.");
							iPC = PBCount(Pseudonym);
							for(int iP = 0; iP < iPC; iP++) // По всем псевдонимам...
							{
								Pseudonym* p_Pseudonym;
								//
								p_Pseudonym = PBAccess(Pseudonym, iP);
								if(p_Pseudonym->oPSchPseudonym.ushiPort ==
								   p_PSchPseudonymEraser->ushiPort) // При совп. с запрошенным...
								{
									LOG_P_2(LOG_CAT_I, "Pseudonym [" <<
											QString(p_Pseudonym->oPSchPseudonym.m_chName).toStdString() << "] erase.");
									RemoveObjectFromPBByPos(Pseudonym, iP);
									goto gEOK;
								}
							}
							LOG_P_0(LOG_CAT_W, "Wrong pseudonym number for erase from client.");
						}
gEOK:					break;
					}
				}
				p_EventsQueue->Remove(0);
				ushNewsQantity--;
			}
			else break;
		}
		//
		if(bPresent)
		{
			if(bAllowToClient)
			{
				LCHECK_BOOL(MainWindow::p_Server->SendBufferToClient(0, RESET_POINTER));
			}
		}
		TryMutexUnlock(ptQueueMutex);
	}
	return p_EventsQueue->Count() != 0;
}

// Удаление линка в позиции и удаление указателя на него.
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

// Удаление элемента в позиции и возможных опустевших групп рекурсивно.
void Environment::EraseElementAt(int iPos)
{
	Element* p_Element = PBAccess(Element, iPos);
	//
	if(p_Element->p_Group)
	{
		p_Element->p_Group->vp_ConnectedElements.removeAll(p_Element);
		EraseGroupIfEmptyAndParentsRecursively(p_Element->p_Group);
	}
	EraseLinksForElement(p_Element);
	RemoveObjectFromPBByPos(Element, iPos);
}

// Удаление группы в позиции и возможных опустевших групп рекурсивно.
void Environment::EraseGroupAt(int iPos)
{
	EraseGroup(PBAccess(Group, iPos));
}

/// Рекурсивное удаление детей группы и все их элементы, включая основную.
Group* Environment::EraseGroupAndChildrenAndElementsRecursively(Group* p_Group, bool bFirst)
{
	Group* p_GroupAboveInt = nullptr;
	//
	for(int iF = 0; iF != p_Group->vp_ConnectedElements.count(); iF++)
	{
		Element* p_Element = p_Group->vp_ConnectedElements.at(iF);
		//
		EraseLinksForElement(p_Element);
		RemoveObjectFromPBByPointer(Element, p_Element);
	}
	for(int iF = 0; iF != p_Group->vp_ConnectedGroups.count(); iF++)
	{
		// Рекурсия без удаления из группы-родителя (она сама).
		EraseGroupAndChildrenAndElementsRecursively(p_Group->vp_ConnectedGroups.at(iF), false);
	}
	if(bFirst)
	{
		p_GroupAboveInt = p_Group->p_GroupAbove;
		if(p_GroupAboveInt)
			p_GroupAboveInt->vp_ConnectedGroups.removeAll(p_Group); // На первом входе удаляем группу из списка у родителя.
	}
	RemoveObjectFromPBByPointer(Group, p_Group); // Удаление группы.
	return p_GroupAboveInt; // Возврат nullptr - родителей нет или адрес родителя.
}

// Рекурсивное удаление пустой группы и её пустых родителей рекурсивно.
void Environment::EraseGroupIfEmptyAndParentsRecursively(Group* p_ParentGroup)
{
	if(p_ParentGroup) // Для взаимодействия с EraseGroupAndChildrenAndElementsRecursively.
	{
		if(p_ParentGroup->vp_ConnectedElements.isEmpty() && p_ParentGroup->vp_ConnectedGroups.isEmpty()) // Если группа пустая...
		{
			if(p_ParentGroup->p_GroupAbove) // Если есть родитель...
			{
				p_ParentGroup->p_GroupAbove->vp_ConnectedGroups.removeAll(p_ParentGroup); // Удаление из списка у родителя.
				EraseGroupIfEmptyAndParentsRecursively(p_ParentGroup->p_GroupAbove); // Рекурсия для родителя.
			}
			RemoveObjectFromPBByPointer(Group, p_ParentGroup); // Удаление группы.
		}
	}
}

// Удаление группы по указателю и возможных опустевших групп.
void Environment::EraseGroup(Group* p_Group)
{
	EraseGroupIfEmptyAndParentsRecursively(EraseGroupAndChildrenAndElementsRecursively(p_Group));
}
