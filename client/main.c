#ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h> // Never before Winsock2
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "Ws2_32.lib")

// Great guide:
// https://docs.microsoft.com/en-us/windows/win32/winsock/creating-a-basic-winsock-application


#define DIE(msg, ...)  { printf("Error: "msg"\n",## __VA_ARGS__); exit_code = 1; goto cleanup; }
#define USAGE(msg)     { printf(msg"\nUsage:\n    client.exe [address port] [message]"); }

static const char* const default_message = "Hello\r\n";
static const char* const default_ip = "127.0.0.1";
static const char* const default_port = "7788";

int main(int argc, char *argv[]) {
    int exit_code = 0;
    WSADATA wsaData;
    SOCKET client = INVALID_SOCKET;
    struct sockaddr_in address = { 0 };
    char* user_message = NULL;
    const char* message = default_message;
    const char* ip = default_ip;
    const char* port = default_port;
    char buffer[1024];
    struct sockaddr from = { 0 };
    int from_len = sizeof(from);

    // Start WSA before anything
    if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        printf("Error: Could not start wsa\n");
        return 1;
    }

    address.sin_family = AF_INET;

    // Parse args
    // Get port and IP
    if (argc > 2)
    {
        ip = argv[1];
        port = argv[2];
    }
    printf("Address: '%s'  Port: '%s'\n", ip, port);
    // Win < Vista you'll have to do:
    //   address.sin_addr.s_addr = inet_addr(user_address);
    // instead of inet_pton()
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons((SHORT)atoi(port));
    // Custom message
    if (argc > 3)
    {
        char* p_msg = argv[3];
        size_t msg_len = strlen(p_msg);
        // null + \r\n
        user_message = malloc(msg_len + 3);
        if (NULL == user_message)
            DIE("Could not malloc user_message")
        strncpy(user_message, p_msg, msg_len);
        strncpy(user_message + msg_len, "\r\n", 3);
        message = user_message;
    }  
    if (argc > 4)
    {
        USAGE("Too many arguments");
        exit_code = 1;
        goto cleanup;
    }
       
    // Create UDP socket
    client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (INVALID_SOCKET == client)
        DIE("Invalid socket");

    printf("Sending: '%s'\n", message);
    int result = sendto(client, message, strlen(message), 0, (SOCKADDR*)&address, sizeof(address));
    if (SOCKET_ERROR == result)
        DIE("Could not send message. code %d", WSAGetLastError());

    result = recvfrom(client, &buffer, 1024 - 1, 0, &from, &from_len);
    if (SOCKET_ERROR == result)
        DIE("Error receiving message. code %d", WSAGetLastError());
    // ensure null terminated string
    buffer[result] = '\0';
    printf("Response: '%s'\n", buffer);

cleanup:
    printf("Shutting down\n");
    if (INVALID_SOCKET != client)
        closesocket(client);
    if (user_message)
        free(user_message);

    WSACleanup();
    return exit_code;
}