#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <assimp/postprocess.h>
#include <fstream>
#include <iostream>
#include <sstream>

#define DEBUG_PRINT() 0

#define LOG_MATRIX(i_mat) logMatrix(i_mat)
template <typename T> void logMatrix(T &i_mat44) {
  std::cout << "Matrix type " << typeid(T).name() << "--------LOGGING------"
            << std::endl;
  for (int j = 0; j < 4; ++j) {
    for (int i = 0; i < 4; ++i) {
      std::cout << i_mat44[j][i] << ", " << std::endl;
    }
  }
}

#define LOG_QUAT(i_quat) logQuat(i_quat)
void logQuat(glm::quat &i_quat) {
  std::ofstream log;
  log.open("QuatLog.txt", std::ios::app);
  log << "-------LOGGING------" << std::endl;
  log << "w = " << i_quat.w << ", "
      << "x = " << i_quat.x << ", "
      << "y = " << i_quat.y << ", "
      << "z = " << i_quat.z << ";" << std::endl;
  log.close();
}

#define LOG_DUALQUAT(i_dualquat) logDualQuat(i_dualquat)
void logDualQuat(glm::fdualquat &i_dualquat) {
  std::ofstream log;
  log.open("DualQuatLog.txt", std::ios::app);
  log << "-------LOGGING------" << std::endl;
  // log real part
  log << "real w = " << i_dualquat.real.w << ", "
      << "real x = " << i_dualquat.real.x << ", "
      << "real y = " << i_dualquat.real.y << ", "
      << "real z = " << i_dualquat.real.z << "; " << std::endl;
  // log dual part
  log << "dual w = " << i_dualquat.dual.w << ", "
      << "dual x = " << i_dualquat.dual.x << ", "
      << "dual y = " << i_dualquat.dual.y << ", "
      << "dual z = " << i_dualquat.dual.z << "; " << std::endl;
  log.close();
}