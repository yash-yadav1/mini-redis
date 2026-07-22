#pragma once

#include <bits/stdc++.h>

using namespace std;

struct commandResult
{
    string reply;
    bool quit;
};

commandResult executeCommand(const vector<string> &tokens); //function declaration