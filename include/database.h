#pragma once

#include <bits/stdc++.h>
#include <shared_mutex>

using namespace std;

class Database
{
private:
    unordered_map<string,string> data;
    shared_mutex mutex;

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
};

extern Database database;//tell others i'm present somewhere use me