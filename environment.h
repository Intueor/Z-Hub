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
#include "p_buffer.h"

//== МАКРОСЫ.
#define MAX_ELEMENTS				1024
#define MAX_LINKS					4096
#define MAX_GROUPS					512
#define QUEUE_NEW_ELEMENT			1
#define QUEUE_CHANGED_ELEMENT		2
#define QUEUE_RENAMED_ELEMENT		3
#define QUEUE_MINCHANGED_ELEMENT	4
#define QUEUE_ERASED_ELEMENT		5
#define QUEUE_NEW_LINK				6
#define QUEUE_CHANGED_LINK			7
#define QUEUE_NEW_GROUP				8
#define QUEUE_CHANGED_GROUP			9
#define QUEUE_RENAMED_GROUP			10
#define QUEUE_MINCHANGED_GROUP		11
#define QUEUE_ERASED_GROUP			12

//== КЛАССЫ.
/// Класс среды.
class Environment
{
public:
	/// Класс очереди отправки.
	class SendingQueue
	{
	public:
		/// Структура сегмента очереди отправки.
		struct QueueSegment
		{
			unsigned char uchType; ///< Тип юнита.
			void* p_vUnitObject; ///< Указатель на объект юнита.
		};
	public:
		/// Деструктор.
		~SendingQueue();
		/// Добавление нового элемента.
		static void AddNewElement(PSchElementBase& aPSchElementBase);
										///< \param[in] aPSchElementBase Ссылка объект базы элемента.
		/// Добавление изменений элемента.
		static void AddElementChanges(PSchElementVars& aPSchElementVars);
										///< \param[in] aPSchElementVars Ссылка объект переменных элемента.
										///< \param[in] aPSchElementVars Ссылка объект переменных элемента.
		/// Добавление изменения имени элемента и очистка аналогов в очереди.
		static void AddElementRenameAndFlush(PSchElementName& aPSchElementName);
										///< \param[in] aPSchElementName Ссылка объект имени элемента.
		/// Добавление удаления элемента.
		static void AddEraseElement(PSchElementEraser& aPSchElementEraser);
										///< \param[in] aPSchElementEraser Ссылка объект удаления элемента.
		/// Добавление нового линка.
		static void AddNewLink(PSchLinkBase& aPSchLinkBase);
										///< \param[in] aPSchLinkBase Ссылка объект базы линка.
		/// Добавление изменений линка.
		static void AddLinkChanges(PSchLinkVars& aPSchLinkVars);
										///< \param[in] aPSchLinkVars Ссылка объект переменных линка.
		/// Добавление новой группы.
		static void AddNewGroup(PSchGroupBase& aPSchGroupBase);
										///< \param[in] aPSchGroupBase Ссылка объект базы группы.
		/// Добавление изменений группы.
		static void AddGroupChanges(PSchGroupVars& aPSchGroupVars);
										///< \param[in] aPShcGroupVars Ссылка объект переменных группы.
		/// Добавление изменения имени группы и очистка аналогов в очереди.
		static void AddGroupRenameAndFlush(PSchGroupName& aPSchGroupName);
										///< \param[in] aPSchGroupName Ссылка объект имени группы.
		/// Добавление удаления группы.
		static void AddEraseGroup(PSchGroupEraser& aPSchGroupEraser);
										///< \param[in] aPSchGroupEraser Ссылка объект удаления группы.
		/// Получение данных из первой позиции.
		static QueueSegment* GetFirst();
										///< \return Указатель на структуру с типом позиции и void-указателем.
		/// Очистка и удаление первой позиции.
		static void RemoveFirst();
		/// Очистка цепочки с удалением содержимого.
		static void Clear();
		/// Получение длины цепочки.
		static int Count();
										///< \return Кол-во звеньев цепочки.
	private:
		static QList<QueueSegment> l_Queue; ///< Лист очереди отправки.
		static QueueSegment oQueueSegment; ///< Служебный объект сегмента.
	};
public:
	/// Конструктор.
	Environment(pthread_mutex_t ptLogMutex, char* p_chEnvName);
										///< \param[in] ptLogMutex Инициализатор мьютекса лога.
										///< \param[in] p_chEnvName Указатель на строку с именем среды.
	/// Деструктор.
	~Environment();
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
	/// Удаление линка в позиции и обнуление указателя на него.
	static void EraseLinkAt(int iPos);
										///< \param[in] uiPos Позиция в массиве.
	/// Удаление линков для элемента.
	static void EraseLinksForElement(Element* p_Element);
										///< \param[in] p_Element Указатель на элемент.
	/// Удаление элемента в позиции и обнуление указателя на него.
	static void EraseElementAt(int iPos);
										///< \param[in] uiPos Позиция в массиве.
	/// Удаление группы в позиции и обнуление указателя на неё.
	static void EraseGroupAt(int iPos);
										///< \param[in] uiPos Позиция в массиве.
	/// Удаление группы по указателю.
	static void EraseGroup(Group* p_Group);
										///< \param[in] p_Group Указатель на группу.
	/// Прогрузка цепочки отправки для подключившегося клиента.
	static void FetchEnvToQueue();
private:
	/// Удаление элементов из группы.
	static void EraseElementsFromGroup(Group* p_Group);
								///< \param[in] p_Group Указатель на группу.
	/// Удаление групп из группы.
	static void EraseGroupsFromGroup(Group* p_Group);
								///< \param[in] p_Group Указатель на группу.
	/// Поток шагов среды.
	static void* EnvThread(void *p_vPlug);
										///< \param[in] p_vPlug Заглушка.
										///< \return Заглушка.
	/// Работа с сетью.
	static void NetOperations();
public:
	StaticPBHeaderInit(Element,, MAX_ELEMENTS)
	StaticPBHeaderInit(Link,, MAX_LINKS)
	StaticPBHeaderInit(Group,, MAX_GROUPS)
	static bool bEnvThreadAlive; ///< Флаг живого потока среды.
	static bool bStopEnvUpdate; ///< Сигнал на остановку потока среды.
	static bool bRequested; ///< Наличие запроса от клиента.
	static PSchReadyFrame oPSchReadyFrame; ///< Данные по запросу.

private:
	LOGDECL
	LOGDECL_PTHRD_INCLASS_ADD
	static char* p_chEnvNameInt; ///< Указатель на внутреннюю строку с именем среды.
	static bool bEnvLoaded; ///< Флаг загруженной среды.
	static QString strEnvPath; ///< Строка для пути среды.
	static QString strEnvFilename; ///< Строка для имени файла среды.
	static pthread_t thrEnv; ///< Идентификатор потока шагов среды.
	static SendingQueue* p_SendingQueue; ///< Указатель на класс очереди отправки.
};

#endif // ENVIRONMENT_H
