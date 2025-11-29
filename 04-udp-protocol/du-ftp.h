#pragma once

#define PROG_MD_CLI     0
#define PROG_MD_SVR     1
#define DEF_PORT_NO     2080
#define FNAME_SZ        150
#define PROG_DEF_FNAME  "test.c"
#define PROG_DEF_SVR_ADDR   "127.0.0.1"

typedef struct prog_config{
    int     prog_mode;
    int     port_number;
    char    svr_ip_addr[16];
    char    file_name[128];
} prog_config;

#ifndef DU_FTP_H
#define DU_FTP_H

#include <stdint.h>

#define DUFTP_PROTO_VER          1
#define DUFTP_MAX_FILENAME       256

// message types
typedef enum {
    DUFTP_MT_OPEN_REQ = 1,   /* client -> server: open file (GET / PUT) */
    DUFTP_MT_OPEN_RESP,      /* server -> client: result of open */
    DUFTP_MT_DATA,           /* data chunk */
    DUFTP_MT_DATA_ACK,       /* optional: ack for data chunk */
    DUFTP_MT_CLOSE_REQ,      /* request to close transfer */
    DUFTP_MT_CLOSE_RESP,     /* response / final status */
    DUFTP_MT_ERROR           /* async error */
} duftp_msg_type_t;

// status 
typedef enum {
    DUFTP_STATUS_OK = 0,
    DUFTP_STATUS_FILE_NOT_FOUND,
    DUFTP_STATUS_PERMISSION_DENIED,
    DUFTP_STATUS_IO_ERROR,
    DUFTP_STATUS_INVALID_REQUEST,
    DUFTP_STATUS_INTERNAL_ERROR
} duftp_status_t;


typedef struct {
    uint8_t           proto_ver;  
    uint8_t           msg_type;         
    uint16_t          reserved;        

    uint32_t          status;                  
    uint32_t          data_len;               

    char              filename[DUFTP_MAX_FILENAME]; 
} duftp_pdu_t;

#endif 
