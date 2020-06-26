#pragma once
#include <glm.hpp>
#include <gtc\matrix_transform.hpp>

#include <fstream>
#include <sstream>
#include <iostream>
#include "assimp\postprocess.h"

using namespace std;

void debuggingaiMatrix(aiMatrix4x4 mat)
{
	ofstream log;
	log.open("aiMatrixLog.txt", ios::app);
	log << "This is aiMatrix4x4 ------------------" << endl;
	for (int r = 0; r < 4; ++r)
	{
		for (int c = 0; c < 4; ++c)
		{
			//std::cout << mat[r][c] << "  ";
			log << mat[r][c] << "  ";
		}
		//std::cout << std::endl;
		log << endl;
	}
	log.close();
}

void debuggingMatrix(glm::mat4 array)
{
	ofstream log;
	log.open("MatrixLog.txt", ios::app);
	log << "This is glm::mat4x4 ------------------" << endl;
	for (int r = 0; r < 4; ++r)
	{
		for (int c = 0; c < 4; ++c)
		{
			//std::cout << array[r][c] << "  ";
			log << array[r][c] << "  ";
		}
		//std::cout << std::endl;
		log << endl;
	}
	log.close();
}

void debuggingQuat(glm::quat q) {
	ofstream log;
	log.open("QuatLog.txt", ios::app);
	log << "This is glm::quat ------------------" << endl;
	log << "w = " << q.w << ", " << "x = " << q.x << ", " << "y = " << q.y << ", " << "z = " << q.z << ";" << endl;
	log.close();
}


void debuggingDualQuat(glm::fdualquat dq) {
	ofstream log;
	log.open("DualQuatLog.txt", ios::app);
	log << "This is dual_quat ------------------" << endl;
	log << "real w = " << dq.real.w << ", "
		<< "real x = " << dq.real.x << ", "
		<< "real y = " << dq.real.y << ", "
		<< "real z = " << dq.real.z << "; "
		<< endl;
	log << "dual w = " << dq.dual.w << ", "
		<< "dual x = " << dq.dual.x << ", "
		<< "dual y = " << dq.dual.y << ", "
		<< "dual z = " << dq.dual.z << "; "
		<< endl;

	log.close();
}