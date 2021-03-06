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


void fillGap(DayEntry prev, DayEntry finish,  std::ofstream& ofstream, int* blank);
void processFile(const std::filesystem::path& pathIn, const std::filesystem::path& pathOut);

std::filesystem::path pathIn, pathOut;
int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "invalid input...";
        return 1;
    }
    pathIn = argv[1];
    pathOut = argv[2];

    if (is_directory(pathIn)) {
        std::cout << "Processing all files in directory...\n";
        for (const auto& entry : std::filesystem::directory_iterator(pathIn)) {
            std::cout << "processing file: " << entry.path() << "\n";
            processFile(entry.path(), pathOut / entry.path().filename());
        }
    } else {
        std::cout << "Processing single file...\n";
        processFile(pathIn, pathOut / pathIn.filename());
    }


    return 0;
}

void processFile(const std::filesystem::path& pathIn, const std::filesystem::path& pathOut) {
    std::vector<DayEntry> entries{};
    std::string line;
    std::ifstream file(pathIn);
    std::ofstream outFile;
    outFile.open(pathOut);
    outFile << "Date,,,Sensor1,Sensor2,Sensor3\n";
    auto skipFirst = false;
    DayEntry prev = {"", "", "", ""};
    if (file.is_open()) {
        auto start = chrono::high_resolution_clock::now();
        int row= 0;
        int blank = 0;
        while (std::getline(file, line)) {
            row++;
            if (!skipFirst) {
                skipFirst = true;
                continue;
            }
            auto elements = split(line, ',');

            DayEntry dayEntry{elements[0],
                              elements[1],
                              elements[2],
                              elements[3]
            };
            // check if we even have a previous, if not skip checking..
            if (!prev.date_raw.empty()) {

                auto hour_diff = (mktime(&dayEntry.date) / 3600) - (mktime(&prev.date) / 3600);

                if (prev.date.tm_hour != dayEntry.date.tm_hour) {
                    prev.to_file(outFile, false);
                }

                if (hour_diff > 1) {
                    std::cout << "====================================================\n";
                    std::cout << "missing! difference between row " << row -1 << " and row " << row << ": " <<hour_diff << "h(s)\n";
                    std::cout << prev.to_string() << " vs " << dayEntry.to_string() << "\n";
                    fillGap(prev, dayEntry, outFile, &blank);
                }

            }
            prev = dayEntry;
        }
        auto stop = chrono::high_resolution_clock::now();
        auto duration = duration_cast<chrono::milliseconds>(stop - start);
        std::cout << "finished " << row << " rows in " << duration.count() << "ms; " << blank << " blank rows were inserted.\n";
    } else throw std::invalid_argument("unable to open file");
}

void fillGap(DayEntry prev, DayEntry finish, std::ofstream& ofstream, int* blank) {
    std::vector<DayEntry> gapEntries{};
    while ((mktime(&finish.date) / 3600) - (mktime(&prev.date) / 3600) > 1) {
//        prev.date.tm_hour++;
        time_t timePrev = mktime(&prev.date);
        timePrev += (60 * 60);
        prev.date = *localtime(&timePrev);
        // this can just throw it into the file output stream later
        prev.to_file(ofstream, true);
        ++*blank;
        std::cout << "missing date was: " << prev.to_string() << "\n";
    }
}


