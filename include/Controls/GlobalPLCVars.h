#pragma once
#include <open62541/types.h>
#include <Common/AtomicVariables.h>
#include <mutex>
#include <string>
#include <utility>
#include <map>
#include <vector>
#include <chrono>
using namespace std::chrono;

class OPClient;

enum class PLCProtocol
{
	ModbusRTU,
	ModbusTCP,
	OPCUA,
};

struct PLCAddressData
{
	void* bindVar;
	PLCProtocol protocol;
	std::atomic<int> collectionInterval = 3000; //ms
	std::chrono::system_clock::time_point lastUpdateTimeStamp;
};

struct PLCParam_ProtocalOpc : public PLCAddressData
{
	PLCParam_ProtocalOpc() = default;
	~PLCParam_ProtocalOpc() = default;

	int ns;
	char identifier[256];
	AtomicVarType dataType;
	bool updateUI = false;
};

extern std::map<std::string,PLCAddressData*> g_PLCVariables;
extern std::map<std::string, void*> g_readPersistance;
extern std::map<std::string, void*> g_writePersistence;
extern std::map<PLCAddressData*, int> g_ParamUpdateInterval;
extern std::recursive_mutex g_varHandleMutex;
extern std::mutex g_GCodeHandleMutex;
extern OPClient* g_opcuaClient;
extern std::vector<CNCSimulateRecord> g_simRecBufferA;
extern std::vector<CNCSimulateRecord> g_simRecBufferB;
void ClearPLCVariablesOpcUA();
void WritePLC_OPCUA(const char* tag,void* newValue, AtomicVarType type);
void ReadPLC_OPCUA(const char* tag,void* storeValue, AtomicVarType type);
//初始化PLC地址数据(会操作全局变量)
void PLCInitOpcInfo(PLCParam_ProtocalOpc* plcInfo, AtomicVarType type, const std::string& tag, int ns, char* identifier);
void UploadFileToFTP(const std::string& fileName,const std::string& content);