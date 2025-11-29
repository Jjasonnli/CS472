#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>

#include "du-ftp.h"
#include "du-proto.h"

// Changed BUFF size
#define DUFTP_BLOCK_SZ   8192

#define BUFF_SZ (sizeof(duftp_pdu_t) + DUFTP_BLOCK_SZ)

static char sbuffer[BUFF_SZ];
static char rbuffer[BUFF_SZ];
static char full_file_path[FNAME_SZ];
static char requested_file[FNAME_SZ];

/*
 *  Helper function that processes the command line arguements.  Highlights
 *  how to use a very useful utility called getopt, where you pass it a
 *  format string and it does all of the hard work for you.  The arg
 *  string basically states this program accepts a -p or -c flag, the
 *  -p flag is for a "pong message", in other words the server echos
 *  back what the client sends, and a -c message, the -c option takes
 *  a course id, and the server looks up the course id and responds
 *  with an appropriate message. 
 */
static int initParams(int argc, char *argv[], prog_config *cfg){
    int option;
    //setup defaults if no arguements are passed
    static char cmdBuffer[64] = {0};

    //setup defaults if no arguements are passed
    cfg->prog_mode = PROG_MD_CLI;
    cfg->port_number = DEF_PORT_NO;
    strcpy(cfg->file_name, PROG_DEF_FNAME);
    strcpy(cfg->svr_ip_addr, PROG_DEF_SVR_ADDR);
    
    while ((option = getopt(argc, argv, ":p:f:a:csh")) != -1){
        switch(option) {
            case 'p':
                strncpy(cmdBuffer, optarg, sizeof(cmdBuffer));
                cfg->port_number = atoi(cmdBuffer);
                break;
            case 'f':
                strncpy(cfg->file_name, optarg, sizeof(cfg->file_name));
                break;
            case 'a':
                strncpy(cfg->svr_ip_addr, optarg, sizeof(cfg->svr_ip_addr));
                break;
            case 'c':
                cfg->prog_mode = PROG_MD_CLI;
                break;
            case 's':
                cfg->prog_mode = PROG_MD_SVR;
                break;
            case 'h':
                printf("USAGE: %s [-p port] [-f fname] [-a svr_addr] [-s] [-c] [-h]\n", argv[0]);
                printf("WHERE:\n\t[-c] runs in client mode, [-s] runs in server mode; DEFAULT= client_mode\n");
                printf("\t[-a svr_addr] specifies the servers IP address as a string; DEFAULT = %s\n", cfg->svr_ip_addr);
                printf("\t[-p portnum] specifies the port number; DEFAULT = %d\n", cfg->port_number);
                printf("\t[-f fname] specifies the filename to send or recv; DEFAULT = %s\n", cfg->file_name);
                printf("\t[-p] displays what you are looking at now - the help\n\n");
                exit(0);
            case ':':
                perror ("Option missing value");
                exit(-1);
            default:
            case '?':
                perror ("Unknown option");
                exit(-1);
        }
    }
    return cfg->prog_mode;
}


static int duftp_send_data_block(dp_connp dpc, const void *data, int len) {
    if (len < 0 || len > DUFTP_BLOCK_SZ) {
        return DP_ERROR_GENERAL;
    }

    char sendBuf[sizeof(duftp_pdu_t) + DUFTP_BLOCK_SZ];

    duftp_pdu_t hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.proto_ver = DUFTP_PROTO_VER;
    hdr.msg_type  = DUFTP_MT_DATA;
    hdr.status    = DUFTP_STATUS_OK;
    hdr.data_len  = (uint32_t)len;

    memcpy(sendBuf, &hdr, sizeof(hdr));
    if (len > 0) {
        memcpy(sendBuf + sizeof(hdr), data, len);
    }

    int totalLen = sizeof(hdr) + len;
    int rc = dpsend(dpc, sendBuf, totalLen);
    if (rc < 0) {
        return rc;
    }
    if (rc != totalLen) {
        fprintf(stderr,
                "duftp_send_data_block: short send %d of %d\n",
                rc, totalLen);
        return DP_ERROR_GENERAL;
    }

    return len;
}

// Receive any du-ftp PDU; copy header out and up to buff_sz of data 
static int duftp_recv_pdu(dp_connp dpc,
                          duftp_pdu_t *hdr_out,
                          void *buff,
                          int buff_sz)
{
    char recvBuf[sizeof(duftp_pdu_t) + DUFTP_BLOCK_SZ];
    int rc = dprecv(dpc, recvBuf, sizeof(recvBuf));

    if (rc == DP_CONNECTION_CLOSED) {
        return DP_CONNECTION_CLOSED;
    }
    if (rc < 0) {
        return rc;
    }
    if (rc < (int)sizeof(duftp_pdu_t)) {
        fprintf(stderr,
                "duftp_recv_pdu: too few bytes for header (%d)\n", rc);
        return DP_ERROR_PROTOCOL;
    }

    memcpy(hdr_out, recvBuf, sizeof(duftp_pdu_t));

    if (hdr_out->proto_ver != DUFTP_PROTO_VER) {
        fprintf(stderr,
                "duftp_recv_pdu: bad proto version %d\n",
                hdr_out->proto_ver);
        return DP_ERROR_PROTOCOL;
    }

    int data_len = (int)hdr_out->data_len;

    if (data_len > 0) {
        if (data_len > buff_sz) {
            fprintf(stderr,
                    "duftp_recv_pdu: buffer too small for %d bytes\n",
                    data_len);
            return DP_BUFF_UNDERSIZED;
        }
        if ((int)(sizeof(duftp_pdu_t) + data_len) > rc) {
            fprintf(stderr,
                    "duftp_recv_pdu: truncated payload (need %lu, got %d)\n",
                    (unsigned long)(sizeof(duftp_pdu_t) + data_len),
                    rc);
            return DP_ERROR_PROTOCOL;
        }
        memcpy(buff,
               recvBuf + sizeof(duftp_pdu_t),
               data_len);
    }

    return data_len;
}

// Send an OPEN_REQ (client -> server)
static int duftp_send_open_req(dp_connp dpc, const char *filename) {
    duftp_pdu_t hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.proto_ver = DUFTP_PROTO_VER;
    hdr.msg_type  = DUFTP_MT_OPEN_REQ;
    hdr.status    = DUFTP_STATUS_OK;
    hdr.data_len  = 0;
    if (filename != NULL) {
        strncpy(hdr.filename, filename, DUFTP_MAX_FILENAME - 1);
    } 

    int rc = dpsend(dpc, &hdr, sizeof(hdr));
    if (rc != (int)sizeof(hdr)) {
        fprintf(stderr,
                "duftp_send_open_req: send %d, expected %zu\n",
                rc, sizeof(hdr));
        return DP_ERROR_GENERAL;
    }
    return 0;
}

static int duftp_send_open_resp(dp_connp dpc, duftp_status_t status) {
    duftp_pdu_t hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.proto_ver = DUFTP_PROTO_VER;
    hdr.msg_type  = DUFTP_MT_OPEN_RESP;
    hdr.status    = status;
    hdr.data_len  = 0;

    int rc = dpsend(dpc, &hdr, sizeof(hdr));
    if (rc != (int)sizeof(hdr)) {
        fprintf(stderr,
                "duftp_send_open_resp: send %d, expected %zu\n",
                rc, sizeof(hdr));
        return DP_ERROR_GENERAL;
    }
    return 0;
}

// Send CLOSE_REQ or CLOSE_RESP
static int duftp_send_close(dp_connp dpc, bool is_resp, duftp_status_t status) {
    duftp_pdu_t hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.proto_ver = DUFTP_PROTO_VER;
    hdr.msg_type  = is_resp ? DUFTP_MT_CLOSE_RESP : DUFTP_MT_CLOSE_REQ;
    hdr.status    = status;
    hdr.data_len  = 0;

    int rc = dpsend(dpc, &hdr, sizeof(hdr));
    if (rc != (int)sizeof(hdr)) {
        fprintf(stderr,
                "duftp_send_close: send %d, expected %zu\n",
                rc, sizeof(hdr));
        return DP_ERROR_GENERAL;
    }
    return 0;
}

// SERVER SIDE

int server_loop(dp_connp dpc, void *sBuff, void *rBuff, int sbuff_sz, int rbuff_sz){
    (void)sBuff;   // unused in this version
    (void)sbuff_sz;

    if (dpc->isConnected == false){
        perror("Expecting the protocol to be in connect state, but its not");
        exit(-1);
    }

    // Receive OPEN_REQ with filename 
    duftp_pdu_t hdr;
    int rc = duftp_recv_pdu(dpc, &hdr, rBuff, rbuff_sz);
    if (rc < 0) {
        fprintf(stderr, "server_loop: failed to receive OPEN_REQ (%d)\n", rc);
        return rc;
    }

    if (hdr.msg_type != DUFTP_MT_OPEN_REQ) {
        fprintf(stderr,
                "server_loop: expected OPEN_REQ, got msg_type=%d\n",
                hdr.msg_type);
        return DP_ERROR_PROTOCOL;
    }

    // Build full_file_path from filename in the PDU 
    if (hdr.filename[0] == '\0') {
        fprintf(stderr, "server_loop: empty filename in OPEN_REQ\n");
        duftp_send_open_resp(dpc, DUFTP_STATUS_INVALID_REQUEST);
        return DP_ERROR_PROTOCOL;
    }

    snprintf(full_file_path, sizeof(full_file_path),
             "./infile/%s", hdr.filename);

    FILE *f = fopen(full_file_path, "wb+");
    if(f == NULL){
        printf("ERROR:  Cannot open file %s\n", full_file_path);
        duftp_send_open_resp(dpc, DUFTP_STATUS_IO_ERROR);
        return DUFTP_STATUS_IO_ERROR;
    }

    // good to go
    duftp_send_open_resp(dpc, DUFTP_STATUS_OK);

    // Receive DATA blocks until CLOSE_REQ 
    while(1) {
        int data_len = duftp_recv_pdu(dpc, &hdr, rBuff, rbuff_sz);

        if (data_len == DP_CONNECTION_CLOSED) {
            printf("Client closed connection\n");
            fclose(f);
            return DP_CONNECTION_CLOSED;
        }

        if (data_len < 0) {
            fprintf(stderr, "server_loop: error %d during data receive\n", data_len);
            fclose(f);
            return data_len;
        }

        if (hdr.msg_type == DUFTP_MT_CLOSE_REQ) {
            // End of transfer
            duftp_send_close(dpc, true, DUFTP_STATUS_OK);
            printf("Server: received CLOSE_REQ, finishing file %s\n",
                   full_file_path);
            break;
        }

        if (hdr.msg_type != DUFTP_MT_DATA) {
            fprintf(stderr,
                    "server_loop: unexpected msg_type=%d\n",
                    hdr.msg_type);
            fclose(f);
            return DP_ERROR_PROTOCOL;
        }

        if (data_len > 0) {
            fwrite(rBuff, 1, data_len, f);
            int printLen = data_len > 50 ? 50 : data_len;
            printf("========================> \n%.*s\n========================> \n", 
                   printLen, (char *)rBuff);
        }
    }

    fclose(f);

    // closes the connection
    while (1) {
        int rc2 = dprecv(dpc, rBuff, rbuff_sz);
        if (rc2 == DP_CONNECTION_CLOSED) {
            printf("Server: transport connection closed\n");
            break;
        }
        if (rc2 < 0) {
            fprintf(stderr,
                    "Server: error while waiting for dpdisconnect (%d)\n",
                    rc2);
            break;
        }
        
    }
    return 0;
}

// CLIENT SIDE

void start_client(dp_connp dpc){
    if(!dpc->isConnected) {
        printf("Client not connected\n");
        return;
    }

    FILE *f = fopen(full_file_path, "rb");
    if(f == NULL){
        printf("ERROR:  Cannot open file %s\n", full_file_path);
        exit(-1);
    }


    int rc = duftp_send_open_req(dpc, requested_file);
    if (rc < 0) {
        fprintf(stderr, "Client: failed to send OPEN_REQ (%d)\n", rc);
        fclose(f);
        return;
    }

    // Wait for OPEN_RESP
    duftp_pdu_t hdr;
    rc = duftp_recv_pdu(dpc, &hdr, rbuffer, sizeof(rbuffer));
    if (rc < 0) {
        fprintf(stderr, "Client: failed to receive OPEN_RESP (%d)\n", rc);
        fclose(f);
        return;
    }

    if (hdr.msg_type != DUFTP_MT_OPEN_RESP) {
        fprintf(stderr, "Client: expected OPEN_RESP, got msg_type=%d\n",
                hdr.msg_type);
        fclose(f);
        return;
    }

    if (hdr.status != DUFTP_STATUS_OK) {
        fprintf(stderr, "Client: server refused open, status=%u\n", hdr.status);
        fclose(f);
        return;
    }

    // Stream file in large DATA blocks
    while (1) {
        size_t bytes = fread(sbuffer, 1, DUFTP_BLOCK_SZ, f);
        if (bytes > 0) {
            rc = duftp_send_data_block(dpc, sbuffer, (int)bytes);
            if (rc < 0) {
                fprintf(stderr, "Client: error sending data (%d)\n", rc);
                fclose(f);
                return;
            }
        }

        if (bytes < DUFTP_BLOCK_SZ) {
            if (feof(f)) {
                break;  // done
            }
            if (ferror(f)) {
                perror("Client: fread error");
                fclose(f);
                return;
            }
        }
    }

    fclose(f);

    // Send CLOSE_REQ and wait for CLOSE_RESP 
    duftp_send_close(dpc, false, DUFTP_STATUS_OK);

    rc = duftp_recv_pdu(dpc, &hdr, rbuffer, sizeof(rbuffer));
    if (rc >= 0 && hdr.msg_type == DUFTP_MT_CLOSE_RESP) {
        printf("Client: transfer closed cleanly by server\n");
    } else {
        printf("Client: did not receive proper CLOSE_RESP\n");
    }

    dpdisconnect(dpc);
}

void start_server(dp_connp dpc){
    server_loop(dpc, sbuffer, rbuffer, sizeof(sbuffer), sizeof(rbuffer));
}


int main(int argc, char *argv[])
{
    prog_config cfg;
    int cmd;
    dp_connp dpc;
    int rc;

    //Process the parameters and init the header
    cmd = initParams(argc, argv, &cfg);

    strncpy(requested_file, cfg.file_name, sizeof(requested_file));
    requested_file[sizeof(requested_file) - 1] = '\0';

    printf("MODE %d\n", cfg.prog_mode);
    printf("PORT %d\n", cfg.port_number);
    printf("FILE NAME: %s\n", cfg.file_name);

    switch(cmd){
        case PROG_MD_CLI:
            // by default client will look for files in the ./outfile directory
            snprintf(full_file_path, sizeof(full_file_path),
                     "./outfile/%s", cfg.file_name);

            dpc = dpClientInit(cfg.svr_ip_addr,cfg.port_number);
            rc = dpconnect(dpc);
            if (rc < 0) {
                perror("Error establishing connection");
                exit(-1);
            }

            start_client(dpc);
            exit(0);
            break;

        case PROG_MD_SVR:
            // Server now uses filename from client's OPEN_REQ;
            // full_file_path is built in server_loop from hdr.filename.
            dpc = dpServerInit(cfg.port_number);
            rc = dplisten(dpc);
            if (rc < 0) {
                perror("Error establishing connection");
                exit(-1);
            }

            start_server(dpc);
            break;

        default:
            printf("ERROR: Unknown Program Mode.  Mode set is %d\n", cmd);
            break;
    }
}
