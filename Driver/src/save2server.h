#ifndef __SAVE2SERVER_H__
#define __SAVE2SERVER_H__

enum s2s_result_t: uint8_t { no, ok, skip, no_wifi_shield, no_wifi_connection };

void save2server_setup(void);

s2s_result_t save2server(float temperature, bool fan);

#endif
