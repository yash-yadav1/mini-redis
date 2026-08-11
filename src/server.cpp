#include <bits/stdc++.h>//to include cpp libraries
#include <sys/socket.h>//to include the operations like bind,socket,listen,accept
#include <netinet/in.h>//contains networking structures like sys_addr_in,address family
#include <unistd.h>//linux system calls like read() , write(),close();
#include <arpa/inet.h>//used for the client port no. and ip address  for INET_ntop
#include <thread>//multipleclient handling


#include "../include/server.h"
#include "../include/command.h"
#include "../include/resp.h"

using namespace std;

#define PORT 6379 //define the local port no
#define BACKLOG 10//maximum number of queue request pending the server can have

int createServer(){
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);//create a socket AF_INET-> IPv4  , AF_INET6-> IPv6 , sock_stream->TCP , sock_dgram->UDP,protocol for TCP/UDP   (0 -> use defualt protocol TCP in this case)
    //socket returns file descripter 
    if (server_fd < 0) {//error 
        perror("socket failed");
        exit(1);
    }

    int opt = 1;//option turn on
    // Allow immediate reuse of the port after restart (avoid "Address already in use")
    int setsock = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if(setsock < 0 ) {
        perror("setsockopt");
        exit(1);
    }
    sockaddr_in addr{};
    // addr family has IP- _____ , port- ________, protocol- ________ initially all empty
    addr.sin_family = AF_INET;//IP= IPv4
    addr.sin_addr.s_addr = INADDR_ANY;// allow  client with any input ip 


    addr.sin_port = htons(PORT);//to convert the big endian ,small endian   htons means (host to network short) 
    //network uses big endian    num=c3f4      in data=c3,f4
    //intel uses small endian    num=c3f4      in data=f4,c3

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {//bind the client to input port number 
        perror("bind failed");
        exit(1);
    }

    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen failed");
        exit(1);
    }

    cout << "Server listening on port " << PORT << "...\n";
    return server_fd;
}



void sendReply(int client_fd,const string & reply){
    write(client_fd,reply.c_str(),reply.size());
}



void handleClient(int client_fd,sockaddr_in client_addr){
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET,&client_addr.sin_addr,client_ip,sizeof(client_ip));
    int client_port=ntohs(client_addr.sin_port);
    cout << "Client connected from "<< client_ip<< ":"<< client_port<< endl;
    
    char buffer[1025];
    string inbuf;
    bool quit=false;
    while(!quit) {
        // read() bytes from client_fd into a buffer
        
        int n = read(client_fd,buffer,1024 );
        
        if(n==0) {
            cout<<"Client with ip and port number : "<<client_ip<<":"<<client_port<<" disconnected "<<endl;
            break;
        }else if(n<0){
            perror("read error");
            break;
        }
        inbuf.append(buffer,n);
        while(true) {


            if(!inbuf.empty() && inbuf[0] != '*')//if client sends direct command
            {
                size_t pos = inbuf.find("\r\n");

                if(pos == string::npos)
                    break;

                string line = inbuf.substr(0,pos);

                vector<string> tokens;

                stringstream ss(line);

                string word;

                while(ss >> word)
                    tokens.push_back(word);

                commandResult cmd = executeCommand(tokens);

                sendReply(client_fd,cmd.reply);

                inbuf.erase(0,pos+2);

                continue;
            }
            RespResult result = ParseRESP(inbuf);
            
            if(result.bytesConsumed==-1){
                sendReply(client_fd, RespError("Protocol error"));
                break;
            }else if(result.bytesConsumed==0){
                break;
            }else{

                //execution command             
                commandResult cmd = executeCommand(result.tokens);
                
                sendReply(client_fd,cmd.reply);
                inbuf.erase(0,result.bytesConsumed);
                if(cmd.quit){
                    cout<<"Client with ip and port number : "<<client_ip<<":"<<client_port<<" said  bye bye! "<<endl;
                    quit=true;
                    break;
                }
            }
        }
    }
    // close(client_fd)
    close(client_fd);
}