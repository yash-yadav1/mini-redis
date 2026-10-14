#include <bits/stdc++.h>// to include cpp libraries
#include "../include/persistence.h"
#include "../include/database.h"

using namespace std;

const string DATABASE_FILE = "data/database.txt";

//Save database into file
void SaveDatabase()
{
    ofstream fout(DATABASE_FILE);

    if (!fout.is_open()) {
        cerr << "Failed to open database file for saving\n";
        return;
    }

    unordered_map<string,string> temp = database.GetData();
    unordered_map<string,chrono::system_clock::time_point> expirations=database.GetExpiryData();

    for(auto &entry : temp) {
        fout << "D "<< entry.first << " " << entry.second << "\n";
    }

    for(auto &entry : expirations) {
        auto timestamp = chrono::duration_cast<chrono::seconds>(entry.second.time_since_epoch() ).count();

        fout << "E " << entry.first << " " << timestamp << "\n";
    }
    
}

//Load database from file
void LoadDatabase()
{
    ifstream fin(DATABASE_FILE);

    unordered_map<string,string> temp;
    unordered_map<string,chrono::system_clock::time_point> expirations;

    string type;
    string key;
    string value;

    while (fin >> type) {
        if (type == "D") {//type = D means DATA
            if (fin >> key >> value) {
                temp[key] = value;
            }
        }
        else if (type == "E") { // type = E means ExpiryData
            long long timestamp;

            if (fin >> key >> timestamp) {
                expirations[key] = chrono::system_clock::time_point(
                    chrono::seconds(timestamp)
                );
            }
        }
        else {
            // Unknown record type: stop rather than misread the file.
            break;
        }
    }



    auto now = chrono::system_clock::now();

    for (auto it = temp.begin(); it != temp.end();) {
        auto expiryIt = expirations.find(it->first);

        if (expiryIt != expirations.end() && expiryIt->second <= now) {
            expirations.erase(expiryIt);
            it = temp.erase(it);
        } else {
            ++it;
        }
    }
    database.SetData(std::move(temp));
    database.SetExpiryData(std::move(expirations));
}