#include <thread> // multiple client handling
using namespace std;//to group standard libraries in code 
#include "../include/server.h"
#include "../include/persistence.h"

#define PORT 6379 //define the local port no
#define BACKLOG 10//maximum number of queue request pending the server can have



int main() {
    LoadDatabase();//load the value in database after starting again
    int server_fd=createServer();//create a server for handling clients

    cout << "Server ready, Waiting for clients..."<<endl;
    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        
        //accept() a connection, get client_fd
        int client_fd=accept(server_fd,(sockaddr*)&client_addr,&client_len);
        
        if(client_fd < 0) {
            perror("accept failed");
            continue;
        }
        thread client_Thread(handleClient,client_fd,client_addr);
        client_Thread.detach();
    }

    close(server_fd);//close local server
    return 0;
}