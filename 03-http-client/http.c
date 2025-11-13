#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#include "http.h"

//---------------------------------------------------------------------------------
// TODO:  Documentation
//
// Note that this module includes a number of helper functions to support this
// assignment.  YOU DO NOT NEED TO MODIFY ANY OF THIS CODE.  What you need to do
// is to appropriately document the socket_connect(), get_http_header_len(), and
// get_http_content_len() functions. 
//
// NOTE:  I am not looking for a line-by-line set of comments.  I am looking for 
//        a comment block at the top of each function that clearly highlights you
//        understanding about how the function works and that you researched the
//        function calls that I used.  You may (and likely should) add additional
//        comments within the function body itself highlighting key aspects of 
//        what is going on.
//
// There is also an optional extra credit activity at the end of this function. If
// you partake, you need to rewrite the body of this function with a more optimal 
// implementation. See the directions for this if you want to take on the extra
// credit. 
//--------------------------------------------------------------------------------

/*
* Purpose: finds a substring inside another string disregarding whether its upper or lower case
* 
* Loops through main string and compares lower case values
* When the first character matches, checks to see if the rest of the substring also matches
* Puts a pointer to the match or NULL if not found
*
* Uses it for HTTP headers
*
*/
char *strcasestr(const char *s, const char *find)
{
	char c, sc;
	size_t len;

    // makes sure string isnt empty
	if ((c = *find++) != 0) {
        // lowercase first character of the "find" string
		c = tolower((unsigned char)c);
		len = strlen(find);
		do {
			do {
                // loops through the string until we match
				if ((sc = *s++) == 0)
					return (NULL); // no match
			} while ((char)tolower((unsigned char)sc) != c);
            // compares rest of string
		} while (strncasecmp(s, find, len) != 0);
		s--; // pointer to the start of the match string
	}

    // returns pointer to match or null if not found
	return ((char *)s);
}

/*
* Purpose: finds a substring inside another string but only within a limited length
* 
* Works the same as the previous function, however, it doesn't lower case the string
* and it wont search pass "slen" characters in "s"
* Puts a pointer to the match or NULL if not found
* 
* Searches inside a buffer when you know the size
*/
char *strnstr(const char *s, const char *find, size_t slen)
{
	char c, sc;
	size_t len;
    
    // not an empty string
	if ((c = *find++) != '\0') {
		len = strlen(find);     // length of substring after the first character
		do {
			do {
                // If we reach end of main string or run out of allowed length, the function stops
				if ((sc = *s++) == '\0' || slen-- < 1)
					return (NULL);
			} while (sc != c); // search until first matching character is found

            // not enough characters left to match, stop
			if (len > slen)
				return (NULL);
		} while (strncmp(s, find, len) != 0); // compares the next "len" values for a full match
		s--; // pointer to matched substring
	} 
    // returns pointer to match or null if not found
	return ((char *)s);
}

/*
* Purpose: establishes a TCP connection to host and port, returns the connected socket
*
* uses gethostbyname() to put hostname into IP address
* sockaddr_in addr structure with the IP, host, and AF_INET
* creates TCP socket
* attempts to connect to server
* returns the socket if successful
*
*
* Uses it for IPv4 only
 */
int socket_connect(const char *host, uint16_t port){
    struct hostent *hp;
    struct sockaddr_in addr;
    int sock;

    // hostname to IP address
    if((hp = gethostbyname(host)) == NULL){
		herror("gethostbyname");
		return -2;
	}
    
    // copy ip address to structure
	bcopy(hp->h_addr_list[0], &addr.sin_addr, hp->h_length);
	addr.sin_port = htons(port); 
	addr.sin_family = AF_INET; // IPv4 address family
	sock = socket(PF_INET, SOCK_STREAM, 0); // creates TCP socket
	
	if(sock == -1){
		perror("socket");
		return -1; // failed to create socket
	}

    // connects to server
    if(connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1){
		perror("connect");
		close(sock); // cleans up socket if it failed
        return -1;
	}

    // returns connected socket
    return sock;
}

/*
* Purpose: finds the length of the http header 
* 
* searches the response buffer for the header "\r\n\r\n",
* which marks it the end of the header and beginning of the body
* uses strnstr to search within the first http_buff_len bytes
* if found, calculates how many bytes makes up the header
* if not found, header is incomplete
*
* separates the http header from its body when processing data from socket
*/
int get_http_header_len(char *http_buff, int http_buff_len){
    char *end_ptr;
    int header_len = 0;

    // looks for end of header
    end_ptr = strnstr(http_buff,HTTP_HEADER_END,http_buff_len);

    // terminator is heading, header is incomplete or invalid
    if (end_ptr == NULL) {
        fprintf(stderr, "Could not find the end of the HTTP header\n");
        return -1;
    }
    
    // calculate header length
    header_len = (end_ptr - http_buff) + strlen(HTTP_HEADER_END);

    // returns header length
    return header_len;
}


int get_http_content_len(char *http_buff, int http_header_len){
    char header_line[MAX_HEADER_LINE];

    char *next_header_line = http_buff;
    char *end_header_buff = http_buff + http_header_len;

    while (next_header_line < end_header_buff){
        bzero(header_line,sizeof(header_line));
        sscanf(next_header_line,"%[^\r\n]s", header_line);

        char *isCLHeader = strcasestr(header_line,CL_HEADER);
        if(isCLHeader != NULL){
            char *header_value_start = strchr(header_line, HTTP_HEADER_DELIM);
            if (header_value_start != NULL){
                char *header_value = header_value_start + 1;
                int content_len = atoi(header_value);
                return content_len;
            }
        }
        next_header_line += strlen(header_line) + strlen(HTTP_HEADER_EOL);
    }
    fprintf(stderr,"Did not find content length\n");
    return 0;
}

//This function just prints the header, it might be helpful for your debugging
//You dont need to document this or do anything with it, its self explanitory. :-)
void print_header(char *http_buff, int http_header_len){
    fprintf(stdout, "%.*s\n",http_header_len,http_buff);
}

//--------------------------------------------------------------------------------------
//EXTRA CREDIT - 10 pts - READ BELOW
//
// Implement a function that processes the header in one pass to figure out BOTH the
// header length and the content length.  I provided an implementation below just to 
// highlight what I DONT WANT, in that we are making 2 passes over the buffer to determine
// the header and content length.
//
// To get extra credit, you must process the buffer ONCE getting both the header and content
// length.  Note that you are also free to change the function signature, or use the one I have
// that is passing both of the values back via pointers.  If you change the interface dont forget
// to change the signature in the http.h header file :-).  You also need to update client-ka.c to 
// use this function to get full extra credit. 
//--------------------------------------------------------------------------------------
int process_http_header(char *http_buff, int http_buff_len, int *header_len, int *content_len){
    int h_len, c_len = 0;
    h_len = get_http_header_len(http_buff, http_buff_len);
    if (h_len < 0) {
        *header_len = 0;
        *content_len = 0;
        return -1;
    }
    c_len = get_http_content_len(http_buff, http_buff_len);
    if (c_len < 0) {
        *header_len = 0;
        *content_len = 0;
        return -1;
    }

    *header_len = h_len;
    *content_len = c_len;
    return 0; //success
}