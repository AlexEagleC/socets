#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>
using namespace std;

struct record
    {
        int fd;
        char *login;
        char *message;
        record *next;
        record *prior;
    };

record * Begin = NULL, *act = NULL, *list = NULL, *list_2 = NULL, *End;

int listener;

void rmv(void)
{
    list_2 = Begin;
    while (list_2)
        {
            if ((*list).fd == (*list_2).fd)
            {
                if ((*list_2).prior == NULL)
                {
                    (*(*list_2).next).prior = NULL;
                    Begin = (*list_2).next;
                }
                if ((*list_2).next == NULL)
                {
                    (*(*list_2).prior).next = NULL;
                    End = (*list_2).prior;
                }
                if (((*list_2).prior != NULL) && ((*list_2).next != NULL))
                {
                    (*(*list_2).next).prior = (*list_2).prior;
                    (*(*list_2).prior).next = (*list_2).next;
                }
            }
            list_2 = list_2->next;
        }
        list = Begin;
}

void set(int sock)
{
    act = new(record);
    (*act).fd = sock;
    (*act).login = NULL;
    if (Begin == NULL)
        {
            (*act).prior = NULL;
            (*act).next = NULL;
            Begin = act;
            End = act;
        }
    else
        {
            (*act).prior = End;
            (*act).next = NULL;
            (*End).next = act;
            End = act;
        } 
}

int Max(void)
{
    list = Begin;
    int mx = listener;
    while (list)
    {
        if ((*list).fd > mx)
        {
            mx = (*list).fd;
        }
        list = (*list).next;
    }
    return(mx);    
}

void send_message(char *message)
{
    char *ms = strchr(message, ':');
    char *rec;
    rec = new(char[strlen(message) - strlen(ms) - 1]);
    strncpy(rec, message, strlen(message) - strlen(ms) - 1);
    char *full_mes = new(char[strlen(message) + 1]);
    strcpy(full_mes, (*list).login);
    strcat(full_mes, ms);
    cout << full_mes << "\n";
    fflush(stdout);
    list_2 = Begin;
    while (list_2)
    {
        if (*((*list_2).login) == *rec)
        {     
            send((*list_2).fd, full_mes, strlen(full_mes) + 1 , 0);
        }
        list_2 = (*list_2).next;
    }
}

int main()
{
    struct sockaddr_in addr;
    char buf[1024];
    int bytes_read;
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0)
    {
        perror("socket");
        exit(1);
    }
    
    fcntl(listener, F_SETFL, O_NONBLOCK);
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3425);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(2);
    }

    listen(listener, 5);
    

    while(1)
    {
        // Заполняем множество сокетов
        fd_set readset;
        FD_ZERO(&readset);
        FD_SET(listener, &readset);

        list = Begin;
        while(list)
        {
            FD_SET((*list).fd, &readset);
            list = (*list).next;
        }
        // Задаём таймаут
        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        // Ждём события в одном из сокетов
        if(select(Max()+1, &readset, NULL, NULL, &timeout) < 0)
        {
            perror("select");
            exit(3);
        }
        
        // Определяем тип события и выполняем соответствующие действия
        if(FD_ISSET(listener, &readset))
        {
            // Поступил новый запрос на соединение, используем accept
            int sock = accept(listener, NULL, NULL);
            if(sock < 0)
            {
                perror("accept");
                exit(3);
            }
            
            fcntl(sock, F_SETFL, O_NONBLOCK);
            set(sock);
            //Соединение установлено, запрос логина
            
        }
        list = Begin;
        while (list)
        {           
            if(FD_ISSET((*list).fd, &readset))
            {
                // Поступили данные от клиента, читаем их
                bytes_read = recv((*list).fd, buf, 1024, 0);
                if ((*list).login != NULL) 
                { 
                    (*list).message = new(char[bytes_read]);
                    strncpy((*list).message, buf, bytes_read);
                    cout << (*list).message << "\n";
                    fflush(stdout);
                    send_message((*list).message);
                }
                else 
                { 
                    (*list).login = new(char[bytes_read]);
                    strncpy((*list).login, buf, bytes_read);
                    list_2 = Begin;
                    while (list_2)
                    {
                        cout << (*list_2).login << "\t" << (*list_2).fd << "\n";
                        fflush(stdout);
                        list_2 = (*list_2).next;
                    }
                }
                if(bytes_read <= 0)
                {
                    // Соединение разорвано, удаляем сокет из множества
                    close((*list).fd);
                    rmv();
                    continue;
                }
            }
            list = (*list).next;
        }
    }
    return 0;
}