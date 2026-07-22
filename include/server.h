#pragma once

#include <thread>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;

int createServer();

void sendReply(int client_fd,const string &reply);

void handleClient(int client_fd,sockaddr_in client_addr);