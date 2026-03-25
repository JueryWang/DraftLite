#pragma once
#include "Graphics/DrawEntity.h"
#include "glm/glm.hpp"
#include <iostream>
#include <vector>
#include <cmath>
using namespace CNCSYS;

int randInt(int min, int max);
double randDouble();
std::vector<int> randPerm(int n);
int findFirst(const std::vector<double>& vec, double target);

class AntColonyPathOptimize
{
public:
	AntColonyPathOptimize(const std::vector<EntRingConnection*>& _rings, const glm::vec3& leftBottom);
	~AntColonyPathOptimize();
	void Execute();
private:
	double Alpha = 1.0;			//信息素重要程度
	double Beta = 10.0;			//启发式因子重要程度
	double Rho = 0.2;			//信息素蒸发系数
	int G = 50;				//最大迭代次数
	double Q = 100.0;			//信息素增加强度系数
	std::vector<std::array<glm::vec3,2> > C;  //距离矩阵
	int idxFirst = 0;

	std::vector<EntRingConnection*> rings;
};