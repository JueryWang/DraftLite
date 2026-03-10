#pragma once
#include <open62541/client_config_default.h>
#include <open62541/client.h>
#include <open62541/types.h>
#include <string>
#include <vector>
#include <tuple>
#include <QObject>
#include <functional>
#include "Controls/SchedulerTask.h"

struct PLCParam_ProtocalOpc;
class ScadaMessageHandler;

class OPClient
{
	friend class ScadaScheduler;
public:
	OPClient();
	~OPClient();

	UA_StatusCode ConnectToServer(const char* url);
	void ReadBackPLC_ProtoOpcUA(const std::vector<PLCParam_ProtocalOpc*>& addresses);
	void ReadOpcSingle(TaskReadOpcUAParam* param);
	void ReadOpcBatch(TaskReadOpcUABatchParam* param);
	void WriteOpcSingle(TaskWriteValueParam* param);
	void WriteOpcBatch(TaskWriteValueBatchParam* param);
	void BrowseChildrenRecursive(UA_Client* client, const UA_NodeId* parentNodeId, int depth, int targetNameSapce,std::vector<std::string> &table);
	void InitDirTable(int ns,const std::string& directory);
	bool Reconnect();
	void ReconnectWithHint();

public:
	std::function<void(void)> reconnectCallback;

private:
	void UpdateBindValue(UA_DataValue* varOpc,void* varBind,AtomicVarType type,char* identifier);
	void UpdateBindValue(UA_Variant* varOpc, void* varBind);

private:
	UA_Client* client = nullptr;
	std::string url;

	UA_StatusCode curStatus;
	ScadaMessageHandler* messageHandler;
};
