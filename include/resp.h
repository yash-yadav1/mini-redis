#pragma once//consider header files only once to avoid errors

#include <bits/stdc++.h>

using namespace std;

struct RespResult
{
    vector<string> tokens;
    int bytesConsumed;
};
// Parse a RESP command into tokens
RespResult ParseRESP(const string &message);

// Reply formatters
string RespSimpleString(const string &msg);//for short status like ok 

string RespBulkString(const string &msg);//sending actual data

string RespInteger(long long value);//sending integer values

string RespError(const string &msg);//sending respective error messages 

string RespNull();//either missing key or 

