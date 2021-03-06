#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

//== ВКЛЮЧЕНИЯ.
#include <QString>
#include "Server/server.h"
#include "logger.h"
#include "parser-ext.h"
#include "main-hub.h"
#include "z-hub-defs.h"
#include "element.h"
#include "link.h"
#include "group.h"
#include "pseudonym.h"
#include "p_buffer.h"

//== МАКРОСЫ.
#define MAX_ELEMENTS				1024
#define MAX_LINKS					4096
#define MAX_GROUPS					512
#define MAX_PSEUDONYMS				256
#define QUEUE_NEW_ELEMENT			1
#define QUEUE_CHANGED_ELEMENT		2
#define QUEUE_RENAMED_ELEMENT		3
#define QUEUE_COLORED_ELEMENT		4
#define QUEUE_ERASED_ELEMENT		5
#define QUEUE_NEW_LINK				6
#define QUEUE_CHANGED_LINK			7
#define QUEUE_ERASED_LINK			8
#define QUEUE_NEW_GROUP				9
#define QUEUE_CHANGED_GROUP			10
#define QUEUE_RENAMED_GROUP			11
#define QUEUE_COLORED_GROUP			12
#define QUEUE_ERASED_GROUP			13
#define QUEUE_SET_PSEUDONYM			14
#define QUEUE_ERASED_PSEUDONYM		15
#define QUEUE_TO_CLIENT				true
#define QUEUE_FROM_CLIENT			false

//== КЛАССЫ.
/// Класс среды.
class Environment
{
public:
	/// Класс очереди событий. Методы вызывать только из-под мьютекса.
	class EventsQueue
	{
	public:
		/// Структура сегмента очереди событий.
		struct QueueSegment
		{
			unsigned int uiNumber; ///< Номер сегмента от старта.
			bool bDirectionOut; ///< true, если от сервера к клиенту.
			unsigned char uchType; ///< Тип юнита.
			void* p_vUnitObject; ///< Указатель на объект юнита.
		};
	public:
		/// Конструктор.
		EventsQueue();
		/// Деструктор.
		~EventsQueue();
		/// Добавление нового элемента.
		static void AddNewElement(PSchElementBase& aPSchElementBase, bool bDirectionOut);
										///< \param[in] aPSchElementBase Ссылка объект базы элемента.
										///< \param[in] true, если от сервера к клиенту.
		/// Добавление изменений элемента.
		static void AddElementChanges(PSchElementVars& aPSchElementVars, bool bDirectionOut);
										///< \param[in] aPSchElementVars Ссылка объект переменных элемента.
										///< \param[in] true, если от сервера к клиенту.
		/// Добавление изменения имени элемента и очистка аналогов в очереди.
		static void AddElementRenameAndFlush(PSchElementName& aPSchElementName, bool bDirectionOut);
										///< \param[in] aPSchElementName Ссылка объект имени элемента.
										///< \param[in] true, если от сервера к клиенту.
		/// Добавление изменения цвета элемента и очистка аналогов в очереди.
		static void AddElementColorAndFlush(PSchElementColor& aPSchElementColor, bool bDirectionOut);
										///< \param[in] aPSchElementName Ссылка объект цвета элемента.
										///< \param[in] true, если от сервера к клиенту.
		/// Добавление удаления элемента.
		static void AddEraseElement(PSchElementEraser& aPSchElementEraser, bool bDirectionOut);
										///< \param[in] aPSchElementEraser Ссылка объект удаления элемента.
										///< \param[in] true, если от сервера к клиенту.
		/// Добавление нового линка.
		static void AddNewLink(PSchLinkBase& aPSchLinkBase, bool bDirectionOut);
										///< \param[in] aPSchLinkBase Ссылка объект базы линка.
										///< \param[in] true, если от сервера к клиенту.
		/// Добавление изменений линка.
		static void AddLinkChanges(PSchLinkVars& aPSchLinkVars, bool bDirectionOut);
										///< \param[in] aPSchLinkVars Ссылка объект переменных линка.
										///< \param[in] true, если от сервера к клиенту.
		/// Добавление удаления линка.
		static void AddEraseLink(PSchLinkEraser& aPSchLinkEraser, bool bDirectionOut);
										///< \param[in] aPSchLinkEraser Ссылка объект удаления элемента.
										///< \param[in] true, если от сервера к клиенту.
		/// Добавление новой группы.
		static void AddNewGroup(PSchGroupBase& aPSchGroupBase, bool bDirectionOut);
										///< \param[in] aPSchGroupBase Ссылка объект базы группы.
										///< \param[in] true, если от сервера к клиенту.
		/// Добавление изменений группы.
		static void AddGroupChanges(PSchGroupVars& aPSchGroupVars, bool bDirectionOut);
										///< \param[in] aPShcGroupVars Ссылка объект переменных группы.
										///< \param[in] true, если от сервера к клиенту.
		/// Добавление изменения имени группы и очистка аналогов в очереди.
		static void AddGroupRenameAndFlush(PSchGroupName& aPSchGroupName, bool bDirectionOut);
										///< \param[in] aPSchGroupName Ссылка объект имени группы.
										///< \param[in] true, если от сервера к клиенту.
		/// Добавление изменения цвета группы и очистка аналогов в очереди.
		static void AddGroupColorAndFlush(PSchGroupColor& aPSchGroupColor, bool bDirectionOut);
										///< \param[in] aPSchGroupName Ссылка объект цвета группы.
										///< \param[in] true, если от сервера к клиенту.
		/// Добавление удаления группы.
		static void AddEraseGroup(PSchGroupEraser& aPSchGroupEraser, bool bDirectionOut);
										///< \param[in] aPSchGroupEraser Ссылка объект удаления группы.
										///< \param[in] true, если от сервера к клиенту.
		/// Добавление установки псевдонима и очистка аналогов в очереди.
		static void AddSetPseudonymAndFlush(PSchPseudonym& aPSchPseudonym, bool bDirectionOut);
										///< \param[in] aPSchPseudonym Ссылка объект псевдонима.
										///< \param[in] true, если от сервера к клиент.
		/// Добавление удаления псевдонима.
		static void AddErasePseudonym(PSchPseudonymEraser& aPSchPseudonymEraser, bool bDirectionOut);
										///< \param[in] aPSchPseudonym Ссылка объект псевдонима.
										///< \param[in] true, если от сервера к клиент
		/// Получение данных из позиции.
		static const QueueSegment* Get(int iNum);
										///< \param[in] iNum Номер позиции.
										///< \return Указатель на структуру с типом позиции и void-указателем.
		/// Очистка и удаление позиции.
		static void Remove(int iNum);
										///< \param[in] iNum Номер позиции.
		/// Очистка цепочки с удалением содержимого.
		static void Clear();
		/// Получение длины цепочки.
		static int Count();
										///< \return Кол-во звеньев цепочки.
	public:
		static unsigned int uiCurrentSegNumber; ///< Текущий номер сегмента.
	private:
		static QList<QueueSegment> l_Queue; ///< Лист очереди событий.
		static QueueSegment oQueueSegment; ///< Служебный объект сегмента.
	};
public:
	/// Конструктор.
	Environment(pthread_mutex_t ptLogMutex, char* p_chEnvName);
										///< \param[in] ptLogMutex Инициализатор мьютекса лога.
										///< \param[in] p_chEnvName Указатель на строку с именем среды.
	/// Деструктор.
	~Environment();
	// Прогрузка элемента и его разновидностей.
	static bool InitElementForEnv(PSchElementBase& a_oPSchElementBase, XMLNode* p_NodeElement, unsigned char uchElementTypeBits);
										///< \param[in] a_oPSchElementBase Ссылка на базу элемента.
										///< \param[in] p_NodeElement Указатель на разъём элемента.
										///< \param[in] uchElementTypeBits Биты для установки типа элемента (обычный, транслятор, приёмник).
										///< \return true при удаче.
	/// Загрузка среды.
	static bool LoadEnv();
										///< \return true при удаче.
	/// Сохранение среды.
	static bool SaveEnv();
										///< \return true при удаче.
	/// Запуск среды.
	static bool Start();
										///< \return true при удаче.
	/// Остановка среды.
	static void Stop();
	/// Проверка инициализированности среды.
	static bool CheckInitialized();
										///< \return true при инициализированной среде.
	/// Удаление линка в позиции и удаление указателя на него.
	static void EraseLinkAt(int iPos);
										///< \param[in] uiPos Позиция в массиве.
	/// Удаление линков для элемента.
	static void EraseLinksForElement(Element* p_Element);
										///< \param[in] p_Element Указатель на элемент.
	// Удаление элемента в позиции и возможных опустевших групп рекурсивно.
	static void EraseElementAt(int iPos);
										///< \param[in] uiPos Позиция в массиве.
	/// Удаление группы в позиции и возможных опустевших групп рекурсивно.
	static void EraseGroupAt(int iPos);
										///< \param[in] uiPos Позиция в массиве.
	/// Удаление группы по указателю и возможных опустевших групп рекурсивно.
	static void EraseGroup(Group* p_Group);
										///< \param[in] p_Group Указатель на группу.
	/// Прогрузка цепочки событий для подключившегося клиента.
	static void FetchEnvToQueue();

private:
	/// Поток шагов среды.
	static void* EnvThread(void*);
										///< \param[in] p_vPlug Заглушка.
										///< \return Заглушка.
	/// Проверка и установка признака последней новости.
	static void CheckLastInQueue(unsigned short ushNewsQantity, bool& abLastInQueue);
										///< \param[in] ushNewsQantity Кол-во новостей за цикл.
										///< \param[in] abLastInQueue Ссылка на признак последней новости.
	/// Работа с сетью.
	static bool NetOperations();
										///< \return false при пустой цепочке новостей.
	/// Рекурсивное удаление детей группы и все их элементы, включая основную.
	static Group* EraseGroupAndChildrenAndElementsRecursively(Group* p_Group, bool bFirst = true);
										///< \param[in] p_Group Указатель на группу.
										///< \param[in] bFirst Служебный флаг первого входа в рекурсию.
										///< \return Указатель на родителя, если был.
	/// Рекурсивное удаление пустой группы и её пустых родителей.
	static void EraseGroupIfEmptyAndParentsRecursively(Group* p_ParentGroup);
										///< \param[in] p_Group Указатель на группу.
public:
	StaticPBHeaderInit(Element,, MAX_ELEMENTS)
	StaticPBHeaderInit(Link,, MAX_LINKS)
	StaticPBHeaderInit(Group,, MAX_GROUPS)
	StaticPBHeaderInit(Pseudonym,, MAX_PSEUDONYMS)
	static bool bEnvThreadAlive; ///< Флаг живого потока среды.
	static bool bStopEnvUpdate; ///< Сигнал на остановку потока среды.
	static bool bRequested; ///< Наличие запроса от клиента.
	static PSchReadyFrame oPSchReadyFrame; ///< Данные по запросу.
	static EventsQueue* p_EventsQueue; ///< Указатель на класс очереди событий.
	static pthread_mutex_t ptQueueMutex; ///< Инициализатор мьютекса отработки очереди событий.
	static int iLastFetchingSegNumber; ///< Номер сегмента следующий за прогруженным при старте.
private:
	LOGDECL
	LOGDECL_PTHRD_INCLASS_ADD
	static char* p_chEnvNameInt; ///< Указатель на внутреннюю строку с именем среды.
	static bool bEnvLoaded; ///< Флаг загруженной среды.
	static QString strEnvPath; ///< Строка для пути среды.
	static QString strEnvFilename; ///< Строка для имени файла среды.
	static pthread_t thrEnv; ///< Идентификатор потока шагов среды.
};

#endif // ENVIRONMENT_H
