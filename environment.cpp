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
														*(Struct*)oQueueSegment.p_vUnitObject = a##Struct;						\
														l_Queue.append(oQueueSegment);
//== ДЕКЛАРАЦИИ СТАТИЧЕСКИХ ПЕРЕМЕННЫХ.
// Основное.
LOGDECL_INIT_INCLASS(Environment)
LOGDECL_INIT_PTHRD_INCLASS_EXT_ADD(Environment)
// Буферы.
StaticPBSourceInit(Element,, Environment, MAX_ELEMENTS)
StaticPBSourceInit(Broadcaster,, Environment, MAX_BROADCASTERS)
StaticPBSourceInit(Receiver,, Environment, MAX_RECEIVERS)
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
QList<Environment::EventsQueue::QueueSegment> Environment::EventsQueue::l_Queue;
Environment::EventsQueue::QueueSegment Environment::EventsQueue::oQueueSegment;
Environment::EventsQueue* Environment::p_EventsQueue = nullptr;
pthread_mutex_t Environment::ptQueueMutex = PTHREAD_MUTEX_INITIALIZER;
unsigned int Environment::EventsQueue::uiCurrentSegNumber;
int Environment::iLastFetchingSegNumber;
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
gF:	while(iN < iQ)
	{
		const QueueSegment* pc_QueueSegmentStored = &l_Queue.at(iN);
		if(pc_QueueSegmentStored->uchType == QUEUE_RENAMED_ELEMENT)
		{
			PSchElementName* p_PSchElementNameStored = (PSchElementName*)(pc_QueueSegmentStored->p_vUnitObject);
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
gF:	while(iN < iQ)
	{
		const QueueSegment* pc_QueueSegmentStored = &l_Queue.at(iN);
		if(pc_QueueSegmentStored->uchType == QUEUE_COLORED_ELEMENT)
		{
			PSchElementColor* p_PSchElementColorStored = (PSchElementColor*)(pc_QueueSegmentStored->p_vUnitObject);
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
gF:	while(iN < iQ)
	{
		const QueueSegment* pc_QueueSegmentStored = &l_Queue.at(iN);
		if(pc_QueueSegmentStored->uchType == QUEUE_RENAMED_GROUP)
		{
			PSchGroupName* p_PSchGroupNameStored = (PSchGroupName*)(pc_QueueSegmentStored->p_vUnitObject);
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
gF:	while(iN < iQ)
	{
		const QueueSegment* pc_QueueSegmentStored = &l_Queue.at(iN);
		if(pc_QueueSegmentStored->uchType == QUEUE_COLORED_ELEMENT)
		{
			PSchGroupColor* p_PSchGroupColorStored = (PSchGroupColor*)(pc_QueueSegmentStored->p_vUnitObject);
			if(p_PSchGroupColorStored->ullIDInt == aPSchGroupColor.ullIDInt)
			{
				l_Queue.removeAt(iN);
				iQ--;
				goto gF;
			}
		}
		iN++;
	}
	_SQ_Util(QUEUE_COLORED_ELEMENT, PSchGroupColor);
}
// Добавление удаления группы.
void Environment::EventsQueue::AddEraseGroup(PSchGroupEraser& aPSchGroupEraser, bool bDirectionOut)
{
	_SQ_Util(QUEUE_ERASED_GROUP, PSchGroupEraser);
}
/////////////////////////////////// ТРАНСЛЯТОР ////////////////////////////////////
// Добавление нового транслятора.
void Environment::EventsQueue::AddNewBroadcaster(PSchBroadcasterBase& aPSchBroadcasterBase, bool bDirectionOut)
{
	_SQ_Util(QUEUE_NEW_GROUP, PSchBroadcasterBase);
}
// Добавление изменений транслятора.
void Environment::EventsQueue::AddBroadcasterChanges(PSchBroadcasterVars& aPSchBroadcasterVars, bool bDirectionOut)
{
	_SQ_Util(QUEUE_CHANGED_LINK, PSchBroadcasterVars);
}
// Добавление изменения портов транслятора.
void Environment::EventsQueue::AddBroadcasterPorts(PSchBroadcasterPorts& aPSchBroadcasterPorts, bool bDirectionOut)
{
	_SQ_Util(QUEUE_ERASED_GROUP, PSchBroadcasterPorts);
}
// Добавление изменения имени транслятора.
void Environment::EventsQueue::AddBroadcasterRenameAndFlush(PSchBroadcasterName& aPSchBroadcasterName, bool bDirectionOut)
{
	// Удаление всех предыдущих переименований в цепочке (ни на что не отразится).
	int iQ = l_Queue.count();
	int iN = 0;
gF:	while(iN < iQ)
	{
		const QueueSegment* pc_QueueSegmentStored = &l_Queue.at(iN);
		if(pc_QueueSegmentStored->uchType == QUEUE_RENAMED_BROADCASTER)
		{
			PSchBroadcasterName* p_PSchBroadcasterNameStored = (PSchBroadcasterName*)(pc_QueueSegmentStored->p_vUnitObject);
			if(p_PSchBroadcasterNameStored->ullIDInt == aPSchBroadcasterName.ullIDInt)
			{
				l_Queue.removeAt(iN);
				iQ--;
				goto gF;
			}
		}
		iN++;
	}
	_SQ_Util(QUEUE_RENAMED_BROADCASTER, PSchBroadcasterName);
}
// Добавление изменения цвета транслятора и очистка аналогов в очереди.
void Environment::EventsQueue::AddBroadcasterColorAndFlush(PSchBroadcasterColor& aPSchBroadcasterColor, bool bDirectionOut)
{
	// Удаление всех предыдущих перекрасов в цепочке (ни на что не отразится).
	int iQ = l_Queue.count();
	int iN = 0;
gF:	while(iN < iQ)
	{
		const QueueSegment* pc_QueueSegmentStored = &l_Queue.at(iN);
		if(pc_QueueSegmentStored->uchType == QUEUE_COLORED_BROADCASTER)
		{
			PSchBroadcasterColor* p_PSchBroadcasterColorStored = (PSchBroadcasterColor*)(pc_QueueSegmentStored->p_vUnitObject);
			if(p_PSchBroadcasterColorStored->ullIDInt == aPSchBroadcasterColor.ullIDInt)
			{
				l_Queue.removeAt(iN);
				iQ--;
				goto gF;
			}
		}
		iN++;
	}
	_SQ_Util(QUEUE_COLORED_BROADCASTER, PSchBroadcasterColor);
}
// Добавление удаления транслятора.
void Environment::EventsQueue::AddEraseBroadcaster(PSchBroadcasterEraser& aPSchBroadcasterEraser, bool bDirectionOut)
{
	_SQ_Util(QUEUE_ERASED_GROUP, PSchBroadcasterEraser);
}
/////////////////////////////////// ПРИЁМНИК ////////////////////////////////////
// Добавление нового приёмника.
void Environment::EventsQueue::AddNewReceiver(PSchReceiverBase& aPSchReceiverBase, bool bDirectionOut)
{
	_SQ_Util(QUEUE_NEW_GROUP, PSchReceiverBase);
}
// Добавление изменений приёмника.
void Environment::EventsQueue::AddReceiverChanges(PSchReceiverVars& aPSchReceiverVars, bool bDirectionOut)
{
	_SQ_Util(QUEUE_CHANGED_LINK, PSchReceiverVars);
}
// Добавление изменения портов приёмника.
void Environment::EventsQueue::AddReceiverPorts(PSchReceiverPorts& aPSchReceiverPorts, bool bDirectionOut)
{
	_SQ_Util(QUEUE_ERASED_GROUP, PSchReceiverPorts);
}
// Добавление изменения имени приёмника.
void Environment::EventsQueue::AddReceiverRenameAndFlush(PSchReceiverName& aPSchReceiverName, bool bDirectionOut)
{
	// Удаление всех предыдущих переименований в цепочке (ни на что не отразится).
	int iQ = l_Queue.count();
	int iN = 0;
gF:	while(iN < iQ)
	{
		const QueueSegment* pc_QueueSegmentStored = &l_Queue.at(iN);
		if(pc_QueueSegmentStored->uchType == QUEUE_RENAMED_BROADCASTER)
		{
			PSchReceiverName* p_PSchReceiverNameStored = (PSchReceiverName*)(pc_QueueSegmentStored->p_vUnitObject);
			if(p_PSchReceiverNameStored->ullIDInt == aPSchReceiverName.ullIDInt)
			{
				l_Queue.removeAt(iN);
				iQ--;
				goto gF;
			}
		}
		iN++;
	}
	_SQ_Util(QUEUE_RENAMED_BROADCASTER, PSchReceiverName);
}
// Добавление изменения цвета приёмника и очистка аналогов в очереди.
void Environment::EventsQueue::AddReceiverColorAndFlush(PSchReceiverColor& aPSchReceiverColor, bool bDirectionOut)
{
	// Удаление всех предыдущих перекрасов в цепочке (ни на что не отразится).
	int iQ = l_Queue.count();
	int iN = 0;
gF:	while(iN < iQ)
	{
		const QueueSegment* pc_QueueSegmentStored = &l_Queue.at(iN);
		if(pc_QueueSegmentStored->uchType == QUEUE_COLORED_BROADCASTER)
		{
			PSchReceiverColor* p_PSchReceiverColorStored = (PSchReceiverColor*)(pc_QueueSegmentStored->p_vUnitObject);
			if(p_PSchReceiverColorStored->ullIDInt == aPSchReceiverColor.ullIDInt)
			{
				l_Queue.removeAt(iN);
				iQ--;
				goto gF;
			}
		}
		iN++;
	}
	_SQ_Util(QUEUE_COLORED_BROADCASTER, PSchReceiverColor);
}
// Добавление удаления приёмника.
void Environment::EventsQueue::AddEraseReceiver(PSchReceiverEraser& aPSchReceiverEraser, bool bDirectionOut)
{
	_SQ_Util(QUEUE_ERASED_GROUP, PSchReceiverEraser);
}
//////////////////////////////////////////////////////////////////////////////
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
		case QUEUE_NEW_ELEMENT:{delete (PSchElementBase*)pc_QueueSegment->p_vUnitObject; break;}
		case QUEUE_CHANGED_ELEMENT:{delete (PSchElementVars*)pc_QueueSegment->p_vUnitObject; break;}
		case QUEUE_RENAMED_ELEMENT:{delete (PSchElementName*)pc_QueueSegment->p_vUnitObject; break;}
		case QUEUE_COLORED_ELEMENT:{delete (PSchElementColor*)pc_QueueSegment->p_vUnitObject; break;}
		case QUEUE_ERASED_ELEMENT:{delete (PSchElementEraser*)pc_QueueSegment->p_vUnitObject; break;}
		case QUEUE_NEW_LINK:{delete (PSchLinkBase*)pc_QueueSegment->p_vUnitObject; break;}
		case QUEUE_CHANGED_LINK:{delete (PSchLinkVars*)pc_QueueSegment->p_vUnitObject; break;}
		case QUEUE_ERASED_LINK:{delete (PSchLinkEraser*)pc_QueueSegment->p_vUnitObject; break;}
		case QUEUE_NEW_GROUP:{delete (PSchGroupBase*)pc_QueueSegment->p_vUnitObject; break;}
		case QUEUE_CHANGED_GROUP:{delete (PSchGroupVars*)pc_QueueSegment->p_vUnitObject; break;}
		case QUEUE_RENAMED_GROUP:{delete (PSchGroupName*)pc_QueueSegment->p_vUnitObject; break;}
		case QUEUE_COLORED_GROUP:{delete (PSchGroupColor*)pc_QueueSegment->p_vUnitObject; break;}
		case QUEUE_ERASED_GROUP:{delete (PSchGroupEraser*)pc_QueueSegment->p_vUnitObject; break;}
		case QUEUE_NEW_BROADCASTER:{delete (PSchBroadcasterBase*)pc_QueueSegment->p_vUnitObject; break;}
		case QUEUE_CHANGED_BROADCASTER:{delete (PSchBroadcasterVars*)pc_QueueSegment->p_vUnitObject; break;}
		case QUEUE_RENAMED_BROADCASTER:{delete (PSchBroadcasterName*)pc_QueueSegment->p_vUnitObject; break;}
		case QUEUE_COLORED_BROADCASTER:{delete (PSchBroadcasterColor*)pc_QueueSegment->p_vUnitObject; break;}
		case QUEUE_ERASED_BROADCASTER:{delete (PSchBroadcasterEraser*)pc_QueueSegment->p_vUnitObject; break;}
		case QUEUE_NEW_RECEIVER:{delete (PSchReceiverBase*)pc_QueueSegment->p_vUnitObject; break;}
		case QUEUE_CHANGED_RECEIVER:{delete (PSchReceiverVars*)pc_QueueSegment->p_vUnitObject; break;}
		case QUEUE_RENAMED_RECEIVER:{delete (PSchReceiverName*)pc_QueueSegment->p_vUnitObject; break;}
		case QUEUE_COLORED_RECEIVER:{delete (PSchReceiverColor*)pc_QueueSegment->p_vUnitObject; break;}
		case QUEUE_ERASED_RECEIVER:{delete (PSchReceiverEraser*)pc_QueueSegment->p_vUnitObject; break;}
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
	ReleasePB(Broadcaster);
	ReleasePB(Receiver);
	delete p_EventsQueue;
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
			oPSchGroupBase.oPSchGroupVars.oSchGraph.oDbObjectFrame.dbX = lstrHelper.at(0).toDouble();
			oPSchGroupBase.oPSchGroupVars.oSchGraph.oDbObjectFrame.dbY = lstrHelper.at(1).toDouble();
			oPSchGroupBase.oPSchGroupVars.oSchGraph.oDbObjectFrame.dbW = lstrHelper.at(2).toDouble();
			oPSchGroupBase.oPSchGroupVars.oSchGraph.oDbObjectFrame.dbH = lstrHelper.at(3).toDouble();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListFrames);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvGroup <<
					m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogFrame << m_chLogNode);
			return false;
		}
		FIND_IN_CHILDLIST(p_NodeGroup, p_ListMinimizes,
						  m_chMinimize, FCN_ONE_LEVEL, p_NodeMinimize)
		{
			p_NodeMinimize = p_NodeMinimize; // Заглушка.
			oPSchGroupBase.oPSchGroupVars.oSchGraph.bMinimized = true;
		} FIND_IN_CHILDLIST_END(p_ListMinimizes);
		FIND_IN_CHILDLIST(p_NodeGroup, p_ListHides,
						  m_chHide, FCN_ONE_LEVEL, p_NodeHide)
		{
			p_NodeHide = p_NodeHide; // Заглушка.
			oPSchGroupBase.oPSchGroupVars.oSchGraph.bHided = true;
		} FIND_IN_CHILDLIST_END(p_ListHides);
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
			oPSchGroupBase.oPSchGroupVars.oSchGraph.dbObjectZPos = strHelper.toDouble();
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
		AppendToPB(Group, new Group(oPSchGroupBase));
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
				if(p_GroupInt->oPSchGroupBase.oPSchGroupVars.ullIDInt == p_Group->oPSchGroupBase.oPSchGroupVars.ullIDGroup)
				{
					p_Group->p_GroupAbove = p_GroupInt;
					p_Group->p_GroupAbove->vp_ConnectedGroups.append(p_Group);
					break;
				}
			}
		}
	}
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
						m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect <<
						m_chLogWrong << m_chLogID << m_chLogNode);
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
						m_chLogEnvFileCorrupt << m_chLogEnvElement << m_chLogEnvNodeFormatIncorrect <<
						m_chLogWrong << m_chLogName << m_chLogNode);
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
			oPSchElementBase.uiObjectBkgColor =
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
			oPSchElementBase.oPSchElementVars.oSchGraph.oDbObjectFrame.dbX = lstrHelper.at(0).toDouble();
			oPSchElementBase.oPSchElementVars.oSchGraph.oDbObjectFrame.dbY = lstrHelper.at(1).toDouble();
			oPSchElementBase.oPSchElementVars.oSchGraph.oDbObjectFrame.dbW = lstrHelper.at(2).toDouble();
			oPSchElementBase.oPSchElementVars.oSchGraph.oDbObjectFrame.dbH = lstrHelper.at(3).toDouble();
			bPresent = true;
		} FIND_IN_CHILDLIST_END(p_ListFrames);
		if(!bPresent)
		{
			LOG_P_0(LOG_CAT_E, m_chLogEnvFileCorrupt << m_chLogEnvElement <<
					m_chLogEnvNodeFormatIncorrect << m_chLogMissing << m_chLogFrame << m_chLogNode);
			return false;
		}
		FIND_IN_CHILDLIST(p_NodeElement, p_ListMinimizes,
						  m_chMinimize, FCN_ONE_LEVEL, p_NodeMinimize)
		{
			p_NodeMinimize = p_NodeMinimize; // Заглушка.
			oPSchElementBase.oPSchElementVars.oSchGraph.bMinimized = true;
		} FIND_IN_CHILDLIST_END(p_ListMinimizes);
		FIND_IN_CHILDLIST(p_NodeElement, p_ListHides,
						  m_chHide, FCN_ONE_LEVEL, p_NodeHide)
		{
			p_NodeHide = p_NodeHide; // Заглушка.
			oPSchElementBase.oPSchElementVars.oSchGraph.bHided = true;
		} FIND_IN_CHILDLIST_END(p_ListHides);
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
			oPSchElementBase.oPSchElementVars.oSchGraph.dbObjectZPos = strHelper.toDouble();
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
		oQColor.setRgba(PBAccess(Group,iF)->oPSchGroupBase.uiObjectBkgColor);
		oQColor.getRgb(&iR, &iG, &iB, &iA);
		p_NodeBkgColor->ToElement()->
				SetText((strHOne.setNum(iR) + "," + strHTwo.setNum(iG) + "," + strHThree.setNum(iB) +
						 "," + strHFour.setNum(iA)).toStdString().c_str());
		p_NodeFrame = p_NodeGroup->InsertEndChild(xmlEnv.NewElement(m_chFrame));
		p_NodeFrame->ToElement()->
				SetText((strHOne.setNum(PBAccess(Group,iF)->oPSchGroupBase.oPSchGroupVars.oSchGraph.oDbObjectFrame.dbX) + "," +
						 strHTwo.setNum(PBAccess(Group,iF)->oPSchGroupBase.oPSchGroupVars.oSchGraph.oDbObjectFrame.dbY) + "," +
						 strHThree.setNum(PBAccess(Group,iF)->oPSchGroupBase.oPSchGroupVars.oSchGraph.oDbObjectFrame.dbW) + "," +
						 strHFour.setNum(PBAccess(Group,iF)->oPSchGroupBase.oPSchGroupVars.oSchGraph.oDbObjectFrame.dbH)).
						toStdString().c_str());
		if(PBAccess(Group,iF)->oPSchGroupBase.oPSchGroupVars.oSchGraph.bMinimized)
		{
			p_NodeGroup->InsertEndChild(xmlEnv.NewElement(m_chMinimize));
		}
		if(PBAccess(Group,iF)->oPSchGroupBase.oPSchGroupVars.oSchGraph.bHided)
		{
			p_NodeGroup->InsertEndChild(xmlEnv.NewElement(m_chHide));
		}
		p_NodeZ = p_NodeGroup->InsertEndChild(xmlEnv.NewElement(m_chZ));
		p_NodeZ->ToElement()->
				SetText(strHOne.setNum(PBAccess(Group,iF)->oPSchGroupBase.oPSchGroupVars.oSchGraph.dbObjectZPos).
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
		oQColor.setRgba(PBAccess(Element,iF)->oPSchElementBase.uiObjectBkgColor);
		oQColor.getRgb(&iR, &iG, &iB, &iA);
		p_NodeBkgColor->ToElement()->
				SetText((strHOne.setNum(iR) + "," + strHTwo.setNum(iG) + "," + strHThree.setNum(iB) + "," +
						 strHFour.setNum(iA)).toStdString().c_str());
		p_NodeFrame = p_NodeElement->InsertEndChild(xmlEnv.NewElement(m_chFrame));
		p_NodeFrame->ToElement()->
				SetText((strHOne.setNum(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.
										oSchGraph.oDbObjectFrame.dbX) + "," +
						 strHTwo.setNum(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.
										oSchGraph.oDbObjectFrame.dbY) + "," +
						 strHThree.setNum(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.
										  oSchGraph.oDbObjectFrame.dbW) + "," +
						 strHFour.setNum(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.
										 oSchGraph.oDbObjectFrame.dbH)).
						toStdString().c_str());
		if(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.oSchGraph.bMinimized)
		{
			p_NodeElement->InsertEndChild(xmlEnv.NewElement(m_chMinimize));
		}
		if(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.oSchGraph.bHided)
		{
			p_NodeElement->InsertEndChild(xmlEnv.NewElement(m_chHide));
		}
		p_NodeZ = p_NodeElement->InsertEndChild(xmlEnv.NewElement(m_chZ));
		p_NodeZ->ToElement()->
				SetText(strHOne.setNum(PBAccess(Element,iF)->oPSchElementBase.oPSchElementVars.oSchGraph.dbObjectZPos).
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

// Прогрузка цепочки событий для подключившегося клиента.
void Environment::FetchEnvToQueue()
{
	TryMutexInit;
	TryMutexLock(ptQueueMutex);
	p_EventsQueue->Clear();
	for(unsigned int iF = 0; iF != PBCount(Group); iF++)
	{
		p_EventsQueue->AddNewGroup(PBAccess(Group,iF)->oPSchGroupBase, QUEUE_TO_CLIENT);
	}
	for(unsigned int iF = 0; iF != PBCount(Element); iF++)
	{
		p_EventsQueue->AddNewElement(PBAccess(Element,iF)->oPSchElementBase, QUEUE_TO_CLIENT);
	}
	for(unsigned int iF = 0; iF != PBCount(Link); iF++)
	{
		p_EventsQueue->AddNewLink(PBAccess(Link,iF)->oPSchLinkBase, QUEUE_TO_CLIENT);
	}
	iLastFetchingSegNumber = (int)EventsQueue::uiCurrentSegNumber - 1; // Если не грузилось ничего, будет статус UPLOAD_STATUS_INACTIVE автом.
	TryMutexUnlock(ptQueueMutex);
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

//(p_Element->oPSchElementBase.oPSchElementVars.oSchGraph.oDbObjectFrame.dbX >
// oPSchReadyFrame.oDbFrame.dbX) &
//(p_Element->oPSchElementBase.oPSchElementVars.oSchGraph.oDbObjectFrame.dbY >
// oPSchReadyFrame.oDbFrame.dbY) &
//(p_Element->oPSchElementBase.oPSchElementVars.oSchGraph.oDbObjectFrame.dbX <
// (oPSchReadyFrame.oDbFrame.dbX +
//  oPSchReadyFrame.oDbFrame.dbW)) &
//(p_Element->oPSchElementBase.oPSchElementVars.oSchGraph.oDbObjectFrame.dbY <
// (oPSchReadyFrame.oDbFrame.dbY +
//  oPSchReadyFrame.oDbFrame.dbH))

// Работа с сетью.
void Environment::NetOperations()
{
	PSchElementBase* p_PSchElementBase;
	PSchElementVars* p_PSchElementVars;
	PSchElementName* p_PSchElementName;
	PSchElementEraser* p_PSchElementEraser;
	PSchElementColor* p_PSchElementColor;
	PSchLinkBase* p_PSchLinkBase;
	PSchLinkVars* p_PSchLinkVars;
	PSchLinkEraser* p_PSchLinkEraser;
	PSchGroupBase* p_PSchGroupBase;
	PSchGroupVars* p_PSchGroupVars;
	PSchGroupName* p_PSchGroupName;
	PSchGroupEraser* p_PSchGroupEraser;
	PSchGroupColor* p_PSchGroupColor;
	//
	bool bPresent = false;
	unsigned short ushNewsQantity = 2;
	//
	if(MainWindow::p_Server->GetConnectionData(0).iStatus != NO_CONNECTION)
	{
		if(MainWindow::p_Server->IsConnectionSecured(0))
		{
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
								p_PSchElementBase = (PSchElementBase*)pc_QueueSegment->p_vUnitObject;
								if(pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT)
								{
									if(ushNewsQantity > 1)
									{
										p_PSchElementBase->oPSchElementVars.bLastInQueue = false;
									}
									else
									{
										p_PSchElementBase->oPSchElementVars.bLastInQueue = true;  // На последней новости.
									}
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
								p_PSchElementVars = (PSchElementVars*)pc_QueueSegment->p_vUnitObject;
								if(pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT)
								{
									if(ushNewsQantity > 1)
									{
										p_PSchElementVars->bLastInQueue = false;
									}
									else
									{
										p_PSchElementVars->bLastInQueue = true; // На последней новости.
									}
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
											if(p_PSchElementVars->oSchGraph.uchChangesBits & SCH_ELEMENT_BIT_ZPOS)
											{
												p_Element->oPSchElementBase.oPSchElementVars.oSchGraph.dbObjectZPos =
														p_PSchElementVars->oSchGraph.dbObjectZPos;
												LOG_P_2(LOG_CAT_I, "Element [" << QString(p_Element->oPSchElementBase.m_chName).toStdString()
														<< "] z-pos is: " <<
														QString::number((int)p_PSchElementVars->oSchGraph.dbObjectZPos).toStdString());
											}
											if(p_PSchElementVars->oSchGraph.uchChangesBits & SCH_ELEMENT_BIT_BUSY)
											{
												p_Element->oPSchElementBase.oPSchElementVars.oSchGraph.bBusy =
														p_PSchElementVars->oSchGraph.bBusy;
												if(p_PSchElementVars->oSchGraph.bBusy)
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
											if(p_PSchElementVars->oSchGraph.uchChangesBits & SCH_ELEMENT_BIT_FRAME)
											{
												p_Element->oPSchElementBase.oPSchElementVars.oSchGraph.oDbObjectFrame =
														p_PSchElementVars->oSchGraph.oDbObjectFrame;
												LOG_P_2(LOG_CAT_I, "Element [" << QString(p_Element->oPSchElementBase.m_chName).toStdString()
																   << "] frame.");
											}
											if(p_PSchElementVars->oSchGraph.uchChangesBits & SCH_ELEMENT_BIT_GROUP)
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
								p_PSchElementName = (PSchElementName*)pc_QueueSegment->p_vUnitObject;
								if(pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT)
								{
									if(ushNewsQantity > 1)
									{
										p_PSchElementName->bLastInQueue = false;
									}
									else
									{
										p_PSchElementName->bLastInQueue = true; // На последней новости.
									}
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
								p_PSchElementColor = (PSchElementColor*)pc_QueueSegment->p_vUnitObject;
								if(pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT)
								{
									if(ushNewsQantity > 1)
									{
										p_PSchElementColor->bLastInQueue = false;
									}
									else
									{
										p_PSchElementColor->bLastInQueue = true; // На последней новости.
									}
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
								p_PSchElementEraser = (PSchElementEraser*)pc_QueueSegment->p_vUnitObject;
								if(pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT)
								{
									if(ushNewsQantity > 1)
									{
										p_PSchElementEraser->bLastInQueue = false;
									}
									else
									{
										p_PSchElementEraser->bLastInQueue = true; // На последней новости.
									}
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
								p_PSchLinkBase = (PSchLinkBase*)pc_QueueSegment->p_vUnitObject;
								if(pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT)
								{
									if(ushNewsQantity > 1)
									{
										p_PSchLinkBase->oPSchLinkVars.bLastInQueue = false;
									}
									else
									{
										p_PSchLinkBase->oPSchLinkVars.bLastInQueue= true;  // На последней новости.
									}
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
								p_PSchLinkVars = (PSchLinkVars*)pc_QueueSegment->p_vUnitObject;
								if(pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT)
								{
									if(ushNewsQantity > 1)
									{
										p_PSchLinkVars->bLastInQueue = false;
									}
									else
									{
										p_PSchLinkVars->bLastInQueue = true;  // На последней новости.
									}
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
											if(p_PSchLinkVars->oSchLinkGraph.uchChangesBits & SCH_LINK_BIT_SCR_PORT_POS)
											{
												p_Link->oPSchLinkBase.oPSchLinkVars.oSchLinkGraph.oDbSrcPortGraphPos =
														p_PSchLinkVars->oSchLinkGraph.oDbSrcPortGraphPos;
												LOG_P_2(LOG_CAT_I, "Link [" <<
														QString(p_Link->p_SrcElement->oPSchElementBase.m_chName).toStdString() << "<>" <<
														QString(p_Link->p_DstElement->oPSchElementBase.m_chName).toStdString()
														<< "] vars - src port position.");
											}
											if(p_PSchLinkVars->oSchLinkGraph.uchChangesBits & SCH_LINK_BIT_DST_PORT_POS)
											{
												p_Link->oPSchLinkBase.oPSchLinkVars.oSchLinkGraph.oDbDstPortGraphPos =
														p_PSchLinkVars->oSchLinkGraph.oDbDstPortGraphPos;
												LOG_P_2(LOG_CAT_I, "Link [" <<
														QString(p_Link->p_SrcElement->oPSchElementBase.m_chName).toStdString() << "<>" <<
														QString(p_Link->p_DstElement->oPSchElementBase.m_chName).toStdString()
														<< "] vars - dst port position.");
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
								p_PSchLinkEraser = (PSchLinkEraser*)pc_QueueSegment->p_vUnitObject;
								if(pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT)
								{
									if(ushNewsQantity > 1)
									{
										p_PSchLinkEraser->bLastInQueue = false;
									}
									else
									{
										p_PSchLinkEraser->bLastInQueue = true;  // На последней новости.
									}
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
											LOG_P_2(LOG_CAT_I, "Link [" << p_Link->p_SrcElement->oPSchElementBase.m_chName << "<>" <<
													p_Link->p_DstElement->oPSchElementBase.m_chName << "] erase.");
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
								p_PSchGroupBase = (PSchGroupBase*)pc_QueueSegment->p_vUnitObject;
								if(pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT)
								{
									if(ushNewsQantity > 1)
									{
										p_PSchGroupBase->oPSchGroupVars.bLastInQueue = false;
									}
									else
									{
										p_PSchGroupBase->oPSchGroupVars.bLastInQueue = true;  // На последней новости.
									}
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
								p_PSchGroupVars = (PSchGroupVars*)pc_QueueSegment->p_vUnitObject;
								if(pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT)
								{
									if(ushNewsQantity > 1)
									{
										p_PSchGroupVars->bLastInQueue = false;
									}
									else
									{
										p_PSchGroupVars->bLastInQueue = true; // На последней новости.
									}
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
											if(p_PSchGroupVars->oSchGraph.uchChangesBits & SCH_GROUP_BIT_FRAME)
											{
												p_Group->oPSchGroupBase.oPSchGroupVars.oSchGraph.oDbObjectFrame =
														p_PSchGroupVars->oSchGraph.oDbObjectFrame;
												LOG_P_2(LOG_CAT_I, "Group [" << QString(p_Group->oPSchGroupBase.m_chName).toStdString()
														<< "] frame.");
											}
											if(p_PSchGroupVars->oSchGraph.uchChangesBits & SCH_GROUP_BIT_ZPOS)
											{
												p_Group->oPSchGroupBase.oPSchGroupVars.oSchGraph.dbObjectZPos =
														p_PSchGroupVars->oSchGraph.dbObjectZPos;
												LOG_P_2(LOG_CAT_I, "Group [" << QString(p_Group->oPSchGroupBase.m_chName).toStdString()
														<< "] z-pos is: " <<
														QString::number((int)p_PSchGroupVars->oSchGraph.dbObjectZPos).toStdString());
											}
											if(p_PSchGroupVars->oSchGraph.uchChangesBits & SCH_GROUP_BIT_BUSY)
											{
												p_Group->oPSchGroupBase.oPSchGroupVars.oSchGraph.bBusy =
														p_PSchGroupVars->oSchGraph.bBusy;
												if(p_PSchGroupVars->oSchGraph.bBusy)
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
											if(p_PSchGroupVars->oSchGraph.uchChangesBits & SCH_GROUP_BIT_GROUP)
											{
												if(p_PSchGroupVars->ullIDGroup == 0) // Обработка отсоединения от группы.
												{
													if(p_Group->p_GroupAbove != nullptr)
													{
														if(p_Group->p_GroupAbove->vp_ConnectedGroups.contains(p_Group))
														{
															p_Group->p_GroupAbove->vp_ConnectedGroups.contains(p_Group);
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
gGGEx:														LOG_P_0(LOG_CAT_E, "Error detaching group from group.");
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
								p_PSchGroupName = (PSchGroupName*)pc_QueueSegment->p_vUnitObject;
								if(pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT)
								{
									if(ushNewsQantity > 1)
									{
										p_PSchGroupName->bLastInQueue = false;
									}
									else
									{
										p_PSchGroupName->bLastInQueue = true; // На последней новости.
									}
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
								p_PSchGroupColor = (PSchGroupColor*)pc_QueueSegment->p_vUnitObject;
								if(pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT)
								{
									if(ushNewsQantity > 1)
									{
										p_PSchGroupColor->bLastInQueue = false;
									}
									else
									{
										p_PSchGroupColor->bLastInQueue = true; // На последней новости.
									}
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
								p_PSchGroupEraser = (PSchGroupEraser*)pc_QueueSegment->p_vUnitObject;
								if(pc_QueueSegment->bDirectionOut == QUEUE_TO_CLIENT)
								{
									if(ushNewsQantity > 1)
									{
										p_PSchGroupEraser->bLastInQueue = false;
									}
									else
									{
										p_PSchGroupEraser->bLastInQueue = true; // На последней новости.
									}
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
gEOK:							break;
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
					LCHECK_BOOL(MainWindow::p_Server->SendBufferToClient(0, true));
				}
				TryMutexUnlock(ptQueueMutex);
			}
		}
	}
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
	if(p_Group->p_GroupAbove != nullptr)
	{
		p_Group->p_GroupAbove->vp_ConnectedGroups.removeOne(p_Group);
	}
	// Удаление группы.
	RemoveObjectFromPBByPointer(Group, p_Group);
}
