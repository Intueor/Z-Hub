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
#define MAX_ELEMENTS			1024
#define MAX_LINKS				4096
#define MAX_GROUPS				512

//== КЛАССЫ.
/// Класс среды.
class Environment
{
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
	/// Установка всех флагов всех объектов сцены на новые для клиента.
	static void SetAllNew();
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
	/// Проверка линка на актуальность по представленным элементам.
	static bool CheckLinkForAct(Link* p_Link);
										///< \param[in] p_Link Указатель на линк.
										///< \return true - при актуальном линке.
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
	static QList<Element*> lp_NewElements; ///< Лист для заполнения ссылками на новые элементы.
	static QList<Element*> lp_ChangedElements; ///< Лист для заполнения ссылками на изменённые элементы.
	static QList<Element*> lp_RenamedElements; ///< Лист для заполнения ссылками на переименованные элементы.
	static QList<Element*> lp_MinChangedElements; ///< Лист для заполнения ссылками на элементы со сменой свёрнутого состояния.
	static QList<Link*> lp_NewLinks; ///< Лист для заполнения ссылками на новые линки.
	static QList<Link*> lp_ChangedLinks; ///< Лист для заполнения ссылками на изменённые линки.
	static QList<Group*> lp_NewGroups; ///< Лист для заполнения ссылками на новые группы.
	static QList<Group*> lp_ChangedGroups; ///< Лист для заполнения ссылками на изменённые группы.
	static QList<Group*> lp_RenamedGroups; ///< Лист для заполнения ссылками на переименованные группы.
	static QList<Group*> lp_MinChangedGroups; ///< Лист для заполнения ссылками на группы со сменой свёрнутого состояния.
};

#endif // ENVIRONMENT_H
