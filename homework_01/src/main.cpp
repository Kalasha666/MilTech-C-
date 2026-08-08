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

const int MAX_LINE_LENGTH = 128;
const int EXPECTED_FIELD_COUNT = 8;
const float GRAVITY_FORCE = 9.8f;

int splitLine(char line[], char* fields[], int maxFields);
float parseFloat(const char* text);
char* getOutputPath(const char* inputPath, char* fileName);
bool writeData(const char *path, float fireX, float fireY, float middleXd = 0.0f, float middleYd = 0.0f);

int main(int argc, char** argv) {

    std::cout << "Home Task 1\n";
    std::cout << "Input file path:\n";

    if (argc != 2) {
        std::cerr << "usage: ugv_odometry <input_path>\n";
        return 1;
    }

    std::ifstream input{argv[1]};

    if (!input) {
        std::cerr << "error: failed to open input file: " << argv[1] << '\n';
        return 2;
    }

    char line[MAX_LINE_LENGTH];
    bool any_line = false;
    int line_number = 0;

    while (input.getline(line, MAX_LINE_LENGTH))
    {
        if (line[0] == '\0') continue;

        line_number++;
    }

    int capacity = line_number > 0 ? line_number : 1;
    input.clear();
    input.seekg(0);
    line_number = 0;
    float xd, yd, zd, targetX, targetY, attackSpeed, accelerationPath;
    char* name;

    while (capacity > 0 && input.getline(line, MAX_LINE_LENGTH)) 
    {
        if (line[0] == '\0') 
        {
            std::cout << "empty line, skipping\n";
            continue;
        }
  
        char* fields[EXPECTED_FIELD_COUNT] = {};
        const int field_count = splitLine(line, fields, EXPECTED_FIELD_COUNT);
        
        if(field_count != EXPECTED_FIELD_COUNT)
        {
            std::cout << "error: line " << line_number + 1 << " has " << field_count << " fields, expected " << EXPECTED_FIELD_COUNT << "\n";
            continue;
        }
        else
        {
            xd = parseFloat(fields[0]);
            yd = parseFloat(fields[1]);
            zd = parseFloat(fields[2]);
            targetX = parseFloat(fields[3]);
            targetY = parseFloat(fields[4]);
            attackSpeed = parseFloat(fields[5]);
            accelerationPath = parseFloat(fields[6]);
            name = fields[7];
            break;
        }

        line_number++;
    }

    float m, d, l;
    
    if(strcmp(name, "VOG-17") == 0)
    {
        m = 0.35f;
        d = 0.07f;
        l = 0.0f;
    }
    else if(strcmp(name, "M67") == 0)
    {
        m = 0.6f;
        d = 0.1f;
        l = 0.0f;
    }
    else if(strcmp(name, "RKG-3") == 0)
    {
        m = 1.2f;
        d = 0.1f;
        l = 0.0f;
    }
    else if(strcmp(name, "GLIDING-VOG") == 0)
    {
        m = 0.45f;
        d = 0.1f;
        l = 1.0f;
    }
    else if(strcmp(name, "GLIDING-RKG") == 0)
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
    
    float a = d * GRAVITY_FORCE * m - 2 * d * d * l * attackSpeed;
    float b = - 3 * GRAVITY_FORCE * m*m + 3 * d * l * m * attackSpeed;
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
    float h = attackSpeed * t - powT2 * d * attackSpeed / (2 * m) + powT3 * (6 * d * GRAVITY_FORCE * l * m - 6 * powD2 * (powL2 - 1) * attackSpeed) / (36 * powM2) + powT4 * (-6 * powD2 * GRAVITY_FORCE * l * (1 + powL2 + powL4) * m + 3 * powD3 * powL2 * (1 + powL2) * attackSpeed + 6 * powD3 * powL4 * (1 + powL2) * attackSpeed) / (36 * pow((1 + powL2), 2) * powM3) + powT5 * (3 * powD3 * GRAVITY_FORCE * powL3 * m - 3 * powD4 * powL2 * (1 + powL2) * attackSpeed) / (36 * (1 + powL2) * powM4);
    
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
    
    float middleXd = 0.0f;
    float middleYd = 0.0f;

    if((h + accelerationPath) > dist)
    {
        middleXd = targetX - (targetX - xd) * (h + accelerationPath) / dist;
        middleYd = targetY - (targetY - yd) * (h + accelerationPath) / dist;
        std::cout << "Middle Point: (" << middleXd << "; " << middleYd << ")\n";
    }
    
    float ratio = (dist - h) / dist;
    std::cout << "Ratio: " << ratio << "\n";
    float fireX = xd + (targetX - xd) * ratio;
    float fireY = xd + (targetY - yd) * ratio;
    std::cout << "Fire Point: (" << fireX << "; " << fireY << ")\n";
    
    writeData(getOutputPath(argv[1], "output.txt"), fireX, fireY, middleXd = 0.0f , middleYd = 0.0f);
    
    return EXIT_SUCCESS;
}

float parseFloat(const char* text) 
{
    if (text[0] == '+') 
    {
        ++text;
    }

    return std::strtod(text, nullptr);
}

int splitLine(char line[], char* fields[], int maxFields) 
{
    int count = 0;
    char* cursor = line;

    while (*cursor != '\0' && count < maxFields)
    {
        while (*cursor == ' ' || *cursor == '\t' || *cursor == '\n' || *cursor == '\r') 
        {
            *cursor = '\0';
            ++cursor;
        }

        if (*cursor == '\0') 
        {
            break;
        }

        fields[count] = cursor;
        ++count;

        while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t' && *cursor != '\n' && *cursor != '\r') 
        {
            ++cursor;
        }
    }

    return count;
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

bool writeData(const char *path, float fireX, float fireY, float middleXd = 0.0f, float middleYd = 0.0f) {
  std::ofstream output(path);

  if (!output.is_open()) {
    std::cerr << "error: could not open output file\n";
    return false;
  }

  char line[MAX_LINE_LENGTH];
  int written = (middleXd != 0.0f || middleYd != 0.0f) ? std::snprintf(line, MAX_LINE_LENGTH, "%f %f %f %f", fireX, fireY, middleXd, middleYd) : std::snprintf(line, MAX_LINE_LENGTH, "%f %f", fireX, fireY);

  if (written < 0 || written >= MAX_LINE_LENGTH) {
    std::cerr << "error: line too long at frame " << "\n";
    return false;
  }

  output << line << '\n';
  return true;
}