#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>

using namespace std;

struct Rang {
    double l;
    double r;
};

int main(int argc, char* argv[])
{
    if (argc != 3) {
        cout << "Usage:\n";
        cout << "./lab4 -z file\n";
        cout << "./lab4 -u archive\n";
        return 1;
    }

    string cmd = argv[1];
    string name = argv[2];

    if (cmd == "-z" || cmd == "-zip") {

        ifstream in(name, ios::binary);
        if (!in) {
            cout << "File not found\n";
            return 1;
        }

        vector<unsigned char> data;
        char c;

        while (in.get(c)) {
            data.push_back((unsigned char)c);
        }

        in.close();

        map<unsigned char, int> freq;
        for (int i = 0; i < (int)data.size(); i++) {
            freq[data[i]]++;
        }
        map<unsigned char, Rang> inter;
        double border = 0.0;

        for (auto it = freq.begin(); it != freq.end(); ++it) {
            double p = (double)it->second / (double)data.size();

            Rang in;
            in.l = border;
            in.r = border + p;

            inter[it->first] = in;
            border += p;
        }

        double low = 0.0, high = 1.0;

        for (int i = 0; i < (int)data.size(); i++) {
            unsigned char sym = data[i];
            double range = high - low;
            double newL = low + range * inter[sym].l;
            double newR = low + range * inter[sym].r;
            low = newL;
            high = newR;
        }

        double code = (low + high) / 2.0;

        ofstream out(name + ".mmm", ios::binary);

        size_t sz = data.size();
        out.write((char*)&sz, sizeof(sz));

        size_t len = name.size();
        out.write((char*)&len, sizeof(len));
        out.write(name.c_str(), len);

        int fsize = freq.size();
        out.write((char*)&fsize, sizeof(fsize));

        for (auto it = freq.begin(); it != freq.end(); ++it) {
            unsigned char sym = it->first;
            int cnt = it->second;

            out.write((char*)&sym, 1);
            out.write((char*)&cnt, sizeof(int));
        }

        out.write((char*)&code, sizeof(code));

        out.close();

        cout << "Archive created\n";
    }
    else if (cmd == "-u" || cmd == "-unzip") {

        ifstream in(name, ios::binary);
        if (!in) {
            cout << "Archive not found\n";
            return 1;
        }

        size_t sz;
        in.read((char*)&sz, sizeof(sz));
        size_t len;
        in.read((char*)&len, sizeof(len));
        string orig;
        orig.resize(len);
        in.read(&orig[0], len);
        int fsize;
        in.read((char*)&fsize, sizeof(fsize));
        map<unsigned char, int> freq;

        for (int i = 0; i < fsize; i++) {
            unsigned char sym;
            int cnt;

            in.read((char*)&sym, 1);
            in.read((char*)&cnt, sizeof(int));

            freq[sym] = cnt;
        }

        double code;
        in.read((char*)&code, sizeof(code));

        map<unsigned char, Rang> inter;
        double border = 0.0;

        for (auto it = freq.begin(); it != freq.end(); ++it) {
            double p = (double)it->second / (double)sz;

            Rang in;
            in.l = border;
            in.r = border + p;
            inter[it->first] = in;
            border += p;
        }

        vector<unsigned char> result;
        double val = code;

        for (int i = 0; i < (int)sz; i++) {

            for (auto it = inter.begin(); it != inter.end(); ++it) {
                if (val >= it->second.l && val < it->second.r) {

                    result.push_back(it->first);

                    val = (val - it->second.l) /
                          (it->second.r - it->second.l);

                    break;
                }
            }
        }

        ofstream out(orig, ios::binary);

        for (int i = 0; i < (int)result.size(); i++) {
            out.put((char)result[i]);
        }

        out.close();

        cout << "File restored: " << orig << endl;
    }
    else {
        cout << "Unknown command\n";
    }

    return 0;
}