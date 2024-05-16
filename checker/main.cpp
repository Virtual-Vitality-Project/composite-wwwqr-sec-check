#include <iostream>
#include <fstream>
#include <vector>
#include <cstdio>
#include <string>

std::vector<std::string> fileList;
std::vector<std::string> phpFilter;
std::vector<std::string> warningList;
int returnCode = 0;

void createPhpFilter() {
    std::ifstream conf("phpFilter.wwwqr");
    if (!conf.is_open()) {
        std::cout << "\n\nCould not open phpFilter.wwwqr.\n\n";
        return;
    }
    std::string line;
    while(std::getline(conf, line)) {
        phpFilter.emplace_back(line);
    }
    conf.close();
}

void createFilters() {
    createPhpFilter();
}

std::string exec(const char* cmd) {
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);
    return result;
}

std::string getExtentionName(std::string path) {
    int pos = path.find_last_of('.');
    if (pos != std::string::npos) {
        return path.substr(pos + 1);
    }
    return "";
}

void setFileList() {
    fileList.clear();
    std::string tmpStr = exec("find ../ -type f");
    std::string cache = "";
    for (char c : tmpStr) {
        if (c == '\n') {
            fileList.emplace_back(cache);
            cache = "";
        }
        else {
            cache += c;
        }
    }
}

void checkFiles() {
    for (auto& str : fileList) {
        std::ifstream file(str);
        std::string line;
        if (!file.is_open()) {
            continue;
        }
        //whitelist check
        std::ifstream whitelist("whitelist.wwwqr");

        if (whitelist.is_open()) {
            std::string tmpWLine;
            bool isWhite = false;
            while (std::getline(whitelist, tmpWLine)) {
                if (str == tmpWLine) {
                    isWhite = true;
                    break;
                }
            }
            whitelist.close();
            if (isWhite) {
                continue;
            }
        }
        else {
            std::cout << "\n\nWhitelist.wwwqr file not found.\n\n";
        }
        //
        int lineCount = 0;
        while (std::getline(file, line)) {
            ++lineCount;
            if (getExtentionName(str) == "php") {
                for (auto& val : phpFilter) {
                    int tmpP = line.find(val);
                    if (tmpP != std::string::npos) {
                        warningList.emplace_back("Error at line " + std::to_string(lineCount) + " in '" + str + "' Filter: (" + val + ")\n");
                        if (returnCode == 0) {
                            returnCode = 1;
                        }
                    }
                }
            }
        }
        file.close();
    }
}

int main() {
    setFileList();
    createFilters();
    checkFiles();
    for (auto& str : warningList) {
        std::cout << str;
    }
    return returnCode;
}