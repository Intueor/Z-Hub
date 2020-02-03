#ifndef NETHUBDEFS_H
#define NETHUBDEFS_H

//== МАКРОСЫ.
#define MSG_STR_LEN				128
#define SCH_OBJ_NAME_STR_LEN    64
#define WORLD_FILENAME_STR_LEN	64
#define _NMG					-32768		// !!! Текущий свободный номер _NMG-9 !!!
#define PROTOCOL_CODE			314159265358
#define SERVER_NAME_STR_LEN		64
#define AUTH_PASSWORD_STR_LEN	64
#define INTERFACE_RESPONSE_MS       10
#define USER_RESPONSE_MS            100
#define WAITING_FOR_CLIENT_DSC      1000
#define WAITING_FOR_SRV_STEP		1000
#define MAX_CONN                    2
#define NO_CONNECTION               _NMG-5 // См. protocol.h для занятия нового свободного номера.
#define S_MAX_STORED_POCKETS		64
#define C_MAX_STORED_POCKETS		64
#define PTHREAD_TRYLOCK_ATTEMPTS	50
#define PTHREAD_TRYLOCK_TIMESTEP	50
#define MAX_DATA					8192
#define PORT_STR_LEN				6
#define IP_STR_LEN                  40
#define BUFFER_IS_FULL				_NMG-1 // См. protocol.h для занятия нового свободного номера.
#define RETURN_THREAD				pthread_exit(0); return 0;
#define SizeOfChars(num)			(sizeof(char) * num)
#define	DATA_ACCESS_ERROR			_NMG-2 // См. protocol.h для занятия нового свободного номера.
#define	BUFFER_IS_EMPTY				_NMG-3 // См. protocol.h для занятия нового свободного номера.
#define	DATA_NOT_FOUND				_NMG-8 // См. protocol.h для занятия нового свободного номера.

#endif // NETHUBDEFS_H
