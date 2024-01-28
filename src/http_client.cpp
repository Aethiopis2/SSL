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
 * @brief Connects to a proxy server for forwarding HTTP requests. The proxy is in the
 *  format http://username:password@proxy_host:proxy_port
 * 
 * @param proxy_spec the url string containing the proxy info
 * @param proxy_host gets the host info
 * @param proxy_port parses the port info
 * @param proxy_user parses the user info
 * @param proxy_password parses the user password for the proxy account
 * 
 * @return int 0 on fail, 1 on success.
 */
int Parse_Proxy_Param(char *proxy_spec, char **proxy_host, int *proxy_port, 
    char **proxy_user, char **proxy_password)
{
    char *login_sep, *colon_sep, *tariler_sep;

    if (!strncmp("http://", proxy_spec, 7))
        proxy_spec += 7;

    if ( login_sep = strchr(proxy_spec, '@'))
    {
        colon_sep = strchr(proxy_spec, ':');
        if ( !colon_sep || colon_sep > login_sep)
        {
            fprintf(stderr, "Error: Expected password in %s.\n", proxy_spec);
            return 0;
        } // end if bad newz

        *colon_sep = '\0';
        *login_sep = '\0';
        *proxy_user = proxy_spec;
        *proxy_password = colon_sep + 1;
        proxy_spec = login_sep + 1;
    } // end if login

    tariler_sep = strchr(proxy_spec, '/');
    if (*tariler_sep = '/')
        *tariler_sep = '\0';

    if ( (colon_sep = strchr(proxy_spec, ':')))
    {
        *colon_sep = '\0';
        *proxy_host = proxy_spec;
        *proxy_port = atoi(colon_sep + 1);

        if (*proxy_port == 0)
            return 0;
        else
        {
            *proxy_port = HTTP_PORT;
            *proxy_host = proxy_spec;
        } // end else
    } // end if proxy server part

    return 1;
} // end Parse_Proxy_Param



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
 * @param proxy_host proxy server host address if there is one
 * @param proxy_user proxy username
 * @param proxy_password password for proxy user
 * 
 * @return int int 0 on success, -1 on failure, with errno set appropriately
 */
int Http_Get(int fds, char *host, char *path, const char *proxy_host,
    const char *proxy_user, const char *proxy_password)
{
    static char get_command[MAX_GET_CMD];

    if (proxy_host)
        snprintf(get_command, MAX_GET_CMD, "GET http://%s/%s HTTP/1.1\r\n", host, path);
    else
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
    int fds;                // descriptor to client connection
    char *host, *path;
    char *proxy_host, *proxy_user, *proxy_password;
    int proxy_port;
    int ind = 1;
    struct hostent *hostname;
    struct sockaddr_in hostaddress;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <URL>\n", argv[0]);
        fprintf(stderr, "Usage: %s [-p http://username:password@]proxy-host:proxy-port] <URL>\n", argv[0]);
        return 1;
    } // end if

    proxy_host = proxy_password = proxy_user = host = path = nullptr;
    if (!strncmp("-p", argv[ind], strlen("-p")))
    {
        if (!Parse_Proxy_Param(argv[++ind], &proxy_host, &proxy_port, 
            &proxy_user, &proxy_password))
        {
            fprintf(stderr, "Error -- Malformed proxy parameter: %s\n", argv[2]);
            return 1;
        } // end if Bad Parse
        ind++;
    } // end if proxy


    if (Parse_Url(argv[1], &host, &path) == -1)
    {
        fprintf(stderr, "Error -- Malformed URL: %s\n", argv[1]);
        return 1;
    } // end if

    printf("Connecting to host: %s\n", argv[1]);
    if (proxy_host)
        hostname = gethostbyname(proxy_host);
    else
        hostname = gethostbyname(host);

    if ( !(hostname))
    {
        perror("Error in name resolution");
        return 3;
    } // end if

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


    iZero(&hostaddress, sizeof(hostaddress));
    hostaddress.sin_family = AF_INET;   // PF_INET
    hostaddress.sin_port = (proxy_port ? htons(proxy_port) : htons(HTTP_PORT));
    memcpy(&hostaddress.sin_addr, hostname->h_addr_list[0], sizeof(struct in_addr));

    if (connect(fds, (struct sockaddr *)&hostaddress, sizeof(hostaddress)) < 0)
    {
        fprintf(stderr, "Error- unable to connect to host\n");
        return 4;
    } // end if

    printf("Retrieving documents: %s\n", path);

    Http_Get(fds, host, path, proxy_host, proxy_user, proxy_password);
    Display_Result(fds);


    printf("Shutting down.\n");
    CLOSE(fds);

#ifdef WINDOWS
    WSACleanup();
#endif

    return 0;
} // end main