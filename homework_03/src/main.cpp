//
//  main.cpp
//  MilTech-C-1
//
//  Created by Oleh Kalashnyk on 09.04.2026.
//

#define _USE_MATH_DEFINES

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <sstream>
#include <vector>
#include <filesystem>

const int MAX_STEPS = 10000;
const int TARGETS_COUNT = 5;
const int POINTS_COUNT = 60;
const float GRAVITY_FORCE = 9.81;
const char BOMB_NAMES_LIST[][15] = {"VOG-17", "M67", "RK-3", "GLIDING-VOG", "GLIDING-RKG"};
const float BOMB_M[] = {0.35f, 0.6f, 1.2f, 0.45f, 1.4f};
const float BOMB_D[] = {0.07f, 0.10f, 0.10f, 0.10f, 0.10f};
const float BOMB_L[] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f};

enum class DroneState {
      STOPPED = 0,
      ACCELERATING,
      DECELERATING,
      TURNING,
      MOVING
  };

struct Coord
{
    float x;
    float y;
    
    Coord operator+(const Coord& other) const
    {
        Coord result;
        result.x = x + other.x;
        result.y = y + other.y;
        
        return result;
    }
    
    Coord operator-(const Coord& other) const
    {
        Coord result;
        result.x = x - other.x;
        result.y = y - other.y;
        
        return result;
    }
    
    Coord operator*(float s) const
    {
        Coord result;
        result.x = x * s;
        result.y = y * s;
        
        return result;
    }
    
    Coord operator/(float s) const
    {
        Coord result;
        result.x = s != 0 ? x / s : 0;
        result.y = s != 0 ? y / s : 0;
        
        return result;
    }
    
    bool operator==(const Coord& other) const
    {
        return x == other.x && y == other.y;
    }
    
    float length()
    {
        return std::sqrt(x*x + y*y);
    }
    
    void normalize()
    {
        float len = length();
        
          if (len > 0.0f)
          {
              x /= len;
              y /= len;
          }
    }
    
    Coord normalized()
    {
        Coord result;
        float len = length();
        
        if (len > 0.0f)
        {
            result.x = x / len;
            result.y = y / len;
        }
        
        return result;
    }
};

struct AmmoParams
{
    std::string name;
    float mass;
    float drag;
    float lift;
};

struct DroneConfig
{
    Coord startPos;
    float altitude;
    float initialDir;
    float attackSpeed;
    float accelPath;
    std::string ammoName;
    float arrayTimeStep;
    float simTimeStep;
    float hitRadius;
    float angularSpeed;
    float turnThreshold;
};

struct SimStep
{
    Coord pos;
    float direction;
    int state;
    int targetIdx;
    Coord dropPoint;
    Coord aimPoint;
    Coord predictedTarget;
};

std::string toUpper(std::string str);
float timeDropCalc(const float& m, const float& d, const float& l, const float& attackSpeed, const float& zd);
float horizontalDistance(const float& t, const float& d, const float& l, const float& m, const float attackSpeed);
float distanceToTarget(const float& xd, const float& yd, const float& xTarget, const float& yTarget);
float ratioCalc(const float& h, const float& distance);
float dropCoodinate(const float& coord, const float& target, const float& h, const float& distance);
void targetPointByTime(float& x, float&y, const float& time, const int& targetIndex, const float& arrayTimeStep, const float (&targetX)[TARGETS_COUNT][POINTS_COUNT], const float (&targetY)[TARGETS_COUNT][POINTS_COUNT]);
float angleBetween(const float& x0, const float& y0, const float& x1, const float y1);
void normalize(float& x, float& y);
void directionVector(const float& x0, const float& y0, const float& x1, const float& y1, float& dirX, float& dirY);
int minIndexInArray(const float (&array)[TARGETS_COUNT]);
float lengthVector(const float& x, const float& y);
float lerp(float a, float b, float t);
void clamp(const float& min, const float& max, float& value);
void rotateXY(float& x, float& y, float angle);
void updateDroneState(const float& speed, const float& maxSpeed, const float& angle, const float& turnAngle, DroneState& state, float& dt);
void updateTime(const float (&targetsX)[TARGETS_COUNT][POINTS_COUNT], const float (&targetsY)[TARGETS_COUNT][POINTS_COUNT], const float (&timeToStop)[5], const float& t, const float& dt, const float& xd, const float& yd, const float& arrayTimeStep, const float& dirXDrone, const float& dirYDrone, const DroneState& droneState, const float& accelerationPath, const float& attackSpeed, const float& turnThreshold, float (&distanceTargets)[TARGETS_COUNT], float (&timeToTarget)[TARGETS_COUNT], float (&angleTargets)[TARGETS_COUNT]);

int main(int argc, const char * argv[]) {
    
    std::cout << "Home Task Lesson 2\n";
    std::cout << "Input file path:\n";
    std::cout << std::filesystem::current_path() << "\n";
    std::string inputFilePath = "input.txt";
    std::string dirPath = "";
    std::getline(std::cin, inputFilePath);
    std::filesystem::path filePath = std::filesystem::path(inputFilePath);
    std::cout << "Opening " << inputFilePath << "\n";
    std::ifstream inFile(filePath);
    
    if (!inFile || !inFile.is_open()) {
        std::cerr << "Error: Could not open: " << inputFilePath << "\n";
        return EXIT_FAILURE;
    }
    
    dirPath = filePath.parent_path().string();
    
    float xd = 0;
    float yd = 0;
    float zd = 0;
    float initialDir = 0;
    float attackSpeed = 0;
    float accelerationPath = 0;
    float arrayTimeStep = 0;
    float simTimeStep = 0;
    float hitRadius = 0;
    float angularSpeed = 0;
    float turnThreshold = 0;
    std::string ammo_name;
    std::string line;
    int lineIndex = 0;

    while (std::getline(inFile, line)) {
        if (line.empty()) continue;
        //std::cout << line << "\n";
        std::istringstream iss(line);
        
        switch (lineIndex) {
            case 0:
                iss >> xd >> yd >> zd;
                break;
            case 1:
                iss >> initialDir;
                break;
            case 2:
                iss >> attackSpeed;
                break;
            case 3:
                iss >> accelerationPath;
                break;
            case 4:
                iss >> ammo_name;
                break;
            case 5:
                iss >> arrayTimeStep;
                break;
            case 6:
                iss >> simTimeStep;
                break;
            case 7:
                iss >> hitRadius;
                break;
            case 8:
                iss >> angularSpeed;
                break;
            case 9:
                iss >> turnThreshold;
                break;
                
            default:
                std::cerr << "Error: Unknown line in the input file: " << line << "\n";
                break;
        }
        
        lineIndex +=1;
    }
    
    ammo_name = toUpper(ammo_name);
    std::cout << "amo_name: " << ammo_name << "\n";
    const int bombTypeCount = sizeof(BOMB_NAMES_LIST) / sizeof(BOMB_NAMES_LIST[0]);
    int bombTypeIndex = -1;
    
    for(int i = 0; i < bombTypeCount; i++)
    {
        if(strcmp(ammo_name.c_str(), BOMB_NAMES_LIST[i]) == 0)
        {
            bombTypeIndex = i;
            break;
        }
    }
    
    if(bombTypeIndex < 0)
    {
        std::cout << "Error: Unknown type: " << ammo_name << "\n";
        
        return EXIT_FAILURE;
    }
    
    float m = BOMB_M[bombTypeIndex];
    float d = BOMB_D[bombTypeIndex];
    float l = BOMB_L[bombTypeIndex];
    std::cout << "In: Position: (" << xd << "; " << yd << ";"<< zd <<") / ";
    std::cout << "name: " << ammo_name << " / ";
    std::cout << "attackSpeed: " << attackSpeed << " / ";
    std::cout << "accelerationPath: " << accelerationPath << " \n";
    std::cout << "arrayTimeStep: " << arrayTimeStep << " \n";
    std::cout << "simTimeStep: " << simTimeStep << " \n";
    std::cout << "hitRadius: " << hitRadius << " \n";
    std::cout << "angularSpeed: " << angularSpeed << " \n";
    std::cout << "turnThreshold: " << turnThreshold << " \n";
    std::cout << "Targets file path:\n";
    filePath = filePath.parent_path() / "targets.txt";
    std::cout << "Opening: " << filePath.path::string() << "\n";
    std::ifstream inTargetsFile(filePath);
    
    if (!inTargetsFile || !inTargetsFile.is_open()) {
        std::cerr << "Error: Could not open: " << inputFilePath << "\n";
        return EXIT_FAILURE;
    }
    
    float targetsX[TARGETS_COUNT][POINTS_COUNT];
    float targetsY[TARGETS_COUNT][POINTS_COUNT];
    std::string targetsLine;
    lineIndex = 0;

    while (std::getline(inTargetsFile, targetsLine)) {
        if (targetsLine.empty()) continue;
        
        std::istringstream iss(targetsLine);
        
        if(lineIndex >=0 && lineIndex < TARGETS_COUNT)
        {
            //std::cout <<"X: " << targetsLine << "\n";
            
            for (int i = 0; i < POINTS_COUNT; i++)
            {
                iss >> targetsX[lineIndex][i];
            }
        }
        else if(lineIndex >=TARGETS_COUNT && lineIndex < TARGETS_COUNT * 2)
        {
            //std::cout <<"Y: " << targetsLine << "\n";
            
            for (int i = 0; i < POINTS_COUNT; i++)
            {
                iss >> targetsY[lineIndex - 5][i];
            }
        }
        
        lineIndex +=1;
    }
    
    float timeDrop = 0;
    
    try
    {
        timeDrop = timeDropCalc(m, d, l, attackSpeed, zd);
    }
    catch (const std::invalid_argument& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
    }
    
    std::cout << "Time of flight: " << timeDrop << " \n";
    
    if(timeDrop <= 0)
    {
        std::cout << "Error: t <= 0: " << timeDrop << "\n";
        
        return EXIT_FAILURE;
    }
    
    float h = horizontalDistance(timeDrop, d, l, m, attackSpeed);
    
    std::cout << "h: " << h << " \n";
    
    if(h <= 0)
    {
        std::cout << "Error: h <= 0: " << h << "\n";
        
        return EXIT_FAILURE;
    }
    
    DroneState droneState = DroneState::STOPPED;
    bool isMissionComplete = false;
    int simStep = 0;
    float t = 0;
    float dt = 0.0f;
    float timeStep = simTimeStep; //simTimeStep < arrayTimeStep ? simTimeStep : arrayTimeStep;
    float a = attackSpeed * attackSpeed / (2 * accelerationPath);
    float timeToTarget[TARGETS_COUNT];
    float distanceTargets[TARGETS_COUNT];
    float angleTargets[TARGETS_COUNT];
    std::string coordinatesString = "";
    std::string directionsString = "";
    std::string statesString = "";
    std::string targetsString = "";
    float droneMoveSpeed = 0.0f;
    const float timeToStop[TARGETS_COUNT] = {0.0f, (attackSpeed - droneMoveSpeed) / a, (attackSpeed - droneMoveSpeed) / a, attackSpeed / a, turnThreshold / angularSpeed};
    float dirXDrone, dirYDrone;
    directionVector(xd, yd, targetsX[0][0], targetsY[0][0], dirXDrone, dirYDrone);
    normalize(dirXDrone, dirYDrone);
    int targetIndex = 0;

    while (!isMissionComplete && simStep < MAX_STEPS)
    {
        updateTime(targetsX, targetsY, timeToStop, t, dt, xd, yd, arrayTimeStep, dirXDrone, dirYDrone, droneState, accelerationPath, attackSpeed, turnThreshold, distanceTargets, timeToTarget, angleTargets);
        int target = minIndexInArray(timeToTarget);
        updateTime(targetsX, targetsY, timeToStop, t + timeToTarget[target], dt, xd, yd, arrayTimeStep, dirXDrone, dirYDrone, droneState, accelerationPath, attackSpeed, turnThreshold, distanceTargets, timeToTarget, angleTargets);
        target = minIndexInArray(timeToTarget);
        float x, y;
        targetPointByTime(x, y, t + timeToTarget[target], targetIndex, arrayTimeStep, targetsX, targetsY);
        dirXDrone = x - dirXDrone;
        dirYDrone = y - dirYDrone;
        normalize(dirXDrone, dirYDrone);
        float angleToTarget = angleTargets[targetIndex];
        updateDroneState(droneMoveSpeed, attackSpeed, angleToTarget, turnThreshold, droneState, dt);
        
        switch (droneState) {
            case DroneState::TURNING:
                rotateXY(dirXDrone, dirYDrone, angularSpeed * timeStep);
                break;
                
            case DroneState::ACCELERATING:
                droneMoveSpeed = lerp(0, attackSpeed, dt);
                clamp(0, attackSpeed, droneMoveSpeed);
                xd += droneMoveSpeed * timeStep * dirXDrone;
                yd += droneMoveSpeed * timeStep * dirYDrone;
                break;
                
            case DroneState::DECELERATING:
                droneMoveSpeed = lerp(attackSpeed, 0, dt);
                clamp(0, attackSpeed, droneMoveSpeed);
                xd += droneMoveSpeed * timeStep * dirXDrone;
                yd += droneMoveSpeed * timeStep * dirYDrone;
                break;
                
            default:
                xd += droneMoveSpeed * timeStep * dirXDrone;
                yd += droneMoveSpeed * timeStep * dirYDrone;
                break;
        }
        
        float distance = distanceToTarget(xd, yd, x, y);
        
        std::cout << simStep << " Time:"<< t << " State time:" << dt << " droneSpeed: " << droneMoveSpeed << " Drone position:("<< xd <<","<< yd <<","<<zd <<")" << " State:" << (int)droneState << " target:" << target << " Time:" << timeToTarget[targetIndex] << " Angle To taget:"<< angleToTarget <<"\n";
        
        coordinatesString += std::to_string(xd) + " ";
        coordinatesString += std::to_string(yd) + " ";
        directionsString += std::to_string(angleToTarget) + " ";
        statesString += std::to_string((int)droneState) + " ";
        targetsString += std::to_string(target) + " ";
        
        if(distance <= h)
        {
            isMissionComplete = true;
        }
        
        t += timeStep;
        dt += timeStep;
        simStep += 1;
    }
    
    filePath = filePath.parent_path() / "simulation.txt";
    std::cout << "Saving: " << filePath.path::string()  << "\n";
    std::ofstream outFile(filePath);
    outFile << simStep <<"\n";
    outFile << coordinatesString <<"\n";
    outFile << directionsString <<"\n";
    outFile << targetsString <<"\n";
    outFile.close();
   
    return EXIT_SUCCESS;
}

std::string toUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    
    return str;
}

float timeDropCalc(const float& m, const float& d, const float& l, const float& attackSpeed, const float& zd)
{
    float a = d * GRAVITY_FORCE * m - 2 * d * d * l * attackSpeed;
    float b = - 3 * GRAVITY_FORCE * m * m + 3 * d * l * m * attackSpeed;
    float c = 6 * m * m * zd;
    float p = (- b * b) / (3 * a * a);
    float q = (2 * b * b * b) / (27 * a * a * a) + c / a;
    float value = (3 * q) / (2 * p) * sqrt(-3 / p);
    
    if(value < -1 || value > 1)
    {
        throw std::invalid_argument("Out of the range [-1;1]");
    }
    
    float fi = acos(value);
    float t = 2 * sqrt(-p / 3) * cos((fi + 4 * M_PI) / 3) - b / (3 * a);
    
    return t;
}

float horizontalDistance(const float& t, const float& d, const float& l, const float& m, const float attackSpeed)
{
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
    
    return h;
}

float distanceToTarget(const float& xd, const float& yd, const float& xTarget, const float& yTarget)
{
    return sqrt(pow(xTarget - xd, 2) + pow(yTarget - yd, 2));
}

float ratioCalc(const float& h, const float& distance)
{
    return  (distance - h) / distance;
}

float dropCoodinate(const float& coord, const float& target, const float& h, const float& distance)
{
    return coord + (target - coord) * ratioCalc(h, distance);
}

void targetPointByTime(float& x, float&y, const float& time, const int& targetIndex, const float& arrayTimeStep, const float (&targetX)[TARGETS_COUNT][POINTS_COUNT], const float (&targetY)[TARGETS_COUNT][POINTS_COUNT])
{
    int index = (int)floor(time / arrayTimeStep);
    int idx = index % POINTS_COUNT;
    int next = (idx + 1) % POINTS_COUNT;
    float frac = (time - index * arrayTimeStep) / arrayTimeStep;
    x = targetX[targetIndex][idx] + (targetX[targetIndex][next] - targetX[targetIndex][idx]) * frac;
    y = targetY[targetIndex][idx] + (targetY[targetIndex][next] - targetY[targetIndex][idx]) * frac;
    //std::cout << "i:" << targetIndex << " index:" << index << " idx:"<< idx << " next:" << next << "\n";
}

float angleBetween(const float& x0, const float& y0, const float& x1, const float y1)
{
      float dot = x0 * x1 + y0 * y1;
      float len0 = lengthVector(x0, y0);
      float len1 = lengthVector(x1, y1);
    
      return std::acos(dot / (len0 * len1)); // radians
}

void normalize(float& x, float& y)
{
    float len = lengthVector(x, y);
    
      if (len > 0.0f)
      {
          x /= len;
          y /= len;
      }
}

void directionVector(const float& x0, const float& y0, const float& x1, const float& y1, float& dirX, float& dirY)
{
    dirX = x1 - x0;
    dirY = y1 - y0;
    float len = lengthVector(dirX, dirY);
    
    if (len > 0.0f)
    {
        dirX /= len;
        dirY /= len;
    }
}

int minIndexInArray(const float (&array)[TARGETS_COUNT])
{
    int index = 0;
    float minValue = array[index];
    
    for(int i = 1; i < TARGETS_COUNT; i++)
    {
        if(minValue > array[i])
        {
            minValue = array[i];
            index = i;
        }
    }
    
    return index;
}

float lengthVector(const float& x, const float& y)
{
    return std::sqrt(x*x + y*y);
}

float lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

void clamp(const float& min, const float& max, float& value)
{
    if(value < min) value = min;
    else if(value > max) value = max;
}

void rotateXY(float& x, float& y, float angle)
{
    float c = std::cos(angle);
    float s = std::sin(angle);
    float nx = x * c - y * s;
    float ny = x * s + y * c;
    x = nx;
    y = ny;
}

void updateDroneState(const float& speed, const float& maxSpeed, const float& angle, const float& turnAngle, DroneState& state, float& dt)
{
    if(state == DroneState::ACCELERATING && speed >= maxSpeed && angle < turnAngle && state != DroneState::MOVING)
    {
        state = DroneState::MOVING;
        dt = 0.0f;
    }
    else if (state == DroneState::STOPPED && angle < turnAngle && state != DroneState::ACCELERATING)
    {
        state = DroneState::ACCELERATING;
        dt = 0.0f;
    }
    else if(state == DroneState::STOPPED && angle >= turnAngle && state != DroneState::TURNING)
    {
        state = DroneState::TURNING;
        dt = 0.0f;
    }
    else if((state == DroneState::ACCELERATING || state == DroneState::MOVING)  && angle >= turnAngle && state != DroneState::DECELERATING)
    {
        state = DroneState::DECELERATING;
        dt = 0.0f;
    }
    else if(state == DroneState::DECELERATING && speed <= 0 && state != DroneState::STOPPED)
    {
        state = DroneState::STOPPED;
        dt = 0.0f;
    }
}

void updateTime(const float (&targetsX)[TARGETS_COUNT][POINTS_COUNT], const float (&targetsY)[TARGETS_COUNT][POINTS_COUNT], const float (&timeToStop)[5], const float& t, const float& dt, const float& xd, const float& yd, const float& arrayTimeStep, const float& dirXDrone, const float& dirYDrone, const DroneState& droneState, const float& accelerationPath, const float& attackSpeed, const float& turnThreshold, float (&distanceTargets)[TARGETS_COUNT], float (&timeToTarget)[TARGETS_COUNT], float (&angleTargets)[TARGETS_COUNT])
{
    for(int i = 0; i < TARGETS_COUNT; i++)
    {
        float x1, y1;
        targetPointByTime(x1, y1, t + timeToTarget[i], i, arrayTimeStep, targetsX, targetsY);
        distanceTargets[i] = distanceToTarget(xd, yd, x1, y1);
        float dirXTarget, dirYTarget;
        directionVector(xd, yd, x1, y1, dirXTarget, dirYTarget);
        angleTargets[i] = angleBetween(dirXDrone, dirYDrone, dirXTarget, dirYTarget);
    }
    
    for(int i = 0; i < TARGETS_COUNT; i++)
    {
        switch (droneState) {
                
            case DroneState::TURNING:
                timeToTarget[i] = distanceTargets[i] > accelerationPath ? (timeToStop[(int)DroneState::TURNING] - dt) + (2 * accelerationPath) / attackSpeed + (distanceTargets[i] - accelerationPath) / attackSpeed : timeToStop[(int)DroneState::TURNING] + (2 * accelerationPath) / attackSpeed;
                break;
                
            case DroneState::ACCELERATING:
                if(distanceTargets[i] > accelerationPath)
                {
                    timeToTarget[i] = angleTargets[i] > turnThreshold ? timeToStop[(int)DroneState::TURNING] + (timeToStop[(int)DroneState::ACCELERATING] - dt) + (distanceTargets[i] - accelerationPath) / attackSpeed : (timeToStop[(int)DroneState::ACCELERATING] - dt) + (distanceTargets[i] - accelerationPath) / attackSpeed;
                }
                else
                {
                    timeToTarget[i] = angleTargets[i] > turnThreshold ? timeToStop[(int)DroneState::TURNING] + (timeToStop[(int)DroneState::ACCELERATING] - dt) + (accelerationPath - distanceTargets[i]) / attackSpeed : (timeToStop[(int)DroneState::ACCELERATING] - dt) + (accelerationPath - distanceTargets[i]) / attackSpeed;
                }
                break;
            case DroneState::DECELERATING:
                if(distanceTargets[i] > accelerationPath)
                {
                    timeToTarget[i] = angleTargets[i] > turnThreshold ? timeToStop[(int)DroneState::TURNING] + (timeToStop[(int)DroneState::DECELERATING] - dt) + timeToStop[(int)DroneState::ACCELERATING] + (distanceTargets[i] - accelerationPath) / attackSpeed : (timeToStop[(int)DroneState::DECELERATING] - dt) + timeToStop[(int)DroneState::ACCELERATING] + (distanceTargets[i] - accelerationPath) / attackSpeed;
                }
                else
                {
                    timeToTarget[i] = angleTargets[i] > turnThreshold ? timeToStop[(int)DroneState::TURNING] + (timeToStop[(int)DroneState::DECELERATING] - dt) + timeToStop[(int)DroneState::ACCELERATING] + (distanceTargets[i] - accelerationPath) / attackSpeed : (timeToStop[(int)DroneState::DECELERATING] - dt) + timeToStop[(int)DroneState::ACCELERATING] + (accelerationPath - distanceTargets[i]) / attackSpeed;
                }
                break;
                
            default:
                timeToTarget[i] = distanceTargets[i] > accelerationPath ? timeToStop[(int)DroneState::ACCELERATING] + (distanceTargets[i] - accelerationPath) / attackSpeed : timeToStop[(int)DroneState::ACCELERATING] + (accelerationPath - distanceTargets[i]) / attackSpeed;
                break;
        }
    }
}

