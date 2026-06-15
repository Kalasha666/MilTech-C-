#include "ballistics.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

BallisticInputData parseInputData(char line[]);

int readData(const char *path, BallisticInputData inputsData[], int maxFrames) {
  std::ifstream input{path};

  if (!input) {
    std::cerr << "error: failed to open input file: " << path << '\n';
    return 0;
  }

  int count = 0;
  int lineNumber = 0;
  char line[MAX_LINE_LENGTH];

  while (input.getline(line, MAX_LINE_LENGTH)) {

    if (line[0] == '\0') {
      std::cerr << "error: line is empty: " << lineNumber << "\n";
      ++lineNumber;
      continue;
    }

    char *fields[EXPECTED_FIELD_COUNT] = {};
    char copy[MAX_LINE_LENGTH];
    const int fieldCount =
        splitLine(std::strcpy(copy, line), fields, EXPECTED_FIELD_COUNT);

    if (fieldCount < EXPECTED_FIELD_COUNT) {
      std::cerr << "error: invalid fields count: " << fieldCount << "\n";
      continue;
    }

    if (count < maxFrames) {
      inputsData[count] = parseInputData(line);
      ++count;
    }

    ++lineNumber;
  }

  if (count < 1) {
    std::cerr << "error: file is empty\n";
  }

  return count;
}

int splitLine(char line[], char *fields[], int maxFields) {
  int count = 0;
  char *cursor = line;

  while (*cursor != '\0' && count < maxFields) {
    while (*cursor == ' ' || *cursor == '\t' || *cursor == '\n' ||
           *cursor == '\r') {
      *cursor = '\0';
      ++cursor;
    }

    if (*cursor == '\0') {
      break;
    }

    fields[count] = cursor;
    ++count;

    while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t' &&
           *cursor != '\n' && *cursor != '\r') {
      ++cursor;
    }
  }

  return count;
}

bool parseLong(const char *text, long &value) {
  char *end = nullptr;
  value = std::strtol(text, &end, 10);

  if (end == text) {
    std::cerr << "error: invalid fields value: " << end << "\n";
    value = 0;

    return false;
  }

  return true;
}

bool parseInt(const char *text, int &value) {

  char *end = nullptr;
  value = std::strtol(text, &end, 10);

  if (end == text) {
    std::cerr << "error: invalid fields value: " << end << "\n";
    value = 0;
    return false;
  } else {
    value = static_cast<int>(value);
  }

  return true;
}

bool parseDouble(const char *text, double &value) {
  char *end = nullptr;
  value = std::strtod(text, &end);

  if (end == text) {
    std::cerr << "error: invalid fields value: " << end << "\n";
    value = 0.0;
    return false;
  }

  return true;
}

bool parseFloat(const char *text, float &value) {
  char *end = nullptr;
  value = std::strtof(text, &end);

  if (end == text) {
    std::cerr << "error: invalid fields value: " << end << "\n";
    value = 0.0f;
    return false;
  }

  return true;
}

std::string toUpper(std::string str) {
  std::transform(str.begin(), str.end(), str.begin(), ::toupper);

  return str;
}

/*
struct BallisticInputData {
    float xd;
    float yd;
    float zd;
    float targetX;
    float targetY;
    float attackSpeed;
    float accelerationPath;
};
*/

BallisticInputData parseInputData(char line[]) {
  char *fields[EXPECTED_FIELD_COUNT] = {};
  const int field_count = splitLine(line, fields, EXPECTED_FIELD_COUNT);
  BallisticInputData data{};
  data.isValid = true;
  bool xdResult = parseFloat(fields[0], data.xd);
  bool ydResult = parseFloat(fields[1], data.yd);
  bool zdResult = parseFloat(fields[2], data.zd);
  bool targetXResult = parseFloat(fields[3], data.targetX);
  bool targetYResult = parseFloat(fields[4], data.targetY);
  bool attackSpeedResult = parseFloat(fields[5], data.attackSpeed);
  bool accelerationPathResult = parseFloat(fields[6], data.accelerationPath);
  if (fields[7] != nullptr) {
    data.name = fields[7];
  } else {
    std::cerr << "error: missing name field\n";
    data.isValid = false;
  }

  if (!xdResult || !ydResult || !zdResult || !targetXResult || !targetYResult ||
      !attackSpeedResult || !accelerationPathResult) {
    data.isValid = false;
  }

  return data;
}

AmmoData findAmmoType(std::string name) {
  AmmoData ammoData{};
  ammoData.isValid = true;

  if (name == "VOG-17") {
    ammoData.m = 0.35f;
    ammoData.d = 0.07f;
    ammoData.l = 0.0f;
  } else if (name == "M67") {
    ammoData.m = 0.6f;
    ammoData.d = 0.1f;
    ammoData.l = 0.0f;
  } else if (name == "RKG-3") {
    ammoData.m = 1.2f;
    ammoData.d = 0.1f;
    ammoData.l = 0.0f;
  } else if (name == "GLIDING-VOG") {
    ammoData.m = 0.45f;
    ammoData.d = 0.1f;
    ammoData.l = 0.2f;
  } else {
    std::cerr << "error: Unknown Ammo type: " << name << "\n";
    ammoData.m = 0.0f;
    ammoData.d = 0.0f;
    ammoData.l = 0.0f;
    ammoData.isValid = false;
  }

  return ammoData;
}

BallisticOutputData calculateFirePoint(const BallisticInputData &inputData,
                                       const AmmoData &ammoData) {
  BallisticOutputData outputData{};

  float a = ammoData.d * GRAVITY_FORCE * ammoData.m -
            2 * ammoData.d * ammoData.d * ammoData.l * inputData.attackSpeed;
  float b = -3 * GRAVITY_FORCE * ammoData.m * ammoData.m +
            3 * ammoData.d * ammoData.l * ammoData.m * inputData.attackSpeed;
  float c = 6 * ammoData.m * ammoData.m * inputData.zd;
  float p = (-b * b) / (3 * a * a);
  float q = (2 * b * b * b) / (27 * a * a * a) + c / a;
  float arcArg = (3 * q) / (2 * p) * sqrt(-3 / p);

  if (arcArg < -1 || arcArg > 1) {
    std::cerr << "error: acos argument isn't in the range [-1; 1]: " << arcArg
              << "\n";
  }

  float fi = acos(arcArg);
  float t = 2 * sqrt(-p / 3) * cos((fi + 4 * M_PI) / 3) - b / (3 * a);
  std::cout << "Time of flight: " << t << " \n";

  if (t <= 0) {
    std::cerr << "error: t <= 0: " << t << "\n";
  }

  float powT2 = t * t;
  float powT3 = powT2 * t;
  float powT4 = powT3 * t;
  float powT5 = powT4 * t;
  float powD2 = ammoData.d * ammoData.d;
  float powD3 = powD2 * ammoData.d;
  float powD4 = powD3 * ammoData.d;
  float powL2 = ammoData.l * ammoData.l;
  float powL3 = powL2 * ammoData.l;
  float powL4 = powL3 * ammoData.l;
  float powM2 = ammoData.m * ammoData.m;
  float powM3 = powM2 * ammoData.m;
  float powM4 = powM3 * ammoData.m;
  float h = inputData.attackSpeed * t -
            powT2 * ammoData.d * inputData.attackSpeed / (2 * ammoData.m) +
            powT3 *
                (6 * ammoData.d * GRAVITY_FORCE * ammoData.l * ammoData.m -
                 6 * powD2 * (powL2 - 1) * inputData.attackSpeed) /
                (36 * powM2) +
            powT4 *
                (-6 * powD2 * GRAVITY_FORCE * ammoData.l * (1 + powL2 + powL4) *
                     ammoData.m +
                 3 * powD3 * powL2 * (1 + powL2) * inputData.attackSpeed +
                 6 * powD3 * powL4 * (1 + powL2) * inputData.attackSpeed) /
                (36 * pow(1 + powL2, 2) * powM3) +
            powT5 *
                (3 * powD3 * GRAVITY_FORCE * powL3 * ammoData.m -
                 3 * powD4 * powL2 * (1 + powL2) * inputData.attackSpeed) /
                (36 * (1 + powL2) * powM4);
  std::cout << "h: " << h << " \n";

  if (h <= 0) {
    std::cerr << "error: h <= 0: " << h << "\n";
  }

  float dist = sqrt(pow(inputData.targetX - inputData.xd, 2) +
                    pow(inputData.targetY - inputData.yd, 2));

  if (dist <= 0) {
    std::cerr << "error: D <= 0: " << dist << "\n";
  }

  if ((h + inputData.accelerationPath) > dist) {
    outputData.middleX =
        inputData.targetX - (inputData.targetX - inputData.xd) *
                                (h + inputData.accelerationPath) / dist;
    outputData.middleY =
        inputData.targetY - (inputData.targetY - inputData.yd) *
                                (h + inputData.accelerationPath) / dist;
    std::cout << "Middle Point: (" << outputData.middleX << "; "
              << outputData.middleY << ")\n";
  }

  outputData.ratio = (dist - h) / dist;
  std::cout << "Ratio: " << outputData.ratio << "\n";
  outputData.fireX =
      inputData.xd + (inputData.targetX - inputData.xd) * outputData.ratio;
  outputData.fireY =
      inputData.yd + (inputData.targetY - inputData.yd) * outputData.ratio;
  std::cout << "Fire Point: (" << outputData.fireX << "; " << outputData.fireY
            << ")\n";

  return outputData;
}

bool writeData(const char *path, const BallisticOutputData outputData) {
  std::ofstream output(path);

  if (!output.is_open()) {
    std::cerr << "error: could not open output file\n";
    return false;
  }

  char line[MAX_LINE_LENGTH];
  int written = std::snprintf(line, MAX_LINE_LENGTH, "%f %f", outputData.fireX,
                              outputData.fireY);

  if (written < 0 || written >= MAX_LINE_LENGTH) {
    std::cerr << "error: line too long at frame " << "\n";
    return false;
  }

  output << line << '\n';
  return true;
}