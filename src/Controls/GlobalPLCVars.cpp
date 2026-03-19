#include "Controls/GlobalPLCVars.h"
#include "Controls/ScadaScheduler.h"
#include "Controls//ScadaScheduler.h"
#include "NetWork/FtpClient.h"
#include "Common/ProgressInfo.h"
#include <QString>
#include <QFileInfo>

std::map<std::string, PLCAddressData*> g_PLCVariables;
std::map<std::string, void*> g_readPersistance;
std::map<std::string, void*> g_writePersistence;
std::map<PLCAddressData*, int> g_ParamUpdateInterval;
std::recursive_mutex g_varHandleMutex;
std::mutex g_GCodeHandleMutex;
std::vector<CNCSimulateRecord> g_simRecBufferA;
std::vector<CNCSimulateRecord> g_simRecBufferB;
std::array<bool, 10> g_stationPCChange;
std::array<bool, 10>g_stationPCChangeDone;
std::array<bool, 10> g_stationPCFileFTP;
std::array<bool, 10> g_stationPCFileFTPDone;
int stationSize = 0;
OPClient* g_opcuaClient = nullptr;

void ClearPLCVariablesOpcUA()
{
	for (auto& pair : g_PLCVariables)
	{
		delete pair.second;
	}

	g_PLCVariables.clear();
	for (auto& pair : g_readPersistance)
	{
		delete pair.second;
	}
	g_readPersistance.clear();
	for (auto& pair : g_readPersistance)
	{
		delete pair.second;
	}
	g_writePersistence.clear();
}

void WritePLC_OPCUA(const char* tag, void* newValue, AtomicVarType type)
{
	if (g_opcuaClient)
	{
		ScadaScheduler* dispatcher = ScadaScheduler::GetInstance();
		SCT_SEQUENCE_TASK* writeTask = new SCT_SEQUENCE_TASK();
		writeTask->type = WRITE_OPC_VALUE;
		writeTask->params.writeOpc = new TaskWriteValueParam();
		switch (type)
		{
		case AtomicVarType::None:
			break;
		case AtomicVarType::BOOL:
		{
			void* ptr = g_writePersistence[tag];
			AtomicVar<PLC_TYPE_BOOL>* writeValue = static_cast<AtomicVar<PLC_TYPE_BOOL>*>(g_writePersistence[tag]);
			if (writeValue != nullptr)
			{
				*writeValue = *((PLC_TYPE_BOOL*)newValue);
				writeTask->params.writeOpc->writeValue = writeValue;
			}
			else
			{
				delete writeTask;
				return;
			}
			break;
		}
		case AtomicVarType::WORD:
		{
			AtomicVar<PLC_TYPE_WORD>* writeValue = static_cast<AtomicVar<PLC_TYPE_WORD>*>(g_writePersistence[tag]);
			*writeValue = *((PLC_TYPE_WORD*)newValue);
			writeTask->params.writeOpc->writeValue = writeValue;
			break;
		}
		case AtomicVarType::DWORD:
		{
			AtomicVar<PLC_TYPE_DWORD>* writeValue = static_cast<AtomicVar<PLC_TYPE_DWORD>*>(g_writePersistence[tag]);
			*writeValue = *((PLC_TYPE_DWORD*)newValue);
			writeTask->params.writeOpc->writeValue = writeValue;
			break;
		}
		case AtomicVarType::LWORD:
		{
			AtomicVar<PLC_TYPE_LWORD>* writeValue = static_cast<AtomicVar<PLC_TYPE_LWORD>*>(g_writePersistence[tag]);
			*writeValue = *((PLC_TYPE_LWORD*)newValue);
			writeTask->params.writeOpc->writeValue = writeValue;
			break;
		}
		case AtomicVarType::INT:
		{
			AtomicVar<PLC_TYPE_INT>* writeValue = static_cast<AtomicVar<PLC_TYPE_INT>*>(g_writePersistence[tag]);
			*writeValue = *((PLC_TYPE_INT*)newValue);
			writeTask->params.writeOpc->writeValue = writeValue;
			break;
		}
		case AtomicVarType::DINT:
		{
			AtomicVar<PLC_TYPE_DINT>* writeValue = static_cast<AtomicVar<PLC_TYPE_DINT>*>(g_writePersistence[tag]);
			*writeValue = *((PLC_TYPE_DINT*)newValue);
			writeTask->params.writeOpc->writeValue = writeValue;
			break;
		}
		case AtomicVarType::LINT:
		{
			AtomicVar<PLC_TYPE_LINT>* writeValue = static_cast<AtomicVar<PLC_TYPE_LINT>*>(g_writePersistence[tag]);
			*writeValue = *((PLC_TYPE_LINT*)newValue);
			writeTask->params.writeOpc->writeValue = writeValue;
			break;
		}
		case AtomicVarType::REAL:
		{
			AtomicVar<PLC_TYPE_REAL>* writeValue = static_cast<AtomicVar<PLC_TYPE_REAL>*>(g_writePersistence[tag]);
			*writeValue = *((PLC_TYPE_REAL*)newValue);
			writeTask->params.writeOpc->writeValue = writeValue;
			break;
		}
		case AtomicVarType::LREAL:
		{
			AtomicVar<PLC_TYPE_LREAL>* writeValue = static_cast<AtomicVar<PLC_TYPE_LREAL>*>(g_writePersistence[tag]);
			*writeValue = *((PLC_TYPE_LREAL*)newValue);
			writeTask->params.writeOpc->writeValue = writeValue;
			break;
		}
		case AtomicVarType::STRING:
		{
			AtomicVar<PLC_TYPE_STRING>* writeValue = static_cast<AtomicVar<PLC_TYPE_STRING>*>(g_writePersistence[tag]);
			*writeValue = ((PLC_TYPE_STRING)newValue);
			writeTask->params.writeOpc->writeValue = writeValue;
			break;
		}
		default:
			break;
		}
		strcpy_s(writeTask->params.writeOpc->tag, tag);
		dispatcher->AddTask(writeTask);
	}
}

void ReadPLC_OPCUA(const char* tag, void* storeValue, AtomicVarType type)
{
	if (g_opcuaClient)
	{
		switch (type)
		{
		case AtomicVarType::None:
			break;
		case AtomicVarType::BOOL:
		{
			AtomicVar<PLC_TYPE_BOOL>* readValue = static_cast<AtomicVar<PLC_TYPE_BOOL>*>(g_readPersistance[tag]);
			if (readValue)
			{
				*((PLC_TYPE_BOOL*)storeValue) = *readValue;
			}
			break;
		}
		case AtomicVarType::WORD:
		{
			AtomicVar<PLC_TYPE_WORD>* readValue = static_cast<AtomicVar<PLC_TYPE_WORD>*>(g_readPersistance[tag]);
			if (readValue)
			{
				*((PLC_TYPE_WORD*)storeValue) = *readValue;
			}
			break;
		}
		case AtomicVarType::DWORD:
		{
			AtomicVar<PLC_TYPE_DWORD>* readValue = static_cast<AtomicVar<PLC_TYPE_DWORD>*>(g_readPersistance[tag]);
			if (readValue)
			{
				*((PLC_TYPE_DWORD*)storeValue) = *readValue;
			}
			break;
		}
		case AtomicVarType::LWORD:
		{
			AtomicVar<PLC_TYPE_LWORD>* readValue = static_cast<AtomicVar<PLC_TYPE_LWORD>*>(g_readPersistance[tag]);
			if (readValue)
			{
				*((PLC_TYPE_LWORD*)storeValue) = *readValue;
			}
			break;
		}
		case AtomicVarType::INT:
		{
			AtomicVar<PLC_TYPE_INT>* readValue = static_cast<AtomicVar<PLC_TYPE_INT>*>(g_readPersistance[tag]);
			if (readValue)
			{
				*((PLC_TYPE_INT*)storeValue) = *readValue;
			}
			break;
		}
		case AtomicVarType::DINT:
		{
			AtomicVar<PLC_TYPE_DINT>* readValue = static_cast<AtomicVar<PLC_TYPE_DINT>*>(g_readPersistance[tag]);
			if (readValue)
			{
				*((PLC_TYPE_DINT*)storeValue) = *readValue;
			}
			break;
		}
		case AtomicVarType::LINT:
		{
			AtomicVar<PLC_TYPE_LINT>* readValue = static_cast<AtomicVar<PLC_TYPE_LINT>*>(g_readPersistance[tag]);
			if (readValue)
			{
				*((PLC_TYPE_LINT*)storeValue) = *readValue;
			}
			break;
		}
		case AtomicVarType::REAL:
		{
			AtomicVar<PLC_TYPE_REAL>* readValue = static_cast<AtomicVar<PLC_TYPE_REAL>*>(g_readPersistance[tag]);
			if (readValue)
			{
				*((PLC_TYPE_REAL*)storeValue) = *readValue;
			}
			break;
		}
		case AtomicVarType::LREAL:
		{
			AtomicVar<PLC_TYPE_LREAL>* readValue = static_cast<AtomicVar<PLC_TYPE_LREAL>*>(g_readPersistance[tag]);
			if (readValue)
			{
				*((PLC_TYPE_LREAL*)storeValue) = *readValue;
			}
			break;
		}
		case AtomicVarType::STRING:
		{
			AtomicVar<PLC_TYPE_STRING>* readValue = static_cast<AtomicVar<PLC_TYPE_STRING>*>(g_readPersistance[tag]);
			if (readValue)
			{
				strcpy((char*)storeValue,readValue->GetValue());
			}
			break;
		}
		default:
			break;
		}
	}
}

void PLCInitOpcInfo(PLCParam_ProtocalOpc* plcInfo, AtomicVarType type, const std::string& tag, int ns, char* identifier)
{
	switch (type)
	{
		break;
	case AtomicVarType::BOOL:
	{
		plcInfo->dataType = AtomicVarType::BOOL;
		plcInfo->bindVar = new AtomicVar<PLC_TYPE_BOOL>(0);
		plcInfo->ns = ns;
		plcInfo->protocol = PLCProtocol::OPCUA;
		strcpy_s(plcInfo->identifier, identifier);
		g_PLCVariables[tag] = plcInfo;
		g_readPersistance[tag] = plcInfo->bindVar;
		g_writePersistence[tag] = new AtomicVar<PLC_TYPE_BOOL>(0);
		break;
	}
	case AtomicVarType::WORD:
	{
		plcInfo->dataType = AtomicVarType::WORD;
		plcInfo->bindVar = new AtomicVar<PLC_TYPE_WORD>(0);
		plcInfo->ns = ns;
		plcInfo->protocol = PLCProtocol::OPCUA;
		strcpy_s(plcInfo->identifier, identifier);
		g_PLCVariables[tag] = plcInfo;
		g_readPersistance[tag] = plcInfo->bindVar;
		g_writePersistence[tag] = new AtomicVar<PLC_TYPE_WORD>(0);
		break;
	}
	case AtomicVarType::DWORD:
	{
		plcInfo->dataType = AtomicVarType::DWORD;
		plcInfo->bindVar = new AtomicVar<PLC_TYPE_DWORD>(0);
		plcInfo->ns = ns;
		plcInfo->protocol = PLCProtocol::OPCUA;
		strcpy_s(plcInfo->identifier, identifier);
		g_PLCVariables[tag] = plcInfo;
		g_readPersistance[tag] = plcInfo->bindVar;
		g_writePersistence[tag] = new AtomicVar<PLC_TYPE_DWORD>(0);
		break;
	}
	case AtomicVarType::LWORD:
	{
		plcInfo->dataType = AtomicVarType::LWORD;
		plcInfo->bindVar = new AtomicVar<PLC_TYPE_LWORD>(0);
		plcInfo->ns = ns;
		plcInfo->protocol = PLCProtocol::OPCUA;
		strcpy_s(plcInfo->identifier, identifier);
		g_PLCVariables[tag] = plcInfo;
		g_readPersistance[tag] = plcInfo->bindVar;
		g_writePersistence[tag] = new AtomicVar<PLC_TYPE_LWORD>(0);
		break;
	}
	case AtomicVarType::INT:
	{
		plcInfo->dataType = AtomicVarType::INT;
		plcInfo->bindVar = new AtomicVar<PLC_TYPE_INT>(0);
		plcInfo->ns = ns;
		plcInfo->protocol = PLCProtocol::OPCUA;
		strcpy_s(plcInfo->identifier, identifier);
		g_PLCVariables[tag] = plcInfo;
		g_readPersistance[tag] = plcInfo->bindVar;
		g_writePersistence[tag] = new AtomicVar<PLC_TYPE_INT>(0);
		break;
	}
	case AtomicVarType::DINT:
	{
		plcInfo->dataType = AtomicVarType::DINT;
		plcInfo->bindVar = new AtomicVar<PLC_TYPE_DINT>(0);
		plcInfo->ns = ns;
		plcInfo->protocol = PLCProtocol::OPCUA;
		strcpy_s(plcInfo->identifier, identifier);
		g_PLCVariables[tag] = plcInfo;
		g_readPersistance[tag] = plcInfo->bindVar;
		g_writePersistence[tag] = new AtomicVar<PLC_TYPE_DINT>(0);
		break;
	}
	case AtomicVarType::LINT:
	{
		plcInfo->dataType = AtomicVarType::LINT;
		plcInfo->bindVar = new AtomicVar<PLC_TYPE_LINT>(0);
		plcInfo->ns = ns;
		plcInfo->protocol = PLCProtocol::OPCUA;
		strcpy_s(plcInfo->identifier, identifier);
		g_PLCVariables[tag] = plcInfo;
		g_readPersistance[tag] = plcInfo->bindVar;
		g_writePersistence[tag] = new AtomicVar<PLC_TYPE_LINT>(0);
		break;
	}
	case AtomicVarType::REAL:
	{
		plcInfo->dataType = AtomicVarType::REAL;
		plcInfo->bindVar = new AtomicVar<PLC_TYPE_REAL>(0);
		plcInfo->ns = ns;
		plcInfo->protocol = PLCProtocol::OPCUA;
		strcpy_s(plcInfo->identifier, identifier);
		g_PLCVariables[tag] = plcInfo;
		g_readPersistance[tag] = plcInfo->bindVar;
		g_writePersistence[tag] = new AtomicVar<PLC_TYPE_REAL>(0);
		break;
	}
	case AtomicVarType::LREAL:
	{
		plcInfo->dataType = AtomicVarType::LREAL;
		plcInfo->bindVar = new AtomicVar<PLC_TYPE_LREAL>(0);
		plcInfo->ns = ns;
		plcInfo->protocol = PLCProtocol::OPCUA;
		strcpy_s(plcInfo->identifier, identifier);
		g_PLCVariables[tag] = plcInfo;
		g_readPersistance[tag] = plcInfo->bindVar;
		g_writePersistence[tag] = new AtomicVar<PLC_TYPE_LREAL>(0);
		break;
	}
	case AtomicVarType::STRING:
	{
		plcInfo->dataType = AtomicVarType::STRING;
		plcInfo->bindVar = new AtomicVar<PLC_TYPE_STRING>("");
		plcInfo->ns = ns;
		plcInfo->protocol = PLCProtocol::OPCUA;
		strcpy_s(plcInfo->identifier, identifier);
		g_PLCVariables[tag] = plcInfo;
		g_readPersistance[tag] = plcInfo->bindVar;
		g_writePersistence[tag] = new AtomicVar<PLC_TYPE_STRING>("");
		break;
	}
	case AtomicVarType::STRUCT:
	{
		plcInfo->dataType = AtomicVarType::STRUCT;
		strcpy_s(plcInfo->identifier, identifier);
		plcInfo->ns = ns;
		plcInfo->protocol = PLCProtocol::OPCUA;
		if (strstr(plcInfo->identifier, "astCNCQueueA") != NULL)
		{
			g_simRecBufferA.reserve(200);
			for (int i = 0; i < 200; i++)
			{
				g_simRecBufferA.push_back(CNCSimulateRecord());
			}
			plcInfo->bindVar = g_simRecBufferA.data();
			g_PLCVariables[tag] = plcInfo;
			g_readPersistance[tag] = plcInfo->bindVar;
		}
		else if (strstr(plcInfo->identifier, "astCNCQueueB") != NULL)
		{
			g_simRecBufferB.reserve(200);
			for (int i = 0; i < 200; i++)
			{
				g_simRecBufferB.push_back(CNCSimulateRecord());
			}
			plcInfo->bindVar = g_simRecBufferB.data();
			g_PLCVariables[tag] = plcInfo;
			g_readPersistance[tag] = plcInfo->bindVar;
		}
		break;
	}
	case AtomicVarType::ARRAY_BOOL:
	{
		plcInfo->dataType = AtomicVarType::ARRAY_BOOL;
		strcpy_s(plcInfo->identifier, identifier);
		plcInfo->ns = ns;
		plcInfo->protocol = PLCProtocol::OPCUA;
		if (strstr(plcInfo->identifier, "xPCChangeDone") != NULL)
		{
			for (int i = 0; i < 10;i++)
			{
				g_stationPCChangeDone[i] = false;
			}
			plcInfo->bindVar = g_stationPCChangeDone.data();
			g_PLCVariables[tag] = plcInfo;
			g_readPersistance[tag] = plcInfo->bindVar;
			g_writePersistence[tag] = new std::array<bool, 10>();
		}
		else if (strstr(plcInfo->identifier,"xPCChange") != NULL)
		{
			for (int i = 0; i < 10;i++)
			{
				g_stationPCChange[i] = false;
			}
			plcInfo->bindVar = g_stationPCChangeDone.data();
			g_PLCVariables[tag] = plcInfo;
			g_readPersistance[tag] = plcInfo->bindVar;
		}
		else if(strstr(plcInfo->identifier,"xPCFileFTPDone") != NULL)
		{
			for (int i = 0; i < 10;i++)
			{
				g_stationPCFileFTPDone[i] = false;
			}
			plcInfo->bindVar = g_stationPCFileFTPDone.data();
			g_PLCVariables[tag] = plcInfo;
			g_readPersistance[tag] = plcInfo->bindVar;
			g_writePersistence[tag] = new std::array<bool,10>();
		}
		else if (strstr(plcInfo->identifier, "xPCFileFTP") != NULL)
		{
			for (int i = 0; i < 10;i++)
			{
				g_stationPCFileFTP[i] = false;
			}
			plcInfo->bindVar = g_stationPCFileFTP.data();
			g_PLCVariables[tag] = plcInfo;
			g_readPersistance[tag] = plcInfo->bindVar;
		}
		break;
	}
	case AtomicVarType::None:
	default:
		break;
	}
}

void UploadFileToFTP(const std::string& f, const std::string& content)
{
	QString title = QString::fromLocal8Bit(f.c_str());
	QStringList parts = title.split("/");
	QString fileName = parts.last();
	QString trimmedFileName = fileName.replace(" ", "");
	QFileInfo fileInfo(trimmedFileName);
	QString suffix = fileInfo.suffix(); // 结果："txt"
	fileName = fileInfo.baseName() + ".cnc";
	fileName = "Share_files_anonymity/" + fileName.trimmed();

	FtpClient::UploadFile(QString::fromStdString(content), fileName);
}
