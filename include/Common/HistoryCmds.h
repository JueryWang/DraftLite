#pragma once
#include <list>
#include <string>
#include <utility>
#include <functional>
#include <Graphics/DrawEntity.h>
#include <vector>

#define SAFE_DELETE_PTR(p)                 \
    if (p != nullptr) {                    \
        delete p;                          \
        p = nullptr;                       \
    }

class CNCSYS::SketchGPU;
using namespace CNCSYS;
class HistoryRecorder;

//对象-序列化数据
typedef std::pair<EntityVGPU*, std::string> OperationCommand;

struct HistoryRecord
{
	std::vector<OperationCommand> commands;
	std::function<void(HistoryRecorder*)> cleanFunc = nullptr;
};

class HistoryRecorder
{
public:
	static HistoryRecorder* GetInstance();
	void PushRecord(const HistoryRecord& command);
	//撤销
	void Revoke();
	//恢复
	void Restore();

	void SetSketch(CNCSYS::SketchGPU* sketch);

private:
	//最大撤销步数
	HistoryRecorder(int maxHistory = 50);
	~HistoryRecorder();

private:
	std::list<HistoryRecord> revokeCmds;
	std::list<HistoryRecord> recoverCmds;
	CNCSYS::SketchGPU* sketchHandle = nullptr;
	static HistoryRecorder* instance;
	//一条记录有可能影响多个Entity
	int capacity;
};