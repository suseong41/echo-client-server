#include "client-start.h"
#include "../pch.h"

typedef int SOCKET;
#define SOCKET_ERROR   -1
#define INVALID_SOCKET -1

#define BUFSIZE 4096

int main(int argc, char* argv[]) 
{
    if (argc != 3) return EXIT_FAILURE;
    int ret;
    const char* IP = argv[1];
    uint16_t PORT = atoi(argv[2]);
    
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");
    
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, IP, &serveraddr.sin_addr);
    serveraddr.sin_port = htons(PORT);
    ret = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if(ret == SOCKET_ERROR) err_quit("connect()");
    
    char buf[BUFSIZE+1];
    int len;
    
    while(1)
    {
        printf("\n[보낼 데이터]");
        if(fgets(buf, BUFSIZE+1, stdin) == NULL) break;
        
        len = (int)strlen(buf);
        if(buf[len-1] == '\n') buf[len-1] = '\0';
        if(strlen(buf) == 0) break;
        
        ret = send(sock, buf, (int)strlen(buf), 0);
        if(ret == SOCKET_ERROR)
        {
            err_display("send()");
            break;
        }
        
        ret = recv(sock, buf, ret, MSG_WAITALL);
        if(ret ==SOCKET_ERROR)
        {
            err_display("recv()");
            break;
        }
        else if(ret == 0) break;
        
        buf[ret] = '\0';
        printf("[받은 데이터] %s", buf);
    }
    
    close(sock);   
    
    return 0;
}