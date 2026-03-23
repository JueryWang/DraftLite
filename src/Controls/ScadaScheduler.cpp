#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/client_subscriptions.h>
#include <open62541/plugin/log_stdout.h>
#include "Controls/ScadaNode.h"
#include "UI/GCodeEditor.h"
#include "Controls/ScadaScheduler.h"
#include "NetWork/OPClient.h"
#include "NetWork/FtpClient.h"
#include "Controls/GlobalPLCVars.h"
#include <iostream>
#include <Common/Program.h>

ScadaScheduler* ScadaScheduler::instance = nullptr;
std::mutex ScadaScheduler::mtx;

ScadaScheduler* ScadaScheduler::GetInstance()
{
	if (instance == nullptr) {
		std::lock_guard<std::mutex> lock(mtx);
		if (instance == nullptr) {
			instance = new ScadaScheduler();
		}
	}
	return instance;
}

void ScadaScheduler::Start()
{
	if (!GetFlag(BIT_RUN_POS))
	{
		if (!worker_thread.joinable())
		{
			SetStatus(DISPACTH_FLAG_BIT::RUNNING, true);
			worker_thread = std::thread(&ScadaScheduler::LoopTask, this);
			worker_thread.detach();
		}
	}
}

void ScadaScheduler::Stop()
{
	SetStatus(DISPACTH_FLAG_BIT::RUNNING, false);
}

void ScadaScheduler::RegisterReadBackVarKey(const std::string& key)
{
	g_varHandleMutex.lock();
	regTags.insert(key);
	g_varHandleMutex.unlock();
}

void ScadaScheduler::EraseReadBackVarKey(const std::string& key)
{
	g_varHandleMutex.lock();
	auto find = std::find(regTags.begin(), regTags.end(), key);
	if (find != regTags.end())
	{
		regTags.erase(find);
	}
	g_varHandleMutex.unlock();
}

void ScadaScheduler::RegisterReadBackVarKeys(const std::vector<std::string>& keys)
{
	g_varHandleMutex.lock();
	for (const std::string& key : keys)
	{
		regTags.insert(key);
	}
	g_varHandleMutex.unlock();
}

void ScadaScheduler::ClearReadBackVarKeys()
{
	g_varHandleMutex.lock();
	regTags.clear();
	g_varHandleMutex.unlock();
}

void ScadaScheduler::AddTask(SCT_SEQUENCE_TASK* task)
{
	taskQueue.push(task);
}

inline void ScadaScheduler::SetStatus(DISPACTH_FLAG_BIT bit_mask, bool open_bit)
{
	if (open_bit)
	{
		uint64_t temp = state_bit.load(std::memory_order_acquire) | ((uint64_t)bit_mask);
		state_bit.store(temp, std::memory_order_release);
	}
	else
	{
		uint8_t temp = state_bit.load(std::memory_order_acquire);
		temp = temp & (((uint64_t)bit_mask) ^ temp);
		state_bit.store(temp, std::memory_order_release);
	}
}

inline bool ScadaScheduler::GetFlag(DISPACTH_FLAG_BIT_POS pos)
{
	return state_bit.load(std::memory_order_consume) & (1 << pos);
}

ScadaScheduler::ScadaScheduler()
{

}

ScadaScheduler::~ScadaScheduler()
{

}

void ScadaScheduler::handleTaskRequest()
{
	while (!taskQueue.empty())
	{
		SCT_SEQUENCE_TASK* task = taskQueue.front();
		taskQueue.pop();
		switch (task->type)
		{
		case READ_OPC_VALUE:
		{
			opcClient->ReadOpcSingle(task->params.readOpc);
			break;
		}
		case READ_OPC_VALUE_BATCH:
		{
			opcClient->ReadOpcBatch(task->params.readOpcBatch);
			break;
		}
		case WRITE_OPC_VALUE:
		{
			opcClient->WriteOpcSingle(task->params.writeOpc);
			break;
		}
		case WRITE_OPC_VALUE_BATCH:
		{
			opcClient->WriteOpcBatch(task->params.writeOpcBatch);
			break;
		}
		case OPC_RECONNECT:
		{
			opcClient->ReconnectWithHint();
			break;
		}
		case UPDATE_FTP_GCODE:
		{
			g_GCodeHandleMutex.lock();
			QString content = GCodeEditor::GetInstance()->text();
			FtpClient::UploadFile(content, QString::fromLocal8Bit(task->params.updateRemoteFtp->fileUrl));
			g_GCodeHandleMutex.unlock();
			break;
		}
		}
		delete task;
	}
}


void ScadaScheduler::AddNode(ScadaNode* node)
{
	if (mtx.try_lock())
	{
		auto find = std::find(nodesInControl.begin(), nodesInControl.end(), node);
		if (find == nodesInControl.end())
		{
			nodesInControl.push_back(node);
		}
		mtx.unlock();
	}
}

void ScadaScheduler::EraseNode(ScadaNode* node)
{
	std::lock_guard<std::mutex> lock(mtx);
	auto find = std::find(nodesInControl.begin(), nodesInControl.end(), node);
	if (find != nodesInControl.end())
	{
		nodesInControl.erase(find);
	}
}

void ScadaScheduler::LoopTask()
{
	while (GetFlag(DISPACTH_FLAG_BIT_POS::BIT_RUN_POS))
	{
		//auto start = std::chrono::system_clock::now();
		if (opcClient->client)
		{
			if (GetFlag(DISPACTH_FLAG_BIT_POS::BIT_OPC_CONNECT_POS))
			{

				g_varHandleMutex.lock();
				//前后处理一次写请求
				if (mtx.try_lock())
				{
 					handleTaskRequest();
					mtx.unlock();
				}
				std::vector<PLCParam_ProtocalOpc*> opcAddresses;
				for (const std::string& tag : regTags)
				{
					if (g_PLCVariables[tag] != NULL)
					{
						opcAddresses.push_back((PLCParam_ProtocalOpc*)g_PLCVariables[tag]);
					}
					else
					{
						/*std::cout << "unread: " << tag << std::endl;*/
					}
				}
				opcClient->ReadBackPLC_ProtoOpcUA(opcAddresses);
				g_varHandleMutex.unlock();

				for (ScadaNode* node : nodesInControl)
				{
					if (node)
					{
						node->UpdateNode();
					}
				}
			}
		}
		//auto end = std::chrono::system_clock::now();
		//auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

