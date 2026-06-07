#ifndef KEY_APP_H_
#define KEY_APP_H_

#include "hw_key.h"

enum Key_e
{
	SET_KEY = 0,
	ADD_KEY,
	SUB_KEY,
	KEY_NUM
};

void key_app_init(void);
void key_app_poll(void);

#endif /* KEY_APP_H_ */
