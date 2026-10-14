#include <bits/stdc++.h> // to include cpp libraries
#include <shared_mutex>//for mutex lock of database

#include "../include/database.h"
#include "../include/resp.h"

using namespace std;

Database database;


//check if expired
bool  Database::isExpired(const string &key){
    auto it=expiry.find(key);
    if(it==expiry.end()){
        return false;
    }
    return chrono::steady_clock:: now() >= it->second;
}


// REMOVE ALL EXPIRED KEYS
// Caller must hold the exclusive lock.
void Database::RemoveExpiredKeys(){
    auto now = chrono::steady_clock::now();

    for (auto it = expiry.begin(); it != expiry.end();){
        if (now >= it->second){
            data.erase(it->first);
            it = expiry.erase(it);
        }else{
            it++;
        }
    }
}


//SET value
bool Database::Set(const string &key,const string &value)
{
    unique_lock<shared_mutex> lock(mutex);

    data[key]=value;

    // A normal SET removes any previous expiration.
    expiry.erase(key);

    return true;
}

//GET value
bool Database::Get(const string &key,string &value)
{
    unique_lock<shared_mutex> lock(mutex);

    if (isExpired(key)){
        data.erase(key);
        expiry.erase(key);
        return false;
    }

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

    if (isExpired(key)){
        data.erase(key);
        expiry.erase(key);
        return false;
    }

    auto it=data.find(key);

    if(it==data.end()){
        return false;
    }

    data.erase(it);
    expiry.erase(key);

    return true;
}

//CHECK IF EXISTS
bool Database::Exists(const string &key)
{
    unique_lock<shared_mutex> lock(mutex);

    if (isExpired(key)){
        data.erase(key);
        expiry.erase(key);
    }

    return data.count(key);
}

//CLEAR DATABASE
void Database::Clear()
{
    unique_lock<shared_mutex> lock(mutex);

    data.clear();
    expiry.clear();
}

//DATABASE SIZE
size_t Database::Size()
{
    unique_lock<shared_mutex> lock(mutex);

    RemoveExpiredKeys();

    return data.size();
}

//RETURN ALL KEYS
string Database::Keys()
{
    unique_lock<shared_mutex> lock(mutex);

    RemoveExpiredKeys();

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

    if (isExpired(key)){
        data.erase(key);
        expiry.erase(key);
    }

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
    unique_lock<shared_mutex> lock(mutex);

    RemoveExpiredKeys();

    return data;
}

//LOAD WHOLE DATABASE
void Database::SetData(unordered_map<string,string> loadedData)
{
    unique_lock<shared_mutex> lock(mutex);

    data=std::move(loadedData);
    expiry.clear();
}

// SET EXPIRATION TIME
bool Database::Expire(const string &key, int seconds) {
    unique_lock<shared_mutex> lock(mutex);

    if (isExpired(key)){
        data.erase(key);
        expiry.erase(key);
        return false;
    }

    auto it = data.find(key);
    if (it == data.end()) {
        return false;
    }

    // Non-positive expiration deletes the key immediately.
    if (seconds <= 0){
        data.erase(it);
        expiry.erase(key);
        return true;
    }
    expiry[key] =chrono::steady_clock::now() + chrono::seconds(seconds);
    return true;
}


// GET REMAINING TTL IN SECONDS
long long Database::TTL(const string &key){
    unique_lock<shared_mutex> lock(mutex);

    if (isExpired(key)){
        data.erase(key);
        expiry.erase(key);
        return -2;
    }

    auto it = data.find(key);
    if (it == data.end()) {
        return -2;
    }

    auto expiryIt = expiry.find(key);
    // Key exists, but has no expiration.
    if (expiryIt == expiry.end()) {
        return -1;
    }

    long long remaining =chrono::duration_cast<chrono::seconds>( expiryIt->second - chrono::steady_clock::now() ).count();
    //check epxiry time in map w.r.t current time
    return max(0LL, remaining);
}


// REMOVE EXPIRATION
bool Database::Persist(const string &key) {
    unique_lock<shared_mutex> lock(mutex);

    if (isExpired(key)){
        data.erase(key);
        expiry.erase(key);
        return false;
    }

    auto it = data.find(key);
    if (it == data.end()) {
        return false;
    }

    // Return true only if an expiration actually existed.
    return expiry.erase(key) > 0;
}


//GetexpiryData
unordered_map<string,chrono::system_clock::time_point> Database::GetExpiryData() {

    unique_lock<shared_mutex> lock(mutex);

    unordered_map<string, chrono::system_clock::time_point> result;

    auto steadyNow = chrono::steady_clock::now();
    auto systemNow = chrono::system_clock::now();

    for (const auto &entry : expiry) {
        auto remaining = entry.second - steadyNow;

        if (remaining > chrono::steady_clock::duration::zero()) {
            result[entry.first] = systemNow +
                chrono::duration_cast<chrono::system_clock::duration>(remaining);
        }
    }

    return result;
}

void Database::SetExpiryData( unordered_map<string, chrono::system_clock::time_point> loadedExpiry ) {

    unique_lock<shared_mutex> lock(mutex);

    auto systemNow = chrono::system_clock::now();
    auto steadyNow = chrono::steady_clock::now();

    for (const auto &entry : loadedExpiry) {
        if (entry.second > systemNow && data.count(entry.first)) {
            auto remaining = entry.second - systemNow;

            expiry[entry.first] = steadyNow +
                chrono::duration_cast<chrono::steady_clock::duration>(remaining);
        }
    }
}
