#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <ctime>
#include "./struct/util.h"
#include "./struct/DayEntry.h"

using namespace std::chrono_literals;
namespace chrono = std::chrono;


void fillGap(DayEntry prev, DayEntry finish,  std::ofstream& ofstream);
void processFile(const std::filesystem::path& pathIn, const std::filesystem::path& pathOut);

std::filesystem::path dirIN, dirOUT;
int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "invalid input...";
        return 1;
    }
    dirIN = argv[1];
    dirOUT = argv[2];



    for (const auto& entry : std::filesystem::directory_iterator(dirIN)) {
        std::cout << "processing file: " << entry.path() << "\n";
        processFile(entry.path(), dirOUT / entry.path().filename());
    }
    return 0;
}

void processFile(const std::filesystem::path& pathIn, const std::filesystem::path& pathOut) {
    std::vector<DayEntry> entries{};
    std::string line;
    std::ifstream file(pathIn);
    std::ofstream outFile;
    outFile.open(pathOut);
    auto skipFirst = false;
    DayEntry prev = {"", 0, 0, 0};
    if (file.is_open()) {
        auto start = chrono::high_resolution_clock::now();
        while (std::getline(file, line)) {
            if (!skipFirst) {
                skipFirst = true;
                continue;
            }
            auto elements = split(line, ',');
            DayEntry dayEntry{elements[0],
                              std::stoi(elements[1]),
                              std::stoi(elements[2]),
                              std::stoi(elements[3])};

            // check if we even have a previous, if not skip checking..
            if (!prev.date_raw.empty()) {


                prev.to_file(outFile, false);

                if ((mktime(&dayEntry.date) / 3600) - (mktime(&prev.date) / 3600) > 1) {
                    std::cout << "====================================================\n";
                    std::cout << "missing! difference between last and this row: " << (mktime(&dayEntry.date) / 3600) - (mktime(&prev.date) / 3600) << "h(s)\n";
                    std::cout << prev.to_string() << " vs " << dayEntry.to_string() << "\n";
                    fillGap(prev, dayEntry, outFile);
                }
            }
            prev = dayEntry;
        }
        auto stop = chrono::high_resolution_clock::now();
        auto duration = duration_cast<chrono::milliseconds>(stop - start);
        std::cout << "finished " << duration.count() << "ms\n";
    } else throw std::invalid_argument("unable to open file");
}

void fillGap(DayEntry prev, DayEntry finish, std::ofstream& ofstream) {
    std::vector<DayEntry> gapEntries{};
    while ((mktime(&finish.date) / 3600) - (mktime(&prev.date) / 3600) > 1) {
//        prev.date.tm_hour++;
        time_t timePrev = mktime(&prev.date);
        timePrev += (60 * 60);
        prev.date = *localtime(&timePrev);
        // this can just throw it into the file output stream later
        prev.to_file(ofstream, true);
        std::cout << "missing date was: " << prev.to_string() << "\n";
    }
}

