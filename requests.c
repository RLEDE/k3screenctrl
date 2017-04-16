#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <errno.h>

#include "serial_port.h"
#include "mcu_proto.h"
#include "frame_tx.h"
#include "common.h"

static int request_send_raw(REQUEST_TYPE type, const void* data, int len) {
    unsigned char *cmdbuf = (unsigned char*)malloc(len + 2);
    if (cmdbuf < 0) {
        syslog(LOG_WARNING, "Could not allocate buffer for new request, drop it: %s\n", strerror(errno));
        return FAILURE;
    }

    bzero(cmdbuf, len + 2);
    cmdbuf[0] = PAYLOAD_HEADER;
    cmdbuf[1] = type;
    memmove(cmdbuf + 2, data, len);
    return frame_send(cmdbuf, len + 2);
}

int request_switch_page(int page) {
    if (page < 1 || page > 5) {
        syslog(LOG_WARNING, "Requested page %d does not exist\n", page);
        return FAILURE;
    }
    return request_send_raw(REQUEST_SWITCH_PAGE, &page, 4);
}

int request_notify_status(STATUS_NOTIF status) {
    return request_send_raw(REQUEST_NOTIFY_STATUS, &status, 4);
}

int request_update_wan(int is_connected, int tx_Bps, int rx_Bps) {
    WAN_INFO waninfo;

    waninfo.is_connected = is_connected;
    waninfo.tx_bytes_per_sec = tx_Bps;
    waninfo.rx_bytes_per_sec = rx_Bps;

    return request_send_raw(REQUEST_UPDATE_WAN, &waninfo, sizeof(waninfo));
}

int request_update_basic_info(const char* prod_name, const char* hw_ver, const char* fw_ver, const char* mac_addr) {
    BASIC_INFO basic_info;
    bzero(&basic_info, sizeof(basic_info));

#define ARRAY_SIZED_STRCPY(dst, src) strncpy((dst), (src), sizeof((dst)));
    ARRAY_SIZED_STRCPY(basic_info.product_name, prod_name);
    ARRAY_SIZED_STRCPY(basic_info.hw_version, hw_ver);
    ARRAY_SIZED_STRCPY(basic_info.fw_version, fw_ver);
    ARRAY_SIZED_STRCPY(basic_info.mac_addr_base, mac_addr);

    return request_send_raw(REQUEST_UPDATE_BASIC_INFO, &basic_info, sizeof(basic_info));
}

/* Too many parameters. Fill the struct yourself */
int request_update_ports(PORT_INFO *port_info) {
    return request_send_raw(REQUEST_UPDATE_PORTS, port_info, sizeof(PORT_INFO));
}
