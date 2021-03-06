//
//  main.c
//  tcpserver
//
//  Created by zhengxiao on 09/06/2017.
//  Copyright © 2017 baidu. All rights reserved.
//

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

#define MAX_LEN 100

pthread_mutex_t lock;
pthread_cond_t cond;
int server_is_ready;

int connect_to_server() {
    sleep(1);
    int err = 0;
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    char * inet_addr = "127.0.0.1";
    
    struct sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8888);
    err = inet_pton(AF_INET, inet_addr, &servaddr.sin_addr);
    
    err = connect(sockfd, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    
    char buffer[MAX_LEN + 1];
    
    for(;;){
        long n = read(sockfd, (char *)buffer, MAX_LEN);
        if (n <= 0L) break;
        fputs(buffer, stdout);
    }
    
    err = close(sockfd);
    
    return 0;
}

void run_server() {
    int err = 0;
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    // err = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    
    struct sockaddr_in sockaddr = {0};
    sockaddr.sin_port = htons(8888);
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    err = bind(sockfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
    
    err = listen(sockfd, 3);
    
    // notify
    
    pthread_mutex_lock(&lock);
    server_is_ready = 1;
    printf("server is ready\n");
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);
    
    for(;;){
    
        int client_sockfd = accept(sockfd, NULL, NULL);
    
        char * buffer = "hi";
        write(client_sockfd, buffer, strlen(buffer) + 1);
    
        close(client_sockfd);
        
    }

}


int main(int argc, const char * argv[]) {
    // insert code here...
    printf("Hello, World!\n");
    server_is_ready = 0;
    pthread_t thread;
    
    
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);
    
    // run server
    pthread_create(&thread, NULL, &run_server, NULL);
    
    pthread_mutex_lock(&lock);
    while (!server_is_ready) {
        printf("waiting server\n");
        pthread_cond_wait(&cond, &lock);
    }
    pthread_mutex_unlock(&lock);
    
    // connect to server
    int err = connect_to_server();
    
    pthread_join(thread, NULL);
    
    // clean up
    
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
    

    
    return err;
}
