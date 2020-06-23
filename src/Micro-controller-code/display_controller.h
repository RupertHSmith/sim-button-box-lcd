#include "DisplayDataPacket.h"


void init_display_controller(void);
void init_display_laptimes(void);

void receive_packet(const uint8_t* ReportData);

void process_ac_tyre_data(const uint8_t* ReportData);
void process_ac_data(const uint8_t* ReportData);