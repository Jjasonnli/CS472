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

// Message types
#define DUFTP_MT_OPEN    1   // client -> server: request to open file for write
#define DUFTP_MT_OPEN_ACK 2  // server -> client: open result (OK or error)
#define DUFTP_MT_DATA    4   // client -> server: file data chunk
#define DUFTP_MT_CLOSE   8   // client -> server: finished sending file
#define DUFTP_MT_ERROR   16  // server -> client: error message

// Status codes for OPEN_ACK / ERROR
#define DUFTP_STATUS_OK          0
#define DUFTP_STATUS_NOT_FOUND  -1
#define DUFTP_STATUS_IO_ERROR   -2

typedef struct duftp_pdu {
    int  msg_type;              
    int  status;                
    int  data_len;              // number of valid data bytes that follow header
    char filename[FNAME_SZ];    // used for OPEN / ERROR
} duftp_pdu;