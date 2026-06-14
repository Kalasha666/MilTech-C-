//
//  main.cpp
//  MilTech-C-1
//
//  Created by Oleh Kalashnyk on 09.04.2026.
//

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <filesystem>

std::string toUpper(std::string str);

int main(int argc, const char * argv[]) {
    const float g = 9.81;
    std::cout << "Home Task Lesson 2\n";
    std::cout << "Input file path:\n";
    std::string inputFilePath;
    std::getline(std::cin, inputFilePath);
    std::filesystem::path filePath = std::filesystem::path(inputFilePath);
    std::cout << "Opening: " << filePath << "\n";
    std::ifstream inFile(filePath);
    
    if (!inFile || !inFile.is_open()) {
        std::cerr << "error: Could not open: " << inputFilePath << "\n";
        return EXIT_FAILURE;
    }
    
    std::string line;

    while (std::getline(inFile, line)) {
        std::cout << line << "\n";
    }
    
    std::istringstream iss(line);
    float xd, yd, zd, targetX, targetY, attackSpeed, accelerationPath;
    std::string name;
    iss >> xd >> yd >> zd >> targetX >> targetY >> attackSpeed >> accelerationPath >> name;
    name = toUpper(name);
    float m, d, l;
    
    if(strcmp(name.c_str(), "VOG-17") == 0)
    {
        m = 0.35f;
        d = 0.07f;
        l = 0.0f;
    }
    else if(strcmp(name.c_str(), "M67") == 0)
    {
        m = 0.6f;
        d = 0.1f;
        l = 0.0f;
    }
    else if(strcmp(name.c_str(), "RKG-3") == 0)
    {
        m = 1.2f;
        d = 0.1f;
        l = 0.0f;
    }
    else if(strcmp(name.c_str(), "GLIDING-VOG") == 0)
    {
        m = 0.45f;
        d = 0.1f;
        l = 1.0f;
    }
    else if(strcmp(name.c_str(), "GLIDING-RKG") == 0)
    {
        m = 1.4f;
        d = 0.1f;
        l = 1.0f;
    }
    else
    {
        std::cout << "error: Unknown type: " << name << "\n";
        
        return EXIT_FAILURE;
    }
    
    std::cout << "In: Position: (" << xd << "; " << yd << ";"<< zd <<") / ";
    std::cout << "Target: (" << targetX << "; " << targetY << ") / ";
    std::cout << "name: " << name << " / ";
    std::cout << "attackSpeed: " << attackSpeed << " / ";
    std::cout << "accelerationPath: " << accelerationPath << " \n";
    
    float a = d * g * m - 2 * d * d * l * attackSpeed;
    float b = - 3 * g * m*m + 3 * d * l * m * attackSpeed;
    float c = 6 * m * m * zd;
    float p = (- b * b) / (3 * a * a);
    float q = (2 * b * b * b) / (27 * a * a * a) + c / a;
    float arcArg = (3 * q) / (2 * p) * sqrt(-3 / p);
    
    if(arcArg < -1 || arcArg > 1)
    {
        std::cout << "error: acos argument isn't in the range [-1; 1]: " << arcArg << "\n";
        
        return EXIT_FAILURE;
    }
    
    float fi = acos(arcArg);
    float t = 2 * sqrt(-p / 3) * cos((fi + 4 * M_PI) / 3) - b / (3 * a);
    std::cout << "Time of flight: " << t << " \n";
    
    if(t <= 0)
    {
        std::cout << "error: t <= 0: " << t << "\n";
        
        return EXIT_FAILURE;
    }
    
    float powT2 = t * t;
    float powT3 = powT2 * t;
    float powT4 = powT3 * t;
    float powT5 = powT4 * t;
    float powD2 = d * d;
    float powD3 = powD2 * d;
    float powD4 = powD3 * d;
    float powL2 = l * l;
    float powL3 = powL2 * l;
    float powL4 = powL3 * l;
    float powM2 = m * m;
    float powM3 = powM2 * m;
    float powM4 = powM3 * m;
    float h = attackSpeed * t - powT2 * d * attackSpeed / (2 * m) + powT3 * (6 * d * g * l * m - 6 * powD2 * (powL2 - 1) * attackSpeed) / (36 * powM2) + powT4 * (-6 * powD2 * g * l * (1 + powL2 + powL4) * m + 3 * powD3 * powL2 * (1 + powL2) * attackSpeed + 6 * powD3 * powL4 * (1 + powL2) * attackSpeed) / (36 * pow((1 + powL2), 2) * powM3) + powT5 * (3 * powD3 * g * powL3 * m - 3 * powD4 * powL2 * (1 + powL2) * attackSpeed) / (36 * (1 + powL2) * powM4);
    
    std::cout << "h: " << h << " \n";
    
    if(h <= 0)
    {
        std::cout << "error: h <= 0: " << h << "\n";
        
        return EXIT_FAILURE;
    }
    
    float dist = sqrt(pow(targetX - xd, 2) + pow(targetY - yd, 2));
    
    if(dist <= 0)
    {
        std::cout << "error: D <= 0: " << dist << "\n";
        
        return EXIT_FAILURE;
    }
    
    if((h + accelerationPath) > dist)
    {
        xd = targetX - (targetX - xd) * (h + accelerationPath) / dist;
        yd = targetY - (targetY - yd) * (h + accelerationPath) / dist;
        std::cout << "Middle Point: (" << xd << "; " << yd << ")\n";
    }
    
    float ratio = (dist - h) / dist;
    std::cout << "Ratio: " << ratio << "\n";
    float fireX = xd + (targetX - xd) * ratio;
    float fireY = xd + (targetY - yd) * ratio;
    std::cout << "Fire Point: (" << fireX << "; " << fireY << ")\n";
    
    std::filesystem::path directory = filePath.parent_path();
    std::filesystem::path outFilePath = directory / "output.txt";
    std::cout << "Saving: " << outFilePath << "\n";
    std::ofstream outFile(outFilePath);
    outFile<< fireX << " " << fireY <<"\n";
    outFile.close();
    
    return EXIT_SUCCESS;
}

std::string toUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    
    return str;
}

