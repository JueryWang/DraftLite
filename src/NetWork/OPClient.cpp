#include "Controls/ScadaNode.h"
#include "Controls/ScadaScheduler.h"
#include "NetWork/OPClient.h"
#include "NetWork/MessageValidtor.h"
#include "Controls/GlobalPLCVars.h"
#include "Controls/ScadaMessageHandler.h"
#include "UI/Components/HmiInterfaceDefines.h"
#include "UI/Components/HmiTemplateTableMonitor.h"
#include "UI/TaskListWindow.h"
#include "Common/ProgressInfo.h"
#include "Graphics/Sketch.h"
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/client_subscriptions.h>
#include <open62541/plugin/log_stdout.h>
#include <QApplication>
#include <stdio.h>
#include <iostream>
#include <string>
#include <string.h>
#include <algorithm>

#define MAX_READ_BATCH_SIZE 100
#define UA_LOGLEVEL UA_LOGLEVEL_NONE
#include <string>

UA_BrowseRequest bReq;


void OPClient::BrowseChildrenRecursive(UA_Client* client, const UA_NodeId* parentNodeId, int depth, int targetNameSapce, std::vector<std::string>& table) {
	bReq.nodesToBrowseSize = 1;
	bReq.nodesToBrowse[0].nodeId = *parentNodeId;
	bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ISFORWARD;

	UA_BrowseResponse bResp = UA_Client_Service_browse(client, bReq);

	for (size_t i = 0; i < bResp.resultsSize; ++i) {
		for (size_t j = 0; j < bResp.results[i].referencesSize; ++j) {
			UA_ReferenceDescription* ref = &(bResp.results[i].references[j]);

			if (ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_STRING) {
				std::string identifier = std::string((char*)ref->nodeId.nodeId.identifier.string.data);
				identifier.reserve(ref->nodeId.nodeId.identifier.string.length);
				identifier.assign((char*)ref->nodeId.nodeId.identifier.string.data);
				identifier = extractValidOPCUANodes(identifier);
				table.push_back(identifier);
			}

			if (ref->nodeId.nodeId.namespaceIndex == targetNameSapce)
			{
				BrowseChildrenRecursive(client, &ref->nodeId.nodeId, depth + 1, 4, table);
			}
		}
	}
}

OPClient::OPClient()
{
	messageHandler = ScadaMessageHandler::GetInstance();
}

OPClient::~OPClient()
{
	if (client)
	{
		UA_Client_disconnect(client);
		UA_Client_delete(client);
	}
}

UA_StatusCode OPClient::ConnectToServer(const char* url)
{
	this->url = url;
	if (client != nullptr)
		return true;

	if (client != nullptr)
	{
		UA_Client_disconnect(client);
		UA_Client_delete(client);
	}
	client = UA_Client_new();
	UA_ClientConfig_setDefault(UA_Client_getConfig(client));

	// 获取客户端配置
	UA_ClientConfig* config = UA_Client_getConfig(client);
	config->timeout = 200;

	UA_StatusCode status = UA_Client_connect(client, url);

	if (status != UA_STATUSCODE_GOOD) {

		ScadaScheduler::GetInstance()->SetStatus(DISPACTH_FLAG_BIT::OPC_CONNECT, false);

		if (curStatus != status)
		{
			g_file_logger->critical("PLC连接失败:,error code:{}  ||调用函数{}", UA_StatusCode_name(status), __FUNCTION__);
			QMetaObject::invokeMethod(messageHandler, "handleOpcConnectFailed",
				Qt::QueuedConnection,
				Q_ARG(OPClient*, this));
			curStatus = status;
		}
		return EXIT_FAILURE;
	}
	else
	{
		ScadaScheduler::GetInstance()->SetStatus(DISPACTH_FLAG_BIT::OPC_CONNECT, true);
		PLC_TYPE_BOOL pageInit = true;
		WritePLC_OPCUA(g_ConfigableKeys["PageInit"].c_str(), &pageInit, AtomicVarType::BOOL);
	}

	this->url = url;

	return (status == UA_STATUSCODE_GOOD);
}

void OPClient::ReadBackPLC_ProtoOpcUA(const std::vector<PLCParam_ProtocalOpc*>& addresses)
{
	auto now = std::chrono::system_clock::now();
	std::vector<UA_NodeId> nodeIds;

	for (auto& address : addresses)
	{
		if (address != NULL)
		{
			nodeIds.push_back(UA_NODEID_STRING(address->ns, const_cast<char*>(address->identifier)));
		}
	}

	int index = 0;
	int batchSize = std::ceil(double(nodeIds.size()) / MAX_READ_BATCH_SIZE);

	for (int batchId = 0; batchId < batchSize; batchId++)
	{
		int curBatchSize = ((batchId + 1) * MAX_READ_BATCH_SIZE) > nodeIds.size() ? (nodeIds.size() - (batchId)*MAX_READ_BATCH_SIZE) : MAX_READ_BATCH_SIZE;
		size_t nodeCount = nodeIds.size();

		UA_ReadRequest request;
		UA_init(&request, &UA_TYPES[UA_TYPES_READREQUEST]);
		UA_ReadRequest_init(&request);
		request.requestHeader.timestamp = UA_DateTime_now();
		request.nodesToReadSize = curBatchSize;
		request.nodesToRead = (UA_ReadValueId*)UA_Array_new(curBatchSize, &UA_TYPES[UA_TYPES_READVALUEID]);
		for (size_t i = 0; i < curBatchSize; i++)
		{
			UA_ReadValueId_init(&request.nodesToRead[i]);

			// 深拷贝 nodeId，而非直接赋值(浅拷贝)
			UA_NodeId_copy(&nodeIds[(batchId * MAX_READ_BATCH_SIZE) + i],
				&request.nodesToRead[i].nodeId);
			request.nodesToRead[i].attributeId = UA_ATTRIBUTEID_VALUE;
		}

		UA_ReadResponse response = UA_Client_Service_read(client, request);

		if (response.resultsSize == 0)
		{
			if (curStatus != UA_STATUSCODE_BADCONNECTIONCLOSED)
			{
				curStatus = UA_STATUSCODE_BADCONNECTIONCLOSED;
			}
		}

		int skip = 0;
		for (size_t i = 0; i < addresses.size(); i++)
		{
			if ((i - skip) < response.resultsSize)
			{
				UA_DataValue* dataValue = &response.results[i - skip];

				if (addresses[(batchId * MAX_READ_BATCH_SIZE) + i] != NULL)
				{
					UpdateBindValue(dataValue, addresses[(batchId * MAX_READ_BATCH_SIZE) + i]->bindVar, addresses[(batchId * MAX_READ_BATCH_SIZE) + i]->dataType);
					addresses[(batchId * MAX_READ_BATCH_SIZE) + i]->lastUpdateTimeStamp = now;
					addresses[(batchId * MAX_READ_BATCH_SIZE) + i]->updateUI = true;
				}
				else
				{
					skip++;
				}
			}
		}

		UA_ReadRequest_clear(&request);
		UA_ReadResponse_clear(&response);
	}
}

void OPClient::ReadOpcSingle(TaskReadOpcUAParam* param)
{
	UA_Variant dataValue;

	std::string tag(param->tag);
	PLCParam_ProtocalOpc* info = (PLCParam_ProtocalOpc*)g_PLCVariables[tag];
	UA_NodeId nodeId = UA_NODEID_STRING(info->ns, info->identifier);

	UA_StatusCode status = UA_Client_readValueAttribute(client, nodeId, &dataValue);
	UpdateBindValue(&dataValue, param->readValue);

}
void OPClient::ReadOpcBatch(TaskReadOpcUABatchParam* param)
{
	std::vector<UA_NodeId> nodeIds;


	for (int i = 0; i < param->count; i++)
	{
		std::string tag(param->tags[i]);
		PLCParam_ProtocalOpc* info = (PLCParam_ProtocalOpc*)g_PLCVariables[tag];
		nodeIds.push_back(UA_NODEID_STRING(info->ns, info->identifier));
	}

	int index = 0;
	int batchSize = std::ceil(double(nodeIds.size()) / MAX_READ_BATCH_SIZE);

	for (int batchId = 0; batchId < batchSize; batchId++)
	{
		int curBatchSize = ((batchId + 1) * MAX_READ_BATCH_SIZE) > nodeIds.size() ? (nodeIds.size() - batchId * MAX_READ_BATCH_SIZE) : MAX_READ_BATCH_SIZE;

		UA_ReadRequest request;
		UA_ReadRequest_init(&request);
		request.nodesToReadSize = param->count;
		request.nodesToRead = (UA_ReadValueId*)UA_Array_new(curBatchSize, &UA_TYPES[UA_TYPES_READVALUEID]);
		for (size_t i = 0; i < curBatchSize; i++)
		{
			UA_ReadValueId_init(&request.nodesToRead[i]);
			UA_NodeId_copy(&nodeIds[(batchId * MAX_READ_BATCH_SIZE) + i],
				&request.nodesToRead[i].nodeId);
			request.nodesToRead[i].attributeId = UA_ATTRIBUTEID_VALUE;
		}

		UA_ReadResponse response = UA_Client_Service_read(client, request);

		for (size_t i = 0; i < response.resultsSize; i++)
		{
			UA_DataValue* dataValue = &response.results[i];

			//UpdateBindValue(dataValue, param->readValueBatch[batchId * MAX_READ_BATCH_SIZE + i]);
		}

		UA_ReadResponse_clear(&response);
	}
	delete param;
}

void OPClient::WriteOpcSingle(TaskWriteValueParam* param)
{
	UA_Variant variant;

	std::string tag(param->tag);
	PLCParam_ProtocalOpc* info = (PLCParam_ProtocalOpc*)g_PLCVariables[tag];
	if (info != nullptr)
	{
		UA_NodeId nodeId = UA_NODEID_STRING(info->ns, info->identifier);
		UA_StatusCode status;

		QString hintValueStr;

		switch (info->dataType)
		{
		case AtomicVarType::BOOL:
		{
			UA_Boolean valueToWrite = static_cast<AtomicVar<PLC_TYPE_BOOL>*>(param->writeValue)->GetValue();
			hintValueStr = (bool)valueToWrite ? "True" : "False";
			UA_Variant_setScalar(&variant, &valueToWrite, &UA_TYPES[UA_TYPES_BOOLEAN]);
			status = UA_Client_writeValueAttribute(client, nodeId, &variant);
			break;
		}
		case AtomicVarType::WORD:
		{
			UA_UInt16 valueToWrite = static_cast<AtomicVar<PLC_TYPE_WORD>*>(param->writeValue)->GetValue();
			hintValueStr = QString::number(valueToWrite);
			UA_Variant_setScalar(&variant, &valueToWrite, &UA_TYPES[UA_TYPES_UINT16]);
			status = UA_Client_writeValueAttribute(client, nodeId, &variant);
			break;
		}
		case AtomicVarType::DWORD:
		{
			UA_UInt32 valueToWrite = static_cast<AtomicVar<PLC_TYPE_DWORD>*>(param->writeValue)->GetValue();
			UA_Variant_setScalar(&variant, &valueToWrite, &UA_TYPES[UA_TYPES_UINT32]);
			status = UA_Client_writeValueAttribute(client, nodeId, &variant);
			hintValueStr = QString::number(valueToWrite);
			break;
		}
		case AtomicVarType::LWORD:
		{
			UA_UInt64 valueToWrite = static_cast<AtomicVar<PLC_TYPE_LWORD>*>(param->writeValue)->GetValue();
			UA_Variant_setScalar(&variant, &valueToWrite, &UA_TYPES[UA_TYPES_UINT64]);
			status = UA_Client_writeValueAttribute(client, nodeId, &variant);
			hintValueStr = QString::number(valueToWrite);
			break;
		}
		case AtomicVarType::INT:
		{
			UA_Int16 valueToWrite = static_cast<AtomicVar<PLC_TYPE_INT>*>(param->writeValue)->GetValue();
			UA_Variant_setScalar(&variant, &valueToWrite, &UA_TYPES[UA_TYPES_INT16]);
			status = UA_Client_writeValueAttribute(client, nodeId, &variant);
			hintValueStr = QString::number(valueToWrite);
			break;
		}
		case AtomicVarType::DINT:
		{
			UA_Int32 valueToWrite = static_cast<AtomicVar<PLC_TYPE_DINT>*>(param->writeValue)->GetValue();
			UA_Variant_setScalar(&variant, &valueToWrite, &UA_TYPES[UA_TYPES_INT32]);
			status = UA_Client_writeValueAttribute(client, nodeId, &variant);
			hintValueStr = QString::number(valueToWrite);
			break;
		}
		case AtomicVarType::LINT:
		{
			UA_Int64 valueToWrite = static_cast<AtomicVar<PLC_TYPE_LINT>*>(param->writeValue)->GetValue();
			UA_Variant_setScalar(&variant, &valueToWrite, &UA_TYPES[UA_TYPES_INT64]);
			status = UA_Client_writeValueAttribute(client, nodeId, &variant);
			hintValueStr = QString::number(valueToWrite);
			break;
		}
		case AtomicVarType::REAL:
		{
			UA_Float valueToWrite = static_cast<AtomicVar<PLC_TYPE_REAL>*>(param->writeValue)->GetValue();
			UA_Variant_setScalar(&variant, &valueToWrite, &UA_TYPES[UA_TYPES_FLOAT]);
			status = UA_Client_writeValueAttribute(client, nodeId, &variant);
			hintValueStr = QString::number(valueToWrite);
			break;
		}
		case AtomicVarType::LREAL:
		{
			UA_Double valueToWrite = static_cast<AtomicVar<PLC_TYPE_LREAL>*>(param->writeValue)->GetValue();
			UA_Variant_setScalar(&variant, &valueToWrite, &UA_TYPES[UA_TYPES_DOUBLE]);
			status = UA_Client_writeValueAttribute(client, nodeId, &variant);
			hintValueStr = QString::number(valueToWrite);
			break;
		}
		case AtomicVarType::STRING:
		{
			const char* str = static_cast<AtomicVar<PLC_TYPE_STRING>*>(param->writeValue)->GetValue();
			UA_String valueToWrite = UA_STRING((char*)str);
			UA_Variant_setScalar(&variant, &valueToWrite, &UA_TYPES[UA_TYPES_STRING]);
			status = UA_Client_writeValueAttribute(client, nodeId, &variant);
			hintValueStr = QString::fromLocal8Bit(str);
			break;
		}
		}
		if (status != UA_STATUSCODE_GOOD && status != curStatus)
		{
			const char* description = UA_StatusCode_name(status);
			std::string errorStr;

			QMetaObject::invokeMethod(messageHandler, "handleOpcWriteFailed",
				Qt::QueuedConnection,
				Q_ARG(OPClient*, this),
				Q_ARG(QString, QString::fromLocal8Bit(info->identifier),
					Q_ARG(QString, hintValueStr),
					Q_ARG(QString, QString::fromLocal8Bit(description))
				));
			g_file_logger->critical("写入节点{}失败:{},error code:{}  ||调用函数{}", param->tag, UA_StatusCode_name(status), __FUNCTION__);
		}
		curStatus = status;

	}

}
void OPClient::WriteOpcBatch(TaskWriteValueBatchParam* param)
{
	//std::vector<UA_NodeId> nodeIds;

	//for (int i = 0; i < param->count; i++)
	//{
	//    nodeIds.push_back(UA_NODEID_STRING(param->ua_namespaces[i],param->ua_identifiers[i]));
	//}

	//UA_WriteRequest request;
	//UA_WriteRequest_init(&request);

	//request.nodesToWriteSize = nodeIds.size();
}



std::string extractAfterDot(const std::string& str, int count) {
	int dotCount = 0;
	for (int i = 0; i <= str.size() - 1; ++i) {
		if (str[i] == '.') {
			dotCount++;
			if (dotCount == count) {
				return str.substr(i + 1);
			}
		}
	}
	return ""; // 或 return str;
}

inline std::string getLastVariableName(const std::string& input) {
	size_t lastDotPos = input.find_last_of('.');

	if (lastDotPos == std::string::npos) {
		return input;
	}

	return input.substr(lastDotPos + 1);
}

inline std::string getVarPrefix(const std::string& input)
{
	size_t lastDotPos = input.find_last_of('.');

	if (lastDotPos == std::string::npos) {
		return input;
	}
	return input.substr(0, lastDotPos);
}

bool contains(const std::string& str, const std::string& subStr) {
	auto it = std::search(str.begin(), str.end(), subStr.begin(), subStr.end());
	return it != str.end();
}

void OPClient::InitDirTable(int ns, const std::string& directory)
{
	if (client)
	{
		QApplication::setOverrideCursor(Qt::WaitCursor);
		ScadaScheduler* dispatcher = ScadaScheduler::GetInstance();
		ClearPLCVariablesOpcUA();

		auto startSearchNodes = std::chrono::system_clock::now();

		UA_NodeId parentNodeId = UA_NODEID_STRING(ns, const_cast<char*>(directory.data()));
		std::vector<std::string> table;

		UA_BrowseRequest_init(&bReq);
		bReq.requestedMaxReferencesPerNode = 0;
		bReq.nodesToBrowse = UA_BrowseDescription_new();
		BrowseChildrenRecursive(client, &parentNodeId, 0, ns, table);

		auto endSearchNodes = std::chrono::system_clock::now();
		auto searchDuration = endSearchNodes - startSearchNodes;
		int searchCostsInMilliSec = std::chrono::duration_cast<std::chrono::milliseconds>(searchDuration).count();
		g_file_logger->info("扫描OPCUA结构树耗时:{}  ||调用函数{}", searchCostsInMilliSec, __FUNCTION__);
		int dotCount = std::count(directory.begin(), directory.end(), '.');

		std::vector<std::string> RegKeys;

		auto startAssignVars = std::chrono::system_clock::now();
		for (const std::string& identifier : table)
		{
			UA_NodeId variableNodeId = UA_NODEID_STRING(ns, const_cast<char*>(identifier.data()));
			UA_Variant variant;

			UA_StatusCode retval = UA_Client_readValueAttribute(client, variableNodeId, &variant);
			if (retval == UA_STATUSCODE_GOOD)
			{
				PLCParam_ProtocalOpc* plcInfo = new PLCParam_ProtocalOpc();
				plcInfo->ns = ns;

				strncpy(plcInfo->identifier, identifier.c_str(), sizeof(plcInfo->identifier) - 1);
				std::string varName = extractAfterDot(identifier, dotCount);

				if (UA_Variant_hasScalarType(&variant, &UA_TYPES[UA_TYPES_STRING]))
				{
					PLCInitOpcInfo(plcInfo, AtomicVarType::STRING, varName, ns, const_cast<char*>(identifier.data()));
					RegKeys.push_back(varName);
				}
				else if (UA_Variant_hasScalarType(&variant, &UA_TYPES[UA_TYPES_DOUBLE]))
				{
					PLCInitOpcInfo(plcInfo, AtomicVarType::LREAL, varName, ns, const_cast<char*>(identifier.data()));
					RegKeys.push_back(varName);
				}
				else if (UA_Variant_hasScalarType(&variant, &UA_TYPES[UA_TYPES_FLOAT]))
				{
					PLCInitOpcInfo(plcInfo, AtomicVarType::REAL, varName, ns, const_cast<char*>(identifier.data()));
					RegKeys.push_back(varName);
				}
				else if (UA_Variant_hasScalarType(&variant, &UA_TYPES[UA_TYPES_INT16]))
				{
					PLCInitOpcInfo(plcInfo, AtomicVarType::INT, varName, ns, const_cast<char*>(identifier.data()));
					RegKeys.push_back(varName);
				}
				else if (UA_Variant_hasScalarType(&variant, &UA_TYPES[UA_TYPES_INT32]))
				{
					PLCInitOpcInfo(plcInfo, AtomicVarType::DINT, varName, ns, const_cast<char*>(identifier.data()));
					RegKeys.push_back(varName);
				}
				else if (UA_Variant_hasScalarType(&variant, &UA_TYPES[UA_TYPES_INT64]))
				{
					PLCInitOpcInfo(plcInfo, AtomicVarType::LINT, varName, ns, const_cast<char*>(identifier.data()));
					RegKeys.push_back(varName);
				}
				else if (UA_Variant_hasScalarType(&variant, &UA_TYPES[UA_TYPES_UINT16]))
				{
					PLCInitOpcInfo(plcInfo, AtomicVarType::WORD, varName, ns, const_cast<char*>(identifier.data()));
					RegKeys.push_back(varName);
				}
				else if (UA_Variant_hasScalarType(&variant, &UA_TYPES[UA_TYPES_UINT32]))
				{
					PLCInitOpcInfo(plcInfo, AtomicVarType::DWORD, varName, ns, const_cast<char*>(identifier.data()));
					RegKeys.push_back(varName);
				}
				else if (UA_Variant_hasScalarType(&variant, &UA_TYPES[UA_TYPES_UINT64]))
				{
					PLCInitOpcInfo(plcInfo, AtomicVarType::LWORD, varName, ns, const_cast<char*>(identifier.data()));
					RegKeys.push_back(varName);
				}
				else if (UA_Variant_hasScalarType(&variant, &UA_TYPES[UA_TYPES_BOOLEAN]))
				{
					PLCInitOpcInfo(plcInfo, AtomicVarType::BOOL, varName, ns, const_cast<char*>(identifier.data()));
					RegKeys.push_back(varName);
				}
			}
		}

		auto endAssignVars = std::chrono::system_clock::now();
		auto assignDuration = endAssignVars - startAssignVars;
		int assignCostsInMilliSec = std::chrono::duration_cast<std::chrono::milliseconds>(assignDuration).count();
		g_file_logger->info("分配OPCUA变量耗时:{}  ||调用函数{}", assignCostsInMilliSec, __FUNCTION__);
		QApplication::restoreOverrideCursor();

		ScadaScheduler::GetInstance()->RegisterReadBackVarKey("gvlHMI.stParameterGearChamferMachine.stParameterCADWork.sWorkFileName");
	}
}

bool OPClient::Reconnect()
{
	UA_StatusCode retval = UA_Client_connect(client, this->url.c_str());
	if (retval == UA_STATUSCODE_GOOD)
	{
		ScadaScheduler::GetInstance()->SetStatus(DISPACTH_FLAG_BIT::OPC_CONNECT, true);
		g_opcuaClient = this;
	}
	curStatus = retval;
	return retval == UA_STATUSCODE_GOOD;
}

void OPClient::ReconnectWithHint()
{
	UA_StatusCode retval = UA_Client_connect(client, this->url.c_str());
	if (retval == UA_STATUSCODE_GOOD)
	{
		ScadaScheduler::GetInstance()->SetStatus(DISPACTH_FLAG_BIT::OPC_CONNECT, true);
		PLC_TYPE_BOOL pageInit = true;
		WritePLC_OPCUA(g_ConfigableKeys["PageInit"].c_str(), &pageInit, AtomicVarType::BOOL);
		std::vector<SketchGPU*> sketches = TaskListWindow::GetInstance()->GetAllTaskSketches();
		for (SketchGPU* sketch : sketches)
		{
			UploadFileToFTP(sketch->source, sketch->ToNcProgram());
		}
	}
	else
	{
		QMetaObject::invokeMethod(messageHandler, "handleOpcReconnectFailed",
			Qt::QueuedConnection,
			Q_ARG(OPClient*, this));
		ScadaScheduler::GetInstance()->SetStatus(DISPACTH_FLAG_BIT::OPC_CONNECT, false);
	}
	if (reconnectCallback)
	{
		reconnectCallback();
	}
	curStatus = retval;
}

void OPClient::UpdateBindValue(UA_DataValue* varOpc, void* varBind, AtomicVarType type)
{
	if (varOpc->status != UA_STATUSCODE_GOOD) {
		g_file_logger->error("OPC 节点 读取状态异常:{}  ||调用函数{}", UA_StatusCode_name(varOpc->status), __FUNCTION__);
	}

	if (varOpc->value.type == &UA_TYPES[UA_TYPES_STRING])
	{
		assert(type == AtomicVarType::STRING);
		UA_String* byteString = (UA_String*)varOpc->value.data;
		if (byteString && byteString->length >= 1)
		{
			AtomicVar<PLC_TYPE_STRING>* oldValue = static_cast<AtomicVar<PLC_TYPE_STRING>*>(varBind);
			const char* newValue = (const char*)byteString->data;
			oldValue->SetValue(newValue, byteString->length);
		}
		else
		{
			AtomicVar<PLC_TYPE_STRING>* oldValue = static_cast<AtomicVar<PLC_TYPE_STRING>*>(varBind);
			const char* empty = "";
			*oldValue = empty;
		}
	}
	else if (varOpc->value.type == &UA_TYPES[UA_TYPES_DOUBLE])
	{
		assert(type == AtomicVarType::LREAL);
		double newValue = *(UA_Double*)varOpc->value.data;
		AtomicVar<PLC_TYPE_LREAL>* oldValue = static_cast<AtomicVar<PLC_TYPE_LREAL>*>(varBind);
		*oldValue = newValue;
	}
	else if (varOpc->value.type == &UA_TYPES[UA_TYPES_FLOAT])
	{
		assert(type == AtomicVarType::REAL);
		float newValue = *(UA_Float*)varOpc->value.data;
		AtomicVar<PLC_TYPE_REAL>* oldValue = static_cast<AtomicVar<PLC_TYPE_REAL>*>(varBind);
		*oldValue = newValue;
	}
	else if (varOpc->value.type == &UA_TYPES[UA_TYPES_INT16])
	{
		assert(type == AtomicVarType::INT);
		UA_Int16 newValue = *(UA_Int16*)varOpc->value.data;
		AtomicVar<PLC_TYPE_INT>* oldValue = static_cast<AtomicVar<PLC_TYPE_INT>*>(varBind);
		*oldValue = newValue;
	}
	else if (varOpc->value.type == &UA_TYPES[UA_TYPES_INT32])
	{
		assert(type == AtomicVarType::DINT);
		UA_Int32 newValue = *(UA_Int32*)varOpc->value.data;
		AtomicVar<PLC_TYPE_DINT>* oldValue = static_cast<AtomicVar<PLC_TYPE_DINT>*>(varBind);
		*oldValue = newValue;
	}
	else if (varOpc->value.type == &UA_TYPES[UA_TYPES_INT64])
	{
		assert(type == AtomicVarType::LINT);
		UA_Int64 newValue = *(UA_Int64*)varOpc->value.data;
		AtomicVar<PLC_TYPE_LINT>* oldValue = static_cast<AtomicVar<PLC_TYPE_LINT>*>(varBind);
		*oldValue = newValue;
	}
	else if (varOpc->value.type == &UA_TYPES[UA_TYPES_UINT16])
	{
		assert(type == AtomicVarType::WORD);
		UA_UInt16 newValue = *(UA_UInt16*)varOpc->value.data;
		AtomicVar<PLC_TYPE_WORD>* oldValue = static_cast<AtomicVar<PLC_TYPE_WORD>*>(varBind);
		*oldValue = newValue;
	}
	else if (varOpc->value.type == &UA_TYPES[UA_TYPES_UINT32])
	{
		assert(type == AtomicVarType::DWORD);
		UA_UInt32 newValue = *(UA_UInt32*)varOpc->value.data;
		AtomicVar<PLC_TYPE_DWORD>* oldValue = static_cast<AtomicVar<PLC_TYPE_DWORD>*>(varBind);
		*oldValue = newValue;
	}
	else if (varOpc->value.type == &UA_TYPES[UA_TYPES_UINT64])
	{
		assert(type == AtomicVarType::LWORD);
		UA_UInt64 newValue = *(UA_UInt64*)varOpc->value.data;
		AtomicVar<PLC_TYPE_LWORD>* oldValue = static_cast<AtomicVar<PLC_TYPE_LWORD>*>(varBind);
		*oldValue = newValue;
	}
	else if (varOpc->value.type == &UA_TYPES[UA_TYPES_BOOLEAN])
	{
		assert(type == AtomicVarType::BOOL);
		UA_Boolean newValue = *(UA_Boolean*)varOpc->value.data;
		AtomicVar<PLC_TYPE_BOOL>* oldValue = static_cast<AtomicVar<PLC_TYPE_BOOL>*>(varBind);
		*oldValue = newValue;
	}
}

void OPClient::UpdateBindValue(UA_Variant* varOpc, void* varBind)
{
	if (UA_Variant_hasScalarType(varOpc, &UA_TYPES[UA_TYPES_STRING]))
	{
		UA_String* byteString = (UA_String*)varOpc->data;
		AtomicVar<PLC_TYPE_STRING>* oldValue = static_cast<AtomicVar<PLC_TYPE_STRING>*>(varBind);
		const char* newValue = (const char*)byteString->data;
		*oldValue = newValue;
	}
	else if (UA_Variant_hasScalarType(varOpc, &UA_TYPES[UA_TYPES_DOUBLE]))
	{
		double newValue = *(UA_Double*)varOpc->data;
		AtomicVar<PLC_TYPE_LREAL>* oldValue = static_cast<AtomicVar<PLC_TYPE_LREAL>*>(varBind);
		*oldValue = newValue;
	}
	else if (UA_Variant_hasScalarType(varOpc, &UA_TYPES[UA_TYPES_FLOAT]))
	{
		float newValue = *(UA_Float*)varOpc->data;
		AtomicVar<PLC_TYPE_REAL>* oldValue = static_cast<AtomicVar<PLC_TYPE_REAL>*>(varBind);
		*oldValue = newValue;
	}
	else if (UA_Variant_hasScalarType(varOpc, &UA_TYPES[UA_TYPES_INT16]))
	{
		UA_Int16 newValue = *(UA_Int16*)varOpc->data;
		AtomicVar<PLC_TYPE_INT>* oldValue = static_cast<AtomicVar<PLC_TYPE_INT>*>(varBind);
		*oldValue = newValue;
	}
	else if (UA_Variant_hasScalarType(varOpc, &UA_TYPES[UA_TYPES_INT32]))
	{
		UA_Int32 newValue = *(UA_Int32*)varOpc->data;
		AtomicVar<PLC_TYPE_DINT>* oldValue = static_cast<AtomicVar<PLC_TYPE_DINT>*>(varBind);
		*oldValue = newValue;
	}
	else if (UA_Variant_hasScalarType(varOpc, &UA_TYPES[UA_TYPES_INT64]))
	{
		UA_Int64 newValue = *(UA_Int64*)varOpc->data;
		AtomicVar<PLC_TYPE_LINT>* oldValue = static_cast<AtomicVar<PLC_TYPE_LINT>*>(varBind);
		*oldValue = newValue;
	}
	else if (UA_Variant_hasScalarType(varOpc, &UA_TYPES[UA_TYPES_UINT16]))
	{
		UA_UInt16 newValue = *(UA_UInt16*)varOpc->data;
		AtomicVar<PLC_TYPE_WORD>* oldValue = static_cast<AtomicVar<PLC_TYPE_WORD>*>(varBind);
		*oldValue = newValue;
	}
	else if (UA_Variant_hasScalarType(varOpc, &UA_TYPES[UA_TYPES_UINT32]))
	{
		UA_UInt32 newValue = *(UA_UInt32*)varOpc->data;
		AtomicVar<PLC_TYPE_DWORD>* oldValue = static_cast<AtomicVar<PLC_TYPE_DWORD>*>(varBind);
		*oldValue = newValue;
	}
	else if (UA_Variant_hasScalarType(varOpc, &UA_TYPES[UA_TYPES_UINT32]))
	{
		UA_UInt64 newValue = *(UA_UInt64*)varOpc->data;
		AtomicVar<PLC_TYPE_LWORD>* oldValue = static_cast<AtomicVar<PLC_TYPE_LWORD>*>(varBind);
		*oldValue = newValue;
	}
	else if (UA_Variant_hasScalarType(varOpc, &UA_TYPES[UA_TYPES_BOOLEAN]))
	{
		UA_Boolean newValue = *(UA_Boolean*)varOpc->data;
		AtomicVar<PLC_TYPE_BOOL>* oldValue = static_cast<AtomicVar<PLC_TYPE_BOOL>*>(varBind);
		*oldValue = newValue;
	}
}
