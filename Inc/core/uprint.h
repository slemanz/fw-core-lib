#ifndef INC_UPRINT_H_
#define INC_UPRINT_H_

#include <stdarg.h>
#include "interface/interface.h"

void uprint_setup(uint8_t comm_id);
void uprint(const char *fmt, ...);

#endif /* INC_UPRINT_H_ */