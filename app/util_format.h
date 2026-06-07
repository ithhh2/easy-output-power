#ifndef UTIL_FORMAT_H_
#define UTIL_FORMAT_H_
#include <stdint.h>
void format_voltage(char *buf, uint16_t v_x100);
void format_current_ma(char *buf, uint16_t ma);
#endif
