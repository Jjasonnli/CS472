/**
 * =============================================================================
 * STUDENT ASSIGNMENT: CRYPTO-CLIENT.C
 * =============================================================================
 * 
 * ASSIGNMENT OBJECTIVE:
 * Implement a TCP client that communicates with a server using an encrypted
 * protocol. Your focus is on socket programming and network communication.
 * The cryptographic functions are provided for you in crypto-lib.
 * 
 * =============================================================================
 * WHAT YOU NEED TO IMPLEMENT:
 * =============================================================================
 * 
 * 1. SOCKET CONNECTION (start_client function):
 *    - Create a TCP socket using socket()
 *    - Configure the server address structure (struct sockaddr_in)
 *    - Connect to the server using connect()
 *    - Handle connection errors appropriately
 *    - Call your communication loop function
 *    - Close the socket when done
 * 
 * 2. CLIENT COMMUNICATION LOOP:
 *    - Create a function that handles the request/response cycle
 *    - Allocate buffers for sending and receiving data
 *    - Maintain a session key (crypto_key_t) for encryption
 *    - Loop to:
 *      a) Get user command using get_command() (provided below)
 *      b) Build a PDU (Protocol Data Unit) from the command
 *      c) Send the PDU to the server using send()
 *      d) Receive the server's response using recv()
 *      e) Process the response (extract key, decrypt data, etc.)
 *      f) Handle exit commands and connection closures
 *    - Free allocated buffers before returning
 * 
 * 3. PDU CONSTRUCTION:
 *    - Consider creating a helper function to build PDUs
 *    - Fill in the PDU header (msg_type, direction, payload_len)
 *    - For MSG_DATA: copy plaintext to payload
 *    - For MSG_ENCRYPTED_DATA: use encrypt_string() to encrypt before copying
 *    - For MSG_KEY_EXCHANGE: no payload needed
 *    - For command messages: no payload needed
 *    - Return the total PDU size (header + payload)
 * 
 * =============================================================================
 * ONE APPROACH TO SOLVE THIS PROBLEM:
 * =============================================================================
 * 
 * FUNCTION STRUCTURE:
 * 
 * void start_client(const char* addr, int port) {
 *     // 1. Create TCP socket
 *     // 2. Configure server address (sockaddr_in)
 *     // 3. Connect to server
 *     // 4. Print connection confirmation
 *     // 5. Call your communication loop function
 *     // 6. Close socket
 *     // 7. Print disconnection message
 * }
 * 
 * int client_loop(int socket_fd) {
 *     // 1. Allocate buffers (send, receive, input)
 *     // 2. Initialize session_key to NULL_CRYPTO_KEY
 *     // 3. Loop:
 *     //    a) Call get_command() to get user input
 *     //    b) Build PDU from command (use helper function)
 *     //    c) Send PDU using send()
 *     //    d) If exit command, break after sending
 *     //    e) Receive response using recv()
 *     //    f) Handle recv() return values (0 = closed, <0 = error)
 *     //    g) Process response:
 *     //       - If MSG_KEY_EXCHANGE: extract key from payload
 *     //       - If MSG_ENCRYPTED_DATA: decrypt using decrypt_string()
 *     //       - Print results
 *     //    h) Loop back
 *     // 4. Free buffers
 *     // 5. Return success/error code
 * }
 * 
 * int build_packet(const msg_cmd_t *cmd, crypto_msg_t *pdu, crypto_key_t key) {
 *     // 1. Set pdu->header.msg_type = cmd->cmd_id
 *     // 2. Set pdu->header.direction = DIR_REQUEST
 *     // 3. Based on cmd->cmd_id:
 *     //    - MSG_DATA: copy cmd->cmd_line to payload, set length
 *     //    - MSG_ENCRYPTED_DATA: encrypt cmd->cmd_line, set length
 *     //    - MSG_KEY_EXCHANGE: set length to 0
 *     //    - Command messages: set length to 0
 *     // 4. Return sizeof(crypto_pdu_t) + payload_len
 * }
 * 
 * =============================================================================
 * IMPORTANT PROTOCOL DETAILS:
 * =============================================================================
 * 
 * PDU STRUCTURE:
 *   typedef struct crypto_pdu {
 *       uint8_t  msg_type;      // MSG_DATA, MSG_ENCRYPTED_DATA, etc.
 *       uint8_t  direction;     // DIR_REQUEST or DIR_RESPONSE
 *       uint16_t payload_len;   // Length of payload in bytes
 *   } crypto_pdu_t;
 * 
 *   typedef struct crypto_msg {
 *       crypto_pdu_t header;
 *       uint8_t      payload[]; // Flexible array
 *   } crypto_msg_t;
 * 
 * MESSAGE TYPES (from protocol.h):
 *   MSG_KEY_EXCHANGE     - Request/send encryption key
 *   MSG_DATA             - Plain text message
 *   MSG_ENCRYPTED_DATA   - Encrypted message (requires session key)
 *   MSG_CMD_CLIENT_STOP  - Client exit command
 *   MSG_CMD_SERVER_STOP  - Server shutdown command
 * 
 * TYPICAL MESSAGE FLOW:
 *   1. Client sends MSG_KEY_EXCHANGE request
 *   2. Server responds with MSG_KEY_EXCHANGE + key in payload
 *   3. Client extracts key: memcpy(&session_key, response->payload, sizeof(crypto_key_t))
 *   4. Client can now send MSG_ENCRYPTED_DATA
 *   5. Server responds with MSG_ENCRYPTED_DATA
 *   6. Client decrypts using decrypt_string()
 * 
 * =============================================================================
 * CRYPTO LIBRARY FUNCTIONS YOU'LL USE:
 * =============================================================================
 * 
 * int encrypt_string(crypto_key_t key, uint8_t *out, uint8_t *in, size_t len)
 *   - Encrypts a string before sending
 *   - Returns number of encrypted bytes or negative on error
 * 
 * int decrypt_string(crypto_key_t key, uint8_t *out, uint8_t *in, size_t len)
 *   - Decrypts received data
 *   - Returns number of decrypted chars or negative on error
 *   - NOTE: Output is NOT null-terminated, you must add '\0'
 * 
 * void print_msg_info(crypto_msg_t *msg, crypto_key_t key, int mode)
 *   - Prints PDU details for debugging
 *   - Use CLIENT_MODE for the mode parameter
 *   - VERY helpful for debugging your protocol!
 * 
 * =============================================================================
 * DEBUGGING TIPS:
 * =============================================================================
 * 
 * 1. Use print_msg_info() before sending and after receiving
 * 2. Check return values from ALL socket operations
 * 3. Verify payload_len matches actual data length
 * 4. Remember: recv() may return less bytes than expected
 * 5. Encrypted data requires a valid session key (check for NULL_CRYPTO_KEY)
 * 6. Use printf() liberally to trace program flow
 * 
 * =============================================================================
 * TESTING RECOMMENDATIONS:
 * =============================================================================
 * 
 * 1. Start simple: Get plain MSG_DATA working first
 * 2. Test key exchange: Send '#' command
 * 3. Test encryption: Send '!message' after key exchange
 * 4. Test exit commands: '-' for client exit, '=' for server shutdown
 * 5. Test error cases: What if server closes unexpectedly?
 * 
 * Good luck! Remember: Focus on the socket operations. The crypto is done!
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdint.h>
#include "crypto-client.h"
#include "crypto-lib.h"
#include "protocol.h"
// Added this below
#include <errno.h>

// Helper function to check if a key is null
static int is_null_key(crypto_key_t k) { return (k == NULL_CRYPTO_KEY); }

// Helper to send all bytes over socket
static ssize_t send_all(int fd, const void *buf, size_t len) {
    const uint8_t *p = (const uint8_t*)buf;
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = send(fd, p + sent, len - sent, 0);
        if (n < 0) { if (errno == EINTR) continue; return -1; }
        if (n == 0) break;
        sent += (size_t)n;
    }
    return (ssize_t)sent;
}

// Helper to recieve the exact (len) bytes from a socket
static ssize_t recv_all(int fd, void *buf, size_t len) {
    uint8_t *p = (uint8_t*)buf;
    size_t got = 0;
    while (got < len) {
        ssize_t n = recv(fd, p + got, len - got, 0);
        if (n < 0) { if (errno == EINTR) continue; return -1; }
        if (n == 0) return 0; /* peer closed */
        got += (size_t)n;
    }
    return (ssize_t)got;
}

// Reads a full crypto_msg_t from socket
// Allocates the correct amount of memory
static int recv_message(int fd, crypto_msg_t **out_msg, size_t *out_sz) {
    if (!out_msg || !out_sz) return -1;
    *out_msg = NULL; *out_sz = 0;

    // Reads header
    uint8_t hdr[4];
    ssize_t h = recv_all(fd, hdr, sizeof(hdr));
    if (h == 0) return 0;
    if (h < 0)  return -2;
    if (h != 4) return -3;

    // Convert header bytes into a crypto_pdu_t
    crypto_pdu_t pdu;
    pdu.msg_type  = hdr[0];
    pdu.direction = hdr[1];
    uint16_t nlen; memcpy(&nlen, &hdr[2], sizeof(uint16_t));
    pdu.payload_len = ntohs(nlen);

    // Allocate size of message
    size_t total = sizeof(crypto_msg_t) + pdu.payload_len;
    crypto_msg_t *msg = (crypto_msg_t*)malloc(total);
    if (!msg) return -4;

    msg->header = pdu; 
    // Reads the payload if it exists
    if (pdu.payload_len > 0) {
        ssize_t r = recv_all(fd, msg->payload, pdu.payload_len);
        if (r == 0) { free(msg); return 0; }
        if (r < 0 || (size_t)r != pdu.payload_len) { free(msg); return -5; }
    }

    *out_msg = msg; *out_sz = total;
    return 1;
}


static int build_packet(const msg_cmd_t *cmd, crypto_pdu_t *hdr, uint8_t *payload, crypto_key_t key) {
    if (!cmd || !hdr) return -1;

    hdr->msg_type  = (uint8_t)cmd->cmd_id;
    hdr->direction = DIR_REQUEST;
    hdr->payload_len = 0;

    switch (cmd->cmd_id) {

        // Plain text message
        case MSG_DATA: {
            if (!cmd->cmd_line) { hdr->payload_len = 0; return 0; }
            size_t len = strnlen(cmd->cmd_line, MAX_MSG_DATA_SIZE);
            if (len > MAX_MSG_DATA_SIZE) return -2;
            if (payload && len) memcpy(payload, cmd->cmd_line, len);
            hdr->payload_len = (uint16_t)len;
            return (int)len;
        }


        // Encrypted Message, you need a session key
        case MSG_ENCRYPTED_DATA: {
            if (!cmd->cmd_line) return -3;
            if (is_null_key(key)) return -4; // need session key first 
            size_t len = strnlen(cmd->cmd_line, MAX_MSG_DATA_SIZE);
            if (len > MAX_MSG_DATA_SIZE) return -2;
            if (!payload && len) return -5;
            int enc = encrypt_string(key, payload, (uint8_t*)cmd->cmd_line, len);
            if (enc < 0) return enc;
            hdr->payload_len = (uint16_t)enc;
            return enc;
        }

        // These cases don't require a payload
        case MSG_KEY_EXCHANGE:
        case MSG_CMD_CLIENT_STOP:
        case MSG_CMD_SERVER_STOP:
        case MSG_HELP_CMD:
            hdr->payload_len = 0;
            return 0;

        default:
            // Unknown/random commands
            return -6;
    }
}

// Handles the entire client interaction after connection
static int client_loop(int sockfd) {
    char input[MAX_MSG_DATA_SIZE];
    msg_cmd_t cmd;
    crypto_key_t session_key = NULL_CRYPTO_KEY;

    uint8_t payload[MAX_MSG_DATA_SIZE];

    for (;;) {
        // Get user input
        int gr = get_command(input, sizeof(input), &cmd);
        if (gr == CMD_NO_EXEC) continue;

        // Builds message
        crypto_pdu_t hdr;
        int plen = build_packet(&cmd, &hdr, payload, session_key);
        if (plen < 0) {
            printf("[ERROR] No session key established. Cannot send encrypted data.\n\n", plen);
            if (cmd.cmd_id == MSG_CMD_CLIENT_STOP || cmd.cmd_id == MSG_CMD_SERVER_STOP) break;
            continue;
        }

        // Print message info
        crypto_msg_t *req = malloc(sizeof(crypto_msg_t) + hdr.payload_len);
        if (!req) {
            fprintf(stderr, "[ERROR] Out of memory.\n");
            return -1;

        }
        req->header = hdr;
        if (hdr.payload_len > 0)
            memcpy(req->payload, payload, hdr.payload_len);

        print_msg_info(req, session_key, CLIENT_MODE); //print response
        fflush(stdout);
        free(req);

        // Send header and payload to server side
        uint8_t net_hdr[4];
        net_hdr[0] = hdr.msg_type;
        net_hdr[1] = hdr.direction;
        uint16_t nlen = htons(hdr.payload_len);
        memcpy(&net_hdr[2], &nlen, sizeof(uint16_t));

        if (send_all(sockfd, net_hdr, sizeof(net_hdr)) != (ssize_t)sizeof(net_hdr)) {
            perror("[ERROR] send header");
            return -1;
        }
        if (hdr.payload_len > 0) {
            if (send_all(sockfd, payload, hdr.payload_len) != (ssize_t)hdr.payload_len) {
                perror("[ERROR] send payload");
                return -1;
            }
        }

        // Handles exiting
        if (cmd.cmd_id == MSG_CMD_CLIENT_STOP || cmd.cmd_id == MSG_CMD_SERVER_STOP) {
            //printf("[INFO] Exit command sent. Closing.\n");
            break;
        }

        // Recieves a response
        crypto_msg_t *resp = NULL;
        size_t resp_sz = 0;
        int rr = recv_message(sockfd, &resp, &resp_sz);
        if (rr == 0) {
            printf("[INFO] Server closed connection.\n");
            return 0;
        }
        if (rr < 0) {
            printf("[ERROR] recv_message failed (%d)\n", rr);
            return -1;
        }

        // Debug/inspect 
        print_msg_info(resp, session_key, CLIENT_MODE);

        // Process response things
        if (resp->header.msg_type == MSG_KEY_EXCHANGE &&
            resp->header.payload_len == sizeof(crypto_key_t)) {
            memcpy(&session_key, resp->payload, sizeof(crypto_key_t));
            //printf("[INFO] Session key received: 0x%04x\n\n", session_key);
        } else if (resp->header.msg_type == MSG_ENCRYPTED_DATA &&
                   resp->header.payload_len > 0) {
            /* Optionally print plaintext (print_msg_info already shows it) */
            uint16_t L = resp->header.payload_len;
            uint8_t *plain = (uint8_t*)malloc(L + 1);
            if (plain) {
                int dec = decrypt_string(session_key, plain, resp->payload, L);
                if (dec > 0) {
                    plain[L] = '\0';
                    //printf("[SERVER] %s\n\n", plain);
                }
                free(plain);
            }
        } else if (resp->header.msg_type == MSG_ERROR) {
            printf("[SERVER ERROR]\n\n");
        }

        free(resp);
    }

    return 0;
}

/* =============================================================================
 * STUDENT TODO: IMPLEMENT THIS FUNCTION
 * =============================================================================
 * This is the main client function. You need to:
 * 1. Create a TCP socket
 * 2. Connect to the server
 * 3. Call your communication loop
 * 4. Clean up and close the socket
 * 
 * Parameters:
 *   addr - Server IP address (e.g., "127.0.0.1")
 *   port - Server port number (e.g., 1234)
 */
/*
void start_client(const char* addr, int port) {
    printf("Student TODO: Implement start_client()\n");
    printf("  - Create TCP socket\n");
    printf("  - Connect to %s:%d\n", addr, port);
    printf("  - Implement communication loop\n");
    printf("  - Close socket when done\n");
}
*/
void start_client(const char* addr, int port) {
    printf("Starting TCP client: connecting to %s:%d\n", addr, port);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("[ERROR] socket");
        return;
    }

    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port   = htons((uint16_t)port);
    if (inet_pton(AF_INET, addr, &sa.sin_addr) != 1) {
        fprintf(stderr, "[ERROR] invalid address: %s\n", addr);
        close(fd);
        return;
    }

    if (connect(fd, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
        perror("connect");
        close(fd);
        return;
    }

    printf("Connected to %s:%d\n\n", addr, port);
    printf("Type messages to send to server.\n");
    (void)client_loop(fd);

    close(fd);
    printf("Exiting client...\nClient disconnected.\n");
}


/* =============================================================================
 * PROVIDED HELPER FUNCTION: get_command()
 * =============================================================================
 * This function is FULLY IMPLEMENTED for you. It handles user input and
 * interprets special command characters.
 * 
 * HOW TO USE:
 *   char input_buffer[MAX_MSG_DATA_SIZE];
 *   msg_cmd_t command;
 *   int result = get_command(input_buffer, MAX_MSG_DATA_SIZE, &command);
 *   if (result == CMD_EXECUTE) {
 *       // command.cmd_id contains the message type
 *       // command.cmd_line contains the message text (or NULL)
 *   } else {
 *       // CMD_NO_EXEC means skip this command (like '?' for help)
 *   }
 * 
 * COMMAND FORMAT:
 *   Regular text      -> MSG_DATA (plain text message)
 *   !<message>        -> MSG_ENCRYPTED_DATA (encrypt the message)
 *   #                 -> MSG_KEY_EXCHANGE (request encryption key)
 *   -                 -> MSG_CMD_CLIENT_STOP (exit client)
 *   =                 -> MSG_CMD_SERVER_STOP (shutdown server)
 *   ?                 -> Show help (returns CMD_NO_EXEC)
 * 
 * RETURN VALUES:
 *   CMD_EXECUTE  - Command should be sent to server (use cmd_id and cmd_line)
 *   CMD_NO_EXEC  - Command was handled locally (like help), don't send
 * 
 * IMPORTANT NOTES:
 *   - The returned cmd_line is a pointer into cmd_buff (no need to free)
 *   - For commands without data (like '#'), cmd_line will be NULL
 *   - For '!' commands, cmd_line points to text AFTER the '!' character
 */
int get_command(char *cmd_buff, size_t cmd_buff_sz, msg_cmd_t *msg_cmd)
{
    if ((cmd_buff == NULL) || (cmd_buff_sz == 0)) return CMD_NO_EXEC;

    printf("> ");
    fflush(stdout);
    
    // Get input from user
    if (fgets(cmd_buff, cmd_buff_sz, stdin) == NULL) {
        printf("Error reading input command.\n\n");
        return CMD_NO_EXEC;
    }
    
    // Remove trailing newline
    cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

    // Interpret the command based on first character
    switch (cmd_buff[0]) {
        case '!':
            // Encrypted message - everything after '!' is the message
            msg_cmd->cmd_id = MSG_ENCRYPTED_DATA;
            msg_cmd->cmd_line = cmd_buff + 1; // Skip the '!' character
            return CMD_EXECUTE;
            
        case '#':
            // Key exchange request - no message data
            msg_cmd->cmd_id = MSG_KEY_EXCHANGE;
            msg_cmd->cmd_line = NULL;
            return CMD_EXECUTE;
            
        case '$':
            // Digital signature (not implemented in this assignment)
            msg_cmd->cmd_id = MSG_DIG_SIGNATURE;
            msg_cmd->cmd_line = NULL;
            printf("[INFO] Digital signature command not implemented yet.\n\n");
            return CMD_NO_EXEC;
            
        case '-':
            // Client exit command
            msg_cmd->cmd_id = MSG_CMD_CLIENT_STOP;
            msg_cmd->cmd_line = NULL;
            return CMD_EXECUTE;
            
        case '=':
            // Server shutdown command
            msg_cmd->cmd_id = MSG_CMD_SERVER_STOP;
            msg_cmd->cmd_line = NULL;
            return CMD_EXECUTE;
            
        case '?':
            // Help - display available commands
            msg_cmd->cmd_id = MSG_HELP_CMD;
            msg_cmd->cmd_line = NULL;
            printf("Available commands:\n");
            printf("  <message>  : Send plain text message\n");
            printf("  !<message> : Send encrypted message (requires key exchange first)\n");
            printf("  #          : Request key exchange from server\n");
            printf("  ?          : Show this help message\n");
            printf("  -          : Exit the client\n");
            printf("  =          : Exit the client and request server shutdown\n\n");
            return CMD_NO_EXEC;
            
        default:
            // Regular text message
            msg_cmd->cmd_id = MSG_DATA;
            msg_cmd->cmd_line = cmd_buff;
            return CMD_EXECUTE;
    }
    
    return CMD_NO_EXEC;
}