//== ВКЛЮЧЕНИЯ.
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
//
char* Environment::p_chEnvNameInt = nullptr;

//== ФУНКЦИИ КЛАССОВ.
//== Класс среды.
// Конструктор.
Environment::Environment(pthread_mutex_t ptLogMutex, char* p_chEnvName)
{
	LOG_CTRL_BIND_EXT_MUTEX(ptLogMutex);
	LOG_CTRL_INIT;
	p_chEnvNameInt = p_chEnvName;
}

// Деструктор.
Environment::~Environment()
{
	ReleasePB(Link);
	ReleasePB(Element);
	LOG_CLOSE;
}

// Загрузка среды.
bool Environment::LoadEnv()
{
	LOG_P_0(LOG_CAT_I, "Loading environment from " << p_chEnvNameInt << ".xml");
	return true;
}

// Сохранение среды.
bool Environment::SaveEnv()
{
	LOG_P_0(LOG_CAT_I, "Saving environment to " << p_chEnvNameInt << ".xml");
	return true;
}

// Запуск среды.
void Environment::Start()
{
	LOG_P_0(LOG_CAT_I, "Start environment.");
}

// Остановка среды.
void Environment::Stop()
{
	LOG_P_0(LOG_CAT_I, "Stop environment.");
}
