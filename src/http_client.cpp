/**
 * @file http_client.cpp 
 * @author Rediet Worku aka Aethiops II ben Zahab (aethiopis2rises@gmail.com)
 * 
 * @brief a small command-line http client that demo's how it works under the hood.
 * @version 0.1
 * @date 2024-01-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//=====================================================================================|
//          INCLUDES
//=====================================================================================|
#include "basics.h"


#define HTTP_PORT       80
#define MAX_GET_CMD     255
#define BUFFER_SIZE     255



//=====================================================================================|
//          FUNCTIONS
//=====================================================================================|
/**
 * @brief Accept a well-formed URL (e.g. http://www.company.com/index.html) and return
 *  pointers to the host part and the path part. Note that this function modifies the uri 
 *  itself as well.
 * 
 * @param url the url string to parse
 * @param host pointer to hostname part of url
 * @param path pointer to path part of url
 * @return int 0 on success, -1 if the URL is found to be malformed in any way.
 */
int Parse_Url(char *url, char **host, char **path)
{
    char *pos = strstr(url, "//");
    if (!pos)
        return -1;

    *host = pos + 2;

    pos = strchr(*host, '/');
    if (!pos)
        *path = nullptr;
    else
    {
        *pos = '\0';
        *path = pos + 1;      
    } // end else

    return 0;
} // end Parse_Url



//=====================================================================================|
/**
 * @brief Format and send an HTTP get command. The caller must then retrieve the response.
 * 
 * @param fds a connected socket descriptor
 * @param host the hostname
 * @param path path to fetch from connected host
 * @return int 0 on success, -1 on failure, with errno set appropriately
 */
int Http_Get(int fds, char *host, char *path)
{
    static char get_command[MAX_GET_CMD];

    snprintf(get_command, MAX_GET_CMD, "GET %s HTTP/1.1\r\n", path);
    if ( send(fds, get_command, strlen(get_command), 0) < 0)
        return -1;

    snprintf(get_command, MAX_GET_CMD, "Host: %s\r\n", host);
    if ( send(fds, get_command, strlen(get_command), 0) < 0)
        return -1;

    snprintf(get_command, MAX_GET_CMD, "Connection: close\r\n\r\n");
    if ( send(fds, get_command, strlen(get_command), 0) < 0)
        return -1;

    return 0;
} // end Http_Get



//=====================================================================================|
/**
 * @brief Receive all data available on a connection and dump it to stdout
 * 
 * @param fds a descriptor for a connected socket
 */
void Display_Result(int fds)
{
    int recvd = 0;
    static char recv_buf[BUFFER_SIZE + 1];

    while ( (recvd = recv(fds, recv_buf, BUFFER_SIZE, 0)) > 0)
    {
        recv_buf[recvd] = '\0';
        printf("%s", recv_buf);
    } // end while

    printf("\n");
} // end Display_Result


//=====================================================================================|
/**
 * @brief a simple command http web client
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[])
{
    int fds;         // descriptor to client connection
    char *host, *path;
    struct hostent *hostname;
    struct sockaddr_in hostaddress;

    if (argc < 2)
    {
        fprintf(stderr, "[-] Usage: %s <URL>\n", argv[0]);
        return 1;
    } // end if


    if (Parse_Url(argv[1], &host, &path) == -1)
    {
        fprintf(stderr, "[-] Error -- Malformed URL: %s\n", argv[1]);
        return 1;
    } // end if

    printf("[+] Connecting to host: %s\n", argv[1]);

    #ifdef WINDOWS
        WSADATA wsa;
        if (!WSAStartup(MAKEWORD(2,2), &wsa))
        {
            fprintf(stderr, "[-] Error -- Failed to load WSAStartup()\n");
            return 1;
        } // end if
    #endif

    if ( (fds = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        return 2;
    } // end if

    if ( !(hostname = gethostbyname(host)))
    {
        perror("Error in name resolution");
        return 3;
    } // end if

    iZero(&hostaddress, sizeof(hostaddress));
    hostaddress.sin_family = AF_INET;   // PF_INET
    hostaddress.sin_port = htons(HTTP_PORT);
    memcpy(&hostaddress.sin_addr, hostname->h_addr_list[0], sizeof(struct in_addr));

    if (connect(fds, (struct sockaddr *)&hostaddress, sizeof(hostaddress)) < 0)
    {
        fprintf(stderr, "[-] Error- unable to connect to host\n");
        return 4;
    } // end if

    printf("Retrieving documents: %s\n", path);

    Http_Get(fds, host, path);
    Display_Result(fds);


    printf("Shutting down.\n");
    CLOSE(fds);

#ifdef WINDOWS
    WSACleanup();
#endif

    return 0;
} // end main