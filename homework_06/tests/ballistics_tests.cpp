#include "ballistics.hpp"
#include <cassert>
#include <cmath>

void test_parseInputData_valid();
void test_parseInputData_invalid_float();
void test_parseInputData_missing_name();

int main() {
  // test parseFloat
  float value = 0.0f;
  assert(parseFloat("3.14", value) == true);
  assert(value > 3.13f && value < 3.15f);

  // test findAmmoType
  AmmoData ammo = findAmmoType("VOG-17");
  assert(ammo.isValid == true);
  assert(ammo.m == 0.35f);

  test_parseInputData_valid();
  test_parseInputData_invalid_float();
  test_parseInputData_missing_name();

  return 0; // 0 = pass
}

void test_parseInputData_valid() {
  char line[] = "100.0 200.0 50.0 300.0 400.0 15.0 20.0 VOG-17";
  BallisticInputData data = parseInputData(line);

  assert(data.isValid == true);
  assert(std::fabs(data.xd - 100.0f) < 0.001f);
  assert(std::fabs(data.yd - 200.0f) < 0.001f);
  assert(std::fabs(data.zd - 50.0f) < 0.001f);
  assert(std::fabs(data.targetX - 300.0f) < 0.001f);
  assert(std::fabs(data.targetY - 400.0f) < 0.001f);
  assert(std::fabs(data.attackSpeed - 15.0f) < 0.001f);
  assert(std::fabs(data.accelerationPath - 20.0f) < 0.001f);
  assert(data.name == "VOG-17");
}

void test_parseInputData_invalid_float() {
  char line[] = "abc 200.0 50.0 300.0 400.0 15.0 20.0 VOG-17";
  BallisticInputData data = parseInputData(line);

  assert(data.isValid == false);
}

void test_parseInputData_missing_name() {
  char line[] = "100.0 200.0 50.0 300.0 400.0 15.0 20.0";
  BallisticInputData data = parseInputData(line);

  assert(data.isValid == false);
}
