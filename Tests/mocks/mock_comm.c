#include "interface/interface.h"

void comm_init(uint8_t comm_id)           { (void)comm_id; }
void comm_send(uint8_t comm_id, uint8_t *buf, uint32_t len) { (void)comm_id; (void)buf; (void)len; }
uint8_t comm_receive(uint8_t comm_id, uint8_t *buf, uint32_t len) { (void)comm_id; (void)buf; (void)len; return 0; }
uint8_t comm_data_available(uint8_t comm_id) { (void)comm_id; return 0; }
void comm_deinit(uint8_t comm_id)         { (void)comm_id; }
