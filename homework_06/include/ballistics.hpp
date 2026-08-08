#pragma once

#include <string>

const int MAX_TELEMETRY_FRAMES = 128;
const int EXPECTED_FIELD_COUNT = 8;
const int MAX_LINE_LENGTH = 256;
const float GRAVITY_FORCE = 9.81;

// One ballistic input data sample from the input log.
struct BallisticInputData {
  float xd;
  float yd;
  float zd;
  float targetX;
  float targetY;
  float attackSpeed;
  float accelerationPath;
  std::string name;
  bool isValid;
};

struct AmmoData {
  float m;
  float d;
  float l;
  bool isValid;
};

struct BallisticOutputData {
  float ratio;
  float fireX;
  float fireY;
  float middleX;
  float middleY;
};

int readData(const char *path, BallisticInputData frames[], int maxFrames);
int splitLine(char line[], char *fields[], int maxFields);
AmmoData findAmmoType(std::string name);
BallisticOutputData calculateFirePoint(const BallisticInputData &inputData,
                                       const AmmoData &ammoData);
BallisticInputData parseInputData(char line[]);
bool writeData(const char *path, const BallisticOutputData outputData);
std::string toUpper(std::string str);
bool parseFloat(const char *text, float &value);
bool parseDouble(const char *text, double &value);
bool parseInt(const char *text, int &value);