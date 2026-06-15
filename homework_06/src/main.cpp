#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "ballistics.hpp"

char* getOutputPath(const char* inputPath, char* fileName);

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: input data check <input_path>\n";
        return 1;
    }

    std::ifstream input{argv[1]};

    if (!input) {
        std::cerr << "error: failed to open input file: " << argv[1] << '\n';
        return 0;
    }

    std::cout << "Home Task Lesson 6\n";
    std::cout << "Input file path:\n";
    std::cout << "Opening: " << argv[1] << "\n";
    int count = 1;
    BallisticInputData* inputData = count > 0 ? new BallisticInputData[count] : nullptr;
    readData(argv[1], inputData, count);
    AmmoData ammoData = findAmmoType(inputData[0].name);
    std::cout << "Ammo data: m: " << ammoData.m << " d: " << ammoData.d << " l: " << ammoData.l << "\n";
    std::cout << "In: Position: (" << inputData[0].xd << "; " << inputData[0].yd << ";"<< inputData[0].zd <<") / ";
    std::cout << "Target: (" << inputData[0].targetX << "; " << inputData[0].targetY << ") / ";
    std::cout << "name: " << inputData[0].name << " / ";
    std::cout << "attackSpeed: " << inputData[0].attackSpeed << " / ";
    std::cout << "accelerationPath: " << inputData[0].accelerationPath << " \n";
    BallisticOutputData outputData = calculateFirePoint(inputData[0], ammoData);
    writeData(getOutputPath(argv[1], "output.txt"), outputData);
    delete[] inputData;
    
    return EXIT_SUCCESS;
}

char* getOutputPath(const char* inputPath, char* fileName) {
    char dirPath[MAX_LINE_LENGTH];
      std::strncpy(dirPath, inputPath, MAX_LINE_LENGTH - 1);
      dirPath[MAX_LINE_LENGTH - 1] = '\0';
  
      char* lastSlash = std::strrchr(dirPath, '/');
      if (lastSlash != nullptr) {
          *(lastSlash + 1) = '\0';
      } else { 
          dirPath[0] = '.';
          dirPath[1] = '/';
          dirPath[2] = '\0';
      }
  
      static char outputPath[MAX_LINE_LENGTH];
      std::snprintf(outputPath, MAX_LINE_LENGTH, "%s%s", dirPath, fileName);
      return outputPath;
}