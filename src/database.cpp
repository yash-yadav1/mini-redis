#include <bits/stdc++.h> // to include cpp libraries
#include <shared_mutex>//for mutex lock of database

#include "../include/database.h"
#include "../include/resp.h"

using namespace std;

Database database;

//SET value
bool Database::Set(const string &key,const string &value)
{
    unique_lock<shared_mutex> lock(mutex);

    data[key]=value;

    return true;
}

//GET value
bool Database::Get(const string &key,string &value)
{
    shared_lock<shared_mutex> lock(mutex);

    auto it=data.find(key);

    if(it==data.end()){
        return false;
    }

    value=it->second;

    return true;
}

//DELETE value
bool Database::Del(const string &key)
{
    unique_lock<shared_mutex> lock(mutex);

    auto it=data.find(key);

    if(it==data.end()){
        return false;
    }

    data.erase(it);

    return true;
}

//CHECK IF EXISTS
bool Database::Exists(const string &key)
{
    shared_lock<shared_mutex> lock(mutex);

    return data.count(key);
}

//CLEAR DATABASE
void Database::Clear()
{
    unique_lock<shared_mutex> lock(mutex);

    data.clear();
}

//DATABASE SIZE
size_t Database::Size()
{
    shared_lock<shared_mutex> lock(mutex);

    return data.size();
}

//RETURN ALL KEYS
string Database::Keys()
{
    shared_lock<shared_mutex> lock(mutex);

    string ans="";

    for(auto &it:data){
        ans+=it.first+"\n";
    }

    return ans;
}

//INCREMENT VALUE
bool Database::Incr(const string &key,string &reply)
{
    unique_lock<shared_mutex> lock(mutex);

    auto it=data.find(key);

    if(it==data.end()){
        data[key]="1";
        reply = RespInteger(1); 
        return true;
    }

    try{

        int value=stoi(it->second);

        value++;

        it->second=to_string(value);

        reply = RespInteger(value);

        return true;

    }catch(...){

        reply = RespError("value is not an integer");

        return false;
    }
}

//RETURN WHOLE DATABASE
unordered_map<string,string> Database::GetData()
{
    shared_lock<shared_mutex> lock(mutex);

    return data;
}

//LOAD WHOLE DATABASE
void Database::SetData(unordered_map<string,string> loadedData)
{
    unique_lock<shared_mutex> lock(mutex);

    data=loadedData;
}