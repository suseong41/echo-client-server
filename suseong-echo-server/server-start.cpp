#include "server-start.h"
#include "../pch.h"

#define BUFSIZE 4096

typedef int SOCKET;
#define SOCKET_ERROR   -1
#define INVALID_SOCKET -1

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
std::vector<SOCKET> clients;

struct ThreadArgs 
{
    SOCKET client_sock;
    bool echo;
    bool broadcast;
};

void *ProcessClient(void *arg)
{    
    ThreadArgs* args = static_cast<ThreadArgs*>(arg);
    SOCKET client_sock = args->client_sock;
    bool echo_mode = args->echo;
    bool broadcast_mode = args->broadcast;
    delete args;
    
    int ret;
    struct sockaddr_in clientaddr;
    char addr[INET_ADDRSTRLEN];
    socklen_t addrlen;
    char buf[BUFSIZE + 1];
    
    addrlen = sizeof(clientaddr);
    getpeername(client_sock, (struct sockaddr *)&clientaddr, &addrlen);
    inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
    
    while(true)  
    {
        ret = recv(client_sock, buf, BUFSIZE, 0);
        if (ret == SOCKET_ERROR)
        {
            err_display("recv()");
            break;
        }
        else if (ret == 0) break;
        
        buf[ret] = '\0';
        printf("send: %s => %s\n", addr, buf);
        
        if (broadcast_mode) {
            pthread_mutex_lock(&mutex);
            std::vector<SOCKET> dead;
            for (SOCKET s : clients) 
            {
                if ((s == client_sock) && (!echo_mode)) continue;
                int n = send(s, buf, ret, 0);
                if (n < 0) dead.push_back(s);
            }
            if(!dead.empty())
            {
                for (SOCKET d : dead) std::erase(clients, d);
            }
            pthread_mutex_unlock(&mutex);
        } else if (echo_mode) {
            ret = send(client_sock, buf, ret, 0);
            if (ret == SOCKET_ERROR) {
                err_display("send()");
                break;
            }
        }
    }
    pthread_mutex_lock(&mutex);
    std::erase(clients, client_sock);
    pthread_mutex_unlock(&mutex);
    close(client_sock);

    printf("클라이언트 종료: IP 주소=%s, 포트 번호=%d\n", addr, ntohs(clientaddr.sin_port));

    return 0;
}

int main(int argc, char* argv[]) 
{

    if (argc < 2 || argc > 4) 
    {
        fprintf(stderr, "Usage: %s <port> [-e[-b]]\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    int ret;
    int16_t PORT = atoi(argv[1]);
    
    int opt;
    static bool echo_mode = false;
    static bool broadcast_mode = false;
    
    while ((opt = getopt(argc, argv, "eb")) != -1) 
    {
        switch (opt) 
        {
            case 'e':
                echo_mode = true;
                break;
            case 'b':
                broadcast_mode = true;
                break;
            default:
                break;
        }
    }
    
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");
    
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(PORT);
    ret = bind(listen_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (ret == SOCKET_ERROR) err_quit("bind()");
    
    ret = listen(listen_sock, SOMAXCONN);
    if (ret == SOCKET_ERROR) err_quit("listen()");
    
    SOCKET client_sock;
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    pthread_t tid;
    printf("서버 시작\n");
    while(true) 
    {
        addrlen = sizeof(clientaddr);
        client_sock = accept(listen_sock, (struct sockaddr *)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET)
        {
            err_display("accept()");
            break;
        }
        
        char addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
        printf("\n클라이언트 접속: IP 주소=%s, 포트 번호=%d\n", addr, ntohs(clientaddr.sin_port));
        
        pthread_mutex_lock(&mutex);
        clients.push_back(client_sock);
        pthread_mutex_unlock(&mutex);
        
        ThreadArgs* targs = new ThreadArgs{client_sock, echo_mode, broadcast_mode};
        ret = pthread_create(&tid, NULL, ProcessClient, static_cast<void*>(targs));
        if(ret != 0) {close(client_sock);}
        else 
        {
            pthread_detach(tid);
        }
    
    }
    close(listen_sock);
    pthread_mutex_destroy(&mutex);
    return 0;
}