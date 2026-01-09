#pragma once
#include "Controls/ScadaNode.h"
#include <iostream>

enum SCT_TASK_TYPE
{
	READ_OPC_VALUE = 0,
	READ_OPC_VALUE_BATCH,
	WRITE_OPC_VALUE,
	WRITE_OPC_VALUE_BATCH,
	OPC_RECONNECT,
	UPDATE_FTP_GCODE
};

struct TaskReadOpcUAParam
{
public:
	TaskReadOpcUAParam() = default;
	~TaskReadOpcUAParam()
	{
	}

	void* readValue;
	char tag[256];
};

struct TaskReadOpcUABatchParam
{
public:
	TaskReadOpcUABatchParam() = default;
	~TaskReadOpcUABatchParam()
	{
		//tags和readValueBatch内部的字串是全局变量,不需要释放
		free(readValueBatch);
		free(tags);
	}

	void** readValueBatch;
	char** tags;
	int count;
};

struct TaskWriteValueParam
{
public:
	TaskWriteValueParam() = default;
	~TaskWriteValueParam()
	{

	}
	void* writeValue;
	char tag[256];
};

struct TaskWriteValueBatchParam
{
public:
	TaskWriteValueBatchParam() = default;
	~TaskWriteValueBatchParam()
	{
		//tags和writeValueBatch内部的字串是全局变量,不需要释放
		free(writeValueBatch);
		free(tags);
	}

	void** writeValueBatch;
	char** tags;
	int count;
};

struct TaskUpdateRemoteFtpParam
{
public:
	TaskUpdateRemoteFtpParam() = default;

	std::string fileUrl;
};

union SCT_TASK_params
{
	TaskReadOpcUAParam* readOpc = nullptr;
	TaskReadOpcUABatchParam* readOpcBatch;
	TaskWriteValueParam* writeOpc;
	TaskWriteValueBatchParam* writeOpcBatch;
	TaskUpdateRemoteFtpParam* updateRemoteFtp;
};

struct SCT_SEQUENCE_TASK
{
public:
	SCT_SEQUENCE_TASK() = default;
	~SCT_SEQUENCE_TASK() {
		switch (type)
		{
			case READ_OPC_VALUE:
			{
				delete params.readOpc;
				break;
			}
			case READ_OPC_VALUE_BATCH:
			{
				delete params.readOpcBatch;
				break;
			}
			case WRITE_OPC_VALUE:
			{
				delete params.readOpc;
				break;
			}
			case WRITE_OPC_VALUE_BATCH:
			{
				delete params.writeOpcBatch;
				break;
			}
		}
	}
	SCT_TASK_TYPE type;
	SCT_TASK_params params;
};