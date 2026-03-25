#include "Algorithm/AntColonyOptimize.h"
#include <random>
#include <algorithm>
using namespace std;

// 随机数生成工具函数：生成[min, max]范围内的随机整数
int randInt(int min, int max) {
	return rand() % (max - min + 1) + min;
}

// 随机数生成工具函数：生成[0, 1)范围内的随机浮点数
double randDouble() {
	return static_cast<double>(rand()) / RAND_MAX;
}

vector<int> randPerm(int n) {
	vector<int> perm(n);
	iota(perm.begin(), perm.end(), 1); // 初始化为 1,2,...,n

	// 初始化随机数引擎
	std::random_device rd;
	std::mt19937 g(rd());

	// 使用 shuffle 替代 random_shuffle
	std::shuffle(perm.begin(), perm.end(), g);

	return perm;
}

// 查找向量中第一个等于目标值的元素索引（模拟MATLAB的find）
int findFirst(const vector<double>& vec, double target) {
	for (int i = 0; i < vec.size(); ++i) {
		if (abs(vec[i] - target) < 1e-8) {
			return i;
		}
	}
	return -1;
}

AntColonyPathOptimize::AntColonyPathOptimize(const std::vector<EntRingConnection*>& _rings,const glm::vec3& leftBottom) : rings(_rings)
{
    float nesrestDistanceToZero = FLT_MAX;
    for (int i = 0; i < rings.size();i++)
    {
        if (glm::distance(rings[i]->startPoint, leftBottom) < nesrestDistanceToZero)
        {
            idxFirst = i;
            nesrestDistanceToZero = glm::distance(rings[i]->startPoint, leftBottom);
        }
        C.push_back({ rings[i]->startPoint,rings[i]->endPoint });
    }
}

AntColonyPathOptimize::~AntColonyPathOptimize()
{
}

void AntColonyPathOptimize::Execute()
{
	int m = C.size();

	const int n = C.size();
	const double eps = 1e-8;

	vector<vector<double>> D(n, vector<double>(n, 0.0));
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j) {
			if (i != j) {
                //end -> start
				D[i][j] = glm::distance(C[i][1], C[j][0]);
			}
			else {
				D[i][j] = eps;
			}   
            //end -> start
            D[j][i] = glm::distance(C[j][1], C[i][0]);
		}
	}

	// ========== 3. 初始化启发因子和信息素矩阵 ==========
	vector<vector<double>> Eta(n, vector<double>(n, 0.0)); // 启发因子（距离倒数）
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j) {
			Eta[i][j] = 1.0 / D[i][j];
		}
	}

	vector<vector<double>> Tau(n, vector<double>(n, 1.0)); // 信息素矩阵，初始为1

	// ========== 4. 初始化禁忌表和迭代相关变量 ==========
	vector<vector<int>> Tabu(m, vector<int>(n, 0)); // 存储蚂蚁路径（城市编号从1开始）
	int NC = 1; // 迭代计数器

	vector<vector<int>> R_best(G, vector<int>(n, 0)); // 各代最佳路线
	vector<double> L_best(G, 1e20); // 各代最佳路线长度，初始为极大值

	srand(time(0)); // 初始化随机数种子

	while (NC <= G)
	{
        // 步骤1：将m只蚂蚁放到n个城市上
        vector<int> Randpos;
        int repeat = ceil(static_cast<double>(m) / n);
        for (int i = 0; i < repeat; ++i) {
            vector<int> perm = randPerm(n);
            Randpos.insert(Randpos.end(), perm.begin(), perm.end());
        }
        // 给每只蚂蚁分配初始城市（注意：MATLAB是1-based，C++容器是0-based）
        for (int i = 0; i < m; ++i) {
            Tabu[i][0] = Randpos[i];
        }

        // 步骤2：蚂蚁选择下一个城市，完成周游
        for (int j = 1; j < n; ++j) {
            for (int i = 0; i < m; ++i) { // 第i只蚂蚁
                // 已访问的城市
                vector<int> visited(Tabu[i].begin(), Tabu[i].begin() + j);
                // 待访问的城市
                vector<int> J;
                for (int k = 1; k <= n; ++k) { // 城市编号1~n
                    if (find(visited.begin(), visited.end(), k) == visited.end()) {
                        J.push_back(k);
                    }
                }

                // 计算待选城市的概率分布
                vector<double> P(J.size(), 0.0);
                double sumP = 0.0;
                int current_city = Tabu[i][j - 1]; // 当前所在城市
                for (int k = 0; k < J.size(); ++k) {
                    int next_city = J[k];
                    // 注意：矩阵索引转换为0-based
                    P[k] = pow(Tau[current_city - 1][next_city - 1], Alpha) *
                        pow(Eta[current_city - 1][next_city - 1], Beta);
                    sumP += P[k];
                }
                // 归一化概率
                for (int k = 0; k < P.size(); ++k) {
                    P[k] /= sumP;
                }

                // 按概率选择下一个城市（轮盘赌法）
                double rand_val = randDouble();
                double cumP = 0.0;
                int to_visit = J[0];
                for (int k = 0; k < P.size(); ++k) {
                    cumP += P[k];
                    if (cumP >= rand_val) {
                        to_visit = J[k];
                        break;
                    }
                }
                Tabu[i][j] = to_visit;
            }
        }

        // 步骤3：继承上一代最优路线（加速收敛）
        if (NC >= 2) {
            Tabu[0] = R_best[NC - 2]; // 注意：R_best是0-based
        }

        // 步骤4：计算每只蚂蚁的路径长度，记录本次迭代最佳路线
        vector<double> L(m, 0.0);
        for (int i = 0; i < m; ++i) {
            vector<int>& R = Tabu[i];
            for (int j = 0; j < n - 1; ++j) {
                L[i] += D[R[j] - 1][R[j + 1] - 1];
            }
            // 回到起点
            L[i] += D[R[n - 1] - 1][R[0] - 1];
        }

        // 找到本次迭代的最短路径
        double min_L = *min_element(L.begin(), L.end());
        L_best[NC - 1] = min_L;
        int pos = findFirst(L, min_L);
        R_best[NC - 1] = Tabu[pos];

        // 步骤5：更新信息素
        vector<vector<double>> Delta_Tau(n, vector<double>(n, 0.0));
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n - 1; ++j) {
                int from = Tabu[i][j] - 1;
                int to = Tabu[i][j + 1] - 1;
                Delta_Tau[from][to] += Q / L[i];
            }
            // 最后一个城市回到第一个城市
            int from = Tabu[i][n - 1] - 1;
            int to = Tabu[i][0] - 1;
            Delta_Tau[from][to] += Q / L[i];
        }

        // 信息素更新公式：(1-蒸发系数)*旧信息素 + 新增信息素
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                Tau[i][j] = (1 - Rho) * Tau[i][j] + Delta_Tau[i][j];
            }
        }

        // 步骤6：清空禁忌表
        fill(Tabu.begin(), Tabu.end(), vector<int>(n, 0));

        // 步骤7：输出迭代信息（替代MATLAB的绘图）
        cout << "迭代次数: " << NC
            << " | 本次最短距离: " << fixed << setprecision(2) << L_best[NC - 1] << endl;

        NC++;
	}

    double shortest_length = *min_element(L_best.begin(), L_best.end());
    int best_iter = findFirst(L_best, shortest_length);
    vector<int> shortest_route = R_best[best_iter];
    vector<int> temp = shortest_route;

    //if (!temp.empty()) {
    //    int base = shortest_route[0]; // 第一个元素作为基准
    //    // 从第二个元素（下标1）开始遍历，逐一减去base
    //    for (int i = 0; i < temp.size(); ++i) {
    //        temp[i] = ( base - shortest_route[i] + n) % n;
    //    }
    //}

    int order = 0;
    for (int i = 0; i < shortest_route.size();i++)
    {
        int index = shortest_route[i] - 1;
        rings[index]->processOrder = order;
        order++;
    }
    int sentinel = rings[idxFirst]->processOrder;
    for (int i = 0; i < rings.size();i++)
    {
        rings[i]->processOrder = (sentinel - rings[i]->processOrder + n) % n;
    }
}
