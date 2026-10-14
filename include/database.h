#pragma once

#include <bits/stdc++.h>
#include <shared_mutex>
#include <chrono>
using namespace std;

class Database
{
private:
    unordered_map<string,string> data;

    // Stores the expiration time for keys that have a TTL.
    unordered_map<string, chrono::steady_clock::time_point> expiry;
    shared_mutex mutex;

    // Must be called while holding the appropriate lock.
    bool isExpired(const string &key);

    void RemoveExpiredKeys();

public:

    bool Set(const string &key,const string &value);

    bool Get(const string &key,string &value);

    bool Del(const string &key);

    bool Exists(const string &key);

    void Clear();

    size_t Size();

    string Keys();

    bool Incr(const string &key,string &reply);

    unordered_map<string,string> GetData();

    void SetData(unordered_map<string,string> loadedData);

    //new TTL commands 
    bool Expire(const string &key,int seconds);

    long long TTL(const string &key);

    bool Persist(const string &key);

    //expiry commands persistance
    unordered_map<string,chrono::system_clock::time_point> GetExpiryData();
    void SetExpiryData(unordered_map<string,chrono::system_clock::time_point> loadedExpiry);

};

extern Database database;//tell others i'm present somewhere use me