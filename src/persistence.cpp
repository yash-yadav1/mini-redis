#include <bits/stdc++.h>// to include cpp libraries
#include "../include/persistence.h"
#include "../include/database.h"

using namespace std;

const string DATABASE_FILE = "data/database.txt";

//Save database into file
void SaveDatabase()
{
    ofstream fout(DATABASE_FILE);

    unordered_map<string,string> temp = database.GetData();

    for(auto &data : temp)
    {
        fout << data.first << " " << data.second << "\n";
    }
}

//Load database from file
void LoadDatabase()
{
    ifstream fin(DATABASE_FILE);

    unordered_map<string,string> temp;

    string key;
    string value;

    while(fin >> key >> value)
    {
        temp[key] = value;
    }

    database.SetData(temp);
}