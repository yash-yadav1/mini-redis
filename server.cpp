#include <bits/stdc++.h>//to include cpp libraries
#include <sys/socket.h>//to include the operations like bind,socket,listen,accept
#include <netinet/in.h>//contains networking structures like sys_addr_in,address family
#include <unistd.h>//linux system calls like read() , write(),close();
#include <arpa/inet.h>//used for the client port no. and ip address  for INET_ntop
#include <thread>//mutipleclient handling
#include <mutex>//for mutex lock of database
using namespace std;//to group standard libraries in code 

#define PORT 6379 //define the local port no
#define BACKLOG 10//maximum number of queue request pending the server can have

unordered_map<string,string> database;

mutex databaseMutex;
const string DATABASE_FILE = "database.txt";
void SaveDatabase() {
    ofstream fout(DATABASE_FILE);//create the file if it doesn't exist or just overwrite it
    for(auto &data:database) {
        fout<<data.first<<" "<<data.second<<"\n";
    }
}

void LoadDatabase() {
    ifstream fin(DATABASE_FILE);
    string key;
    string value;
    while(fin>>key>>value) {
        database[key]=value;
    }
}
struct commandResult{
    string reply;
    bool quit;
};

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


commandResult executeCommand(const string & line) {//const to not modify the lines

    stringstream ss(line);
    string cmd;
    ss>>cmd;
    transform(cmd.begin(),cmd.end(),cmd.begin(),::toupper);
    //store information 
    vector<string> args;
    string temp_args;
    while(ss>>temp_args){
        args.push_back(temp_args);
    }
    //execute the commands
    string reply;
    bool quit=false;
    lock_guard<mutex> lock(databaseMutex);
    if(cmd=="PING") {
        reply="PONG\n";

    }else if(cmd=="SET"){
        if(args.size()!=2){
            reply="ERROR insufficient number of arguments.\n";
        }else{
            database[args[0]]=args[1];
            SaveDatabase();
            reply="OK\n";
        }

    }else if(cmd=="GET") {
        if(args.size()!=1){
            reply="ERROR wrong number of arguments.\n";
        }else {
            auto it=database.find(args[0]);
            if(it!=database.end()){
                reply=it->second + "\n";
            }else{
                reply="not in database\n";
            }
        }

    }else  if(cmd=="DEL"){
        if(args.size()!=1){
            reply="ERROR wrong number of arguments.\n";
        }else{
            auto it=database.find(args[0]);
            if(it!=database.end()){
                database.erase(it);
                SaveDatabase();
                reply="OK\n";
            }else{
                reply="not in database\n";
            }
        }

    }else if(cmd=="EXISTS"){
        if(args.size()!=1){
            reply="ERROR wrong number of arguments.\n";
        
        }else{
            reply = database.count(args[0]) ? "true\n" : "false\n";
        }
    
    }else if(cmd=="SIZE"){
        if(!args.empty()){
            reply="ERROR SIZE takes no arguements\n";
        }else{
            reply=to_string(database.size()) + "\n";
        }
    }else if(cmd=="CLEAR"){
        if(!args.empty()){
            reply="ERROR CLEAR takes no arguements\n";
        }else{
            database.clear();
            SaveDatabase();
            reply="ok\n" ;
        }
    }else if(cmd=="KEYS"){
        if(!args.empty()){
            reply="ERROR KEYS takes no arguements\n";
        }else{
            if(database.empty()){
                reply="Database in empty\n";
            }else{
                for(auto &data:database){
                    reply+=data.first+"\n";
                }
            }
        }
    }else if(cmd=="INCR") {
        if(args.size()!=1) {
            reply="ERROR wrong number of arguments.\n";
        }else{
            auto it=database.find(args[0]);
            if(it==database.end()) {
                database[args[0]]="1";
                reply="1\n";

            }else {
                ////try this code if it doesn't work do not crash just go to catch code instead
                try {//if val integer
                    int value=stoi(it->second);
                    value++;

                    it->second=to_string(value);
                    reply="ok new val: " + it->second + "\n";
                }
                catch(...) {
                    reply="ERROR value is not an integer.\n";
                }
            }
            SaveDatabase();
        }
    }else if (cmd=="QUIT") {
        reply="bye!\n";
        quit=true;
    }else{
        reply="ERR unknown command.\n";
    }
    return {reply,quit};
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
        
        // cout << "read() returned: " << n << endl;
        if(n==0) {
            cout<<"Client with ip and port number : "<<client_ip<<":"<<client_port<<" disconnected "<<endl;
            break;
        }else if(n<0){
            perror("read error");
            break;
        }
        inbuf.append(buffer,n);
        while(true) {
            size_t pos=inbuf.find('\n');

            if(pos==string:: npos) {
                break;//no complete command yet
            }
            string line=inbuf.substr(0,pos);
            inbuf.erase(0,pos+1);//delete processed command
            if(!line.empty() && line.back()=='\r'){
                line.pop_back();
            }
            if(line.empty()) continue;   
            //execution command             
            commandResult result=executeCommand(line);
            sendReply(client_fd,result.reply);

            if(result.quit){
                cout<<"Client with ip and port number : "<<client_ip<<":"<<client_port<<" said  bye bye! "<<endl;
                quit=true;
                break;
            }
        }
    }
    // close(client_fd)
    close(client_fd);
}
int main() {
    int server_fd=createServer();//create a server for handling clients
    LoadDatabase();//load the value in database after starting again

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