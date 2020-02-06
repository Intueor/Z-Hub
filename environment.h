#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

//== ВКЛЮЧЕНИЯ.
#include "logger.h"
#include "Server/server.h"
#include "parser-ext.h"
#include "main-hub.h"
#include "z-hub-defs.h"
#include "element.h"
#include "link.h"
#include "p_buffer.h"

//== МАКРОСЫ.
#define MAX_ELEMENTS			1024
#define MAX_LINKS				4096

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
										///< \return true, при удаче.
	/// Сохранение среды.
	static bool SaveEnv();
										///< \return true, при удаче.
	/// Запуск среды.
	static void Start();
	/// Остановка среды.
	static void Stop();

private:
	LOGDECL
	LOGDECL_PTHRD_INCLASS_ADD
	StaticPBHeaderInit(Element,, MAX_ELEMENTS)
	StaticPBHeaderInit(Link,, MAX_LINKS)
	static char* p_chEnvNameInt; ///< Указатель на внутреннюю строку с именем среды.
};

#endif // ENVIRONMENT_H
