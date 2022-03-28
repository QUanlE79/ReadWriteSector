#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

bool ReadSector(const char* _disk, char*& _buff, unsigned int _startsect, int _nsect){
    DWORD dwRead;
    HANDLE hDisk = CreateFileA(_disk, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (hDisk == INVALID_HANDLE_VALUE){ // this may happen if another program is already reading from disk
        CloseHandle(hDisk);
        cout << "ERROR - 1 | another program is already using disk" << endl;
        return 1;
    }
    SetFilePointer(hDisk, _startsect * 512, 0, FILE_BEGIN);
    ReadFile(hDisk, _buff, _nsect * 512, &dwRead, 0);
    CloseHandle(hDisk);
    return 0;
}
bool WriteSector(const char* _disk, char*& _buff, unsigned int _nsect){
    DWORD dwWrite;
    HANDLE hDisk = CreateFileA(_disk, GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    if (hDisk == INVALID_HANDLE_VALUE){ // this may happen if another program is already reading from disk
        CloseHandle(hDisk);
        cout << "ERROR - 1 | another program is already using disk" << endl;
        return 1;
    }
    SetFilePointer(hDisk, _nsect * 512, 0, FILE_BEGIN);
    WriteFile(hDisk, _buff, 512, &dwWrite, 0);
    CloseHandle(hDisk);
    return 0;
}

long long convertToInteger(char* buff) {
    string tmp="";
    int i = 0;
    while(buff[i] != '\0') {
        tmp += buff[i++];
    }
    long long res = 0;
    if (tmp[0] == '-'){
        for (int i = 1; i < tmp.length(); i++){
            if (tmp[i] >= '0' && tmp[i] <= '9')
                res = res*16 + ((int)tmp[i] - 48);
            else res = res*16 + ((int)tmp[i] - 87);
        }
        return - res;
    }
    for (int i = 0; i < tmp.length(); i++){
        if (tmp[i] >= '0' && tmp[i] <= '9')
            res = res*16 + ((int)tmp[i] - 48);
        else res = res*16 + ((int)tmp[i] - 87);
    }
    return res;
}
bool ReadIntFromSect(const char* disk, int sector,long long& intNum, int offset){
    char* buff = new char[512];
    if (ReadSector(disk, buff, sector, 1) == 1) return 1;
    char* num = new char [512];
    int j = 0;
    for (int i = offset; i < 512;i++){
        if (buff[i]!='\0'){
            num[j++] = buff[i];
        }else{
            num[j++] = buff[i];
            break;
        }
    }
    intNum = convertToInteger(num);
    delete[] num;
    delete[] buff;
    return 0;

}

char* convertToArrChar(int base, long long number, int& size) {
    char digits[]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
    vector <char> numHex;
    long long tmp = abs(number);
    while(tmp>0){
        numHex.push_back(digits[tmp%base]);    
        tmp = tmp/base;
    }
    if (number < 0) numHex.push_back('-');
    reverse (numHex.begin(),numHex.end());
    char* buff = new char[numHex.size() + 1];
    for (int i = 0; i < numHex.size(); i++) {
        buff[i] = numHex[i];
    }
    buff[numHex.size()] = '\0';
    size = numHex.size() + 1;
    return buff;
}
bool WriteIntToSect(const char* disk, int sector,long long intNum, int offset) {
    int size = 0;
    char* num = convertToArrChar(16, intNum, size);
    char* buff = new char[512];
    if (ReadSector(disk, buff, sector, 1) == 1) return 1;
    if (offset > 512){
        cout << "Error - 1 | Offset must less than 512"<<endl;
        return 1;
    }
    int space = 0;
    for (int i = offset; i < 512;i++){
        if (buff[i] != 0){
            space = i - offset;
            break;
        }
        if (i == 511) space = 512 - offset;
    }
    if (space < size){
        cout << "The area has available data may be writen! Do you want to continue? (Y/N):";
        char choice;
        cin >> choice;
        if (choice == 'Y') {
            int j = 0;
            for (int i = offset; i < offset + size;i++){
                buff[i] = num[j++];
            }
            WriteSector(disk, buff, sector);
            return 0;
        }
        else return 2;
    }
    int j = 0;
    for (int i = offset; i < offset + size;i++){
        buff[i] = num[j++];
    }
    delete[] num;
    WriteSector(disk, buff, sector);
    delete[] buff;
    return 0;
}

void __init__(string &disk, int &nsector){
    cout << "Enter disk name (ex: C, D, E,...): ";
    string name; cin >> name;
    disk = "\\\\.\\" + name + ":";
    cout << "Start from sector: ";
    cin >> nsector;
}
int main(){
    int choice = 9;
    int nsector = 0;
    string disk;
    while (choice != 0){
        if (choice == 1){
            char* buff = new char[512];
            cout << "Enter data: ";
            cin.ignore(1);
            cin.getline(buff, 512);
            __init__(disk, nsector);
            const char* _disk = disk.c_str();
            if (WriteSector(_disk,buff,nsector) == 0) {
                cout << "Write complete!" << endl;
                delete[] buff;
            }
            else {
                delete[] buff;
            }
        }
        if (choice == 2){
            __init__(disk, nsector);
            cout << "Enter the number of sector you want to read: ";
            int nsect;
            cin >> nsect;
            const char* _disk = disk.c_str();
            char* buff = new char[512 * nsect];
            if (ReadSector(_disk,buff,nsector,nsect) == 0){
                if (nsect > 1) cout << "Data from sector " << nsector << " to sector "<<
                                    nsector + nsect - 1 << " is: " << buff << endl;
                else cout << "Data from sector " << nsector << " is: "<< buff << endl;
                delete[] buff;
            }
            else {
                delete[] buff;
            }    
        }
        if (choice == 3){
            long long num;
            cin.ignore(1);
            cout << "Enter an integer: ";
            cin >> num;
            __init__(disk, nsector);
            const char* _disk = disk.c_str();
            cout << "Enter start offset of sector "<< nsector <<" :";
            int offset;
            cin >> offset;
            if ((WriteIntToSect(_disk, nsector, num, offset) == 0)) cout << "Write complete!" << endl;   
        }
        if (choice == 4){
            __init__(disk, nsector);
            const char* _disk = disk.c_str();
            long long num;
            cout << "Enter start offset of sector "<< nsector <<" :";
            int offset;
            cin >> offset;
            if (ReadIntFromSect(_disk, nsector, num, offset) == 0) 
                cout << "Integer from sector "<< nsector << " is: "<< num << endl;
        }
        if (choice == 5){
            system("cls");
        }
        if (choice == 0){
            break;
        }
        cout << "--------------------------------------------------------" << endl;
        cout << " _____________________________________" << endl;
        cout << "|  [1] - Write Sector                 |" << endl;
        cout << "|  [2] - Read Sector                  |" << endl;
        cout << "|  [3] - Write an integer to sector   |" << endl;
        cout << "|  [4] - Read an integer from sector  |" << endl;
        cout << "|  [5] - Clear screen                 |" << endl;
        cout << "|  [0] - Exit program                 |" << endl;
        cout << "|_____________________________________|" << endl;
        cout << "Your choice: ";
        cin >> choice;
    }
    getchar();
}