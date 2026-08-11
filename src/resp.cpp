#include "../include/resp.h"//include declaration header file 

using namespace std;

RespResult ParseRESP(const string &message) {
    if (message.empty()) return {{}, 0}; // nothing yet, wait for more data

    vector<string> tokens;
    if (message[0] != '*') return {{}, -1}; // not RESP array -> protocol error

    size_t pos = 1;
    long long arg_count = 0;

    while (pos < message.size() && message[pos] != '\r') {
        if (!isdigit(message[pos])) return {{}, -1};
        arg_count = arg_count * 10 + (message[pos] - '0');
        pos++;
    }
    if (pos + 1 >= message.size()) return {{}, 0}; // \r\n not fully arrived yet
    pos += 2;

    for (long long i = 0; i < arg_count; i++) {
        if (pos >= message.size()) return {{}, 0};      // waiting for more

        if (message[pos] != '$') return {{}, -1};        // malformed

        pos++;

        long long word_len = 0;
        while (pos < message.size() && message[pos] != '\r') {
            if (!isdigit(message[pos])) return {{}, -1};
            word_len = word_len * 10 + (message[pos] - '0');
            pos++;
        }
        if (word_len < 0 || word_len > 512 * 1024 * 1024) return {{}, -1};//if too much length of word just return malformed data

        if (pos + 1 >= message.size()) return {{}, 0};
        if (message[pos] != '\r' || message[pos + 1] != '\n'){
            return {{}, -1};
        }
        pos += 2;
        
        
        if (pos + word_len + 2 > message.size()) return {{}, 0}; // incomplete

        tokens.push_back(message.substr(pos, word_len));
        if (message[pos + word_len] != '\r' ||
            message[pos + word_len + 1] != '\n')
        {
            return {{}, -1};
        }
        pos += word_len + 2;
    }

    return {tokens, (int)pos};
}


// Format: +OK\r\n
string RespSimpleString(const string &msg) {
    return "+" + msg + "\r\n";
}

// Format: $<length>\r\n<data>\r\n
string RespBulkString(const string &msg) {
    return "$" + to_string(msg.size()) + "\r\n" + msg + "\r\n";
}

// Format: :123\r\n
string RespInteger(long long value) {
    return ":" + to_string(value) + "\r\n";
}

// Format: -ERR something went wrong\r\n
string RespError(const string &msg) {
    return "-ERR " + msg + "\r\n";
}

// format : $-1\r\n
string RespNull() {
    return "$-1\r\n";
}