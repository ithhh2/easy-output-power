#ifndef UI_APP_H_
#define UI_APP_H_

#include <stdint.h>

void ui_init(void);
void ui_render(void);
void ui_mark_dirty(void);
void ui_display_tick(void);

#endif /* UI_APP_H_ */
