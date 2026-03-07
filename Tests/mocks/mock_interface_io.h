#ifndef MOCK_INTERFACE_IO_H_
#define MOCK_INTERFACE_IO_H_

#include <stdint.h>

void mock_io_set_pin_level  (uint8_t pin_id, uint8_t level);
void mock_io_reset_levels   (void);

#endif /* MOCK_INTERFACE_IO_H_ */