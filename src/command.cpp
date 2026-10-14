#include <bits/stdc++.h>// to include cpp libraries

#include "../include/command.h"
#include "../include/database.h"
#include "../include/persistence.h"
#include "../include/resp.h"

using namespace std;


commandResult executeCommand(const vector<string> & tokens) { //const to not modify the lines
    if(tokens.empty()) {
        return {RespError("Protocol error"), false};
    }
    string cmd=tokens[0];
    transform(cmd.begin(),cmd.end(),cmd.begin(),::toupper);
    //store information 
    vector<string> args;
    for(int i = 1; i < tokens.size(); i++){
        args.push_back(tokens[i]);
    }

    //execute the commands
    string reply;
    bool quit=false;

    if(cmd=="PING") {
        reply=RespSimpleString("PONG");

    }else if(cmd=="SET"){
        if(args.size()!=2){
            reply = RespError("wrong number of arguments");
        }else{
            database.Set(args[0],args[1]);
            SaveDatabase();
            reply = RespSimpleString("OK");
        }

    }else if(cmd=="GET") {
        if(args.size()!=1){
            reply = RespError("wrong number of arguments");
        }else {
            string value;
            if(database.Get(args[0],value)){
                reply = RespBulkString(value);
            }else{
                reply = RespNull();
            }
        }

    }else  if(cmd=="DEL"){
        if(args.size()!=1){
            reply = RespError("wrong number of arguments");
        }else{
            if(database.Del(args[0])){
                SaveDatabase();
                reply = RespSimpleString("OK");
            }else{
                reply = RespNull();
            }
        }

    }else if(cmd=="EXISTS"){
        if(args.size()!=1){
            reply = RespError("wrong number of arguments");
        
        }else{
            reply=database.Exists(args[0]) ? RespSimpleString("true"):RespSimpleString("false");
        }
    
    }else if(cmd=="SIZE"){
        if(!args.empty()){
            reply = RespError("wrong number of arguments");
        }else{

            reply = RespInteger(database.Size());
        }
    }else if(cmd=="CLEAR"){
        if(!args.empty()){
            reply = RespError("wrong number of arguments");
        }else{

            database.Clear();
            SaveDatabase();
            reply = RespSimpleString("OK");
        }
    }else if(cmd=="KEYS"){
        string keys = database.Keys();
        if(keys.empty())
        {
            reply = RespBulkString("Database is empty");
        }
        else
        {
            reply = RespBulkString(keys);
        }
    }else if(cmd=="INCR") {
        if(args.size()!=1) {
            reply = RespError("wrong number of arguments");
        }else{
            if(database.Incr(args[0],reply)){
                SaveDatabase();
            }
        }
    }else if (cmd=="QUIT") {
        reply = RespSimpleString("BYE!");
        quit=true;
    }else if(cmd == "EXPIRE") {
        if(args.size() != 2) {
            reply = RespError("wrong number of arguments");
        }else {
            try {
                int seconds = stoi(args[1]);

                bool success = database.Expire(args[0], seconds);
                reply = RespInteger(success ? 1 : 0);

                if(success) {
                    SaveDatabase();
                }
            }catch(...) {
                reply = RespError("invalid expiration time");
            }
        }

    } else if(cmd == "TTL") {
        if(args.size() != 1) {
            reply = RespError("wrong number of arguments");
        }else {
            long long seconds = database.TTL(args[0]);
            reply = RespInteger(seconds);
        }

    } else if(cmd == "PERSIST") {
        if(args.size() != 1) {
            reply = RespError("wrong number of arguments");
        }else {
            bool success = database.Persist(args[0]);
            reply = RespInteger(success ? 1 : 0);

            if(success) {
                SaveDatabase();
            }
        }
    } else if(cmd == "CONFIG") {

        if(args.size() != 2) {
            reply = RespError("wrong number of arguments");
        }
        else if(args[0] == "GET") {

            if(args[1] == "save") {
                reply =
                    "*2\r\n"
                    "$4\r\nsave\r\n"
                    "$0\r\n\r\n";
            }
            else if(args[1] == "appendonly") {
                reply =
                    "*2\r\n"
                    "$10\r\nappendonly\r\n"
                    "$2\r\nno\r\n";
            }
            else {
                reply = "*0\r\n";
            }
        }
        else {
            reply = RespError("unsupported CONFIG");
        }
    }
    else{
        reply = RespError("unknown command");
    }
    return {reply,quit};
}