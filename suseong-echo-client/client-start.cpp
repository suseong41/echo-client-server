#include "client-start.h"
#include "../pch.h"

typedef int SOCKET;
#define SOCKET_ERROR   -1
#define INVALID_SOCKET -1

#define BUFSIZE 4096

static void* recv_thread(void* p)
{
    SOCKET sock = *(SOCKET*)p;
    char buf[BUFSIZE+1];
    
    while(true)
    {
        int n = recv(sock, buf, BUFSIZE, 0);
        if (n<=0)
        {
            if(n<0) err_display("recv()");
            break;
        }
        buf[n] = '\0';
        printf("\n[받은 데이터] %s\n", buf);
        fflush(stdout);
    }
    return nullptr;
}

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
    
    pthread_t tid;
    pthread_create(&tid, nullptr, recv_thread, &sock);
    pthread_detach(tid);
    
    char buf[BUFSIZE+1];
    
    while(1)
    {
        printf("\n[보낼 데이터]");
        if (!fgets(buf, BUFSIZE+1, stdin)) break;
        
        int len = (int)strlen(buf);
        if (len && buf[len-1] == '\n') 
        {
            buf[len-1] = '\0'; 
            --len;
        }
        if(len == 0) continue;
        
        ret = send(sock, buf, len, MSG_NOSIGNAL);
        if(ret == SOCKET_ERROR)
        {
            err_display("send()");
            break;
        }
    }
    close(sock);   
    return 0;
}