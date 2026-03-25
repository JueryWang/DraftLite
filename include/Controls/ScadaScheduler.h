#pragma once
#include <thread>
#include <queue>
#include <vector>
#include <mutex>
#include <set>
#include <unordered_set>
#include <QTimer>
#include "Controls/SchedulerTask.h"

class ScadaNode;
class OPClient;
class PLCParam_ProtocalOpc;

enum class DISPACTH_FLAG_BIT : uint64_t
{
	RUNNING = 1ULL,	//thread run flag
	OPC_CONNECT = 1ULL << 1,   //if opc client connecting
	THREAD_CRASHED = 1ULL << 2,  //thread destroyed
	REBOOT_THREAD = 1ULL << 3,
};

enum DISPACTH_FLAG_BIT_POS
{
	BIT_RUN_POS = 0,
	BIT_OPC_CONNECT_POS = 1,
	BIT_THREAD_CRASHED = 2,
	BIT_REBOOT_THREAD = 3
};

class ScadaScheduler
{
public:
	static ScadaScheduler* GetInstance();
	void Start();
	void Stop();
	void AddNode(ScadaNode* node);
	void EraseNode(ScadaNode* node);
	void RegisterReadBackVarKey(const std::string& key);
	void EraseReadBackVarKey(const std::string& key);
	void RegisterReadBackVarKeys(const std::vector<std::string>& keys);
	void ClearReadBackVarKeys();
	void AddTask(SCT_SEQUENCE_TASK* task);
	OPClient* GetOPCommClient() { return opcClient; }
	void SetOPCommClient(OPClient* client) { this->opcClient = client; }

public:
	void inline SetStatus(DISPACTH_FLAG_BIT bit_mask, bool open_bit);
	bool inline GetFlag(DISPACTH_FLAG_BIT_POS pos);

private:
	ScadaScheduler();
	~ScadaScheduler();
	void handleTaskRequest();
	void LoopTask();

private:
	static ScadaScheduler* instance;
	static std::mutex mtx;

	std::vector<ScadaNode*> nodesInControl;

	std::unordered_set<std::string> regTags;
	std::queue<SCT_SEQUENCE_TASK*> taskQueue;

	OPClient* opcClient = nullptr;
	std::thread worker_thread;
	//0-5 reserved |4 - Writing | 5 Reading |6 if opc client connected | 7 thread run 
	std::atomic<uint64_t> state_bit = 0b00000000;
};
