#pragma once

#include <QOBject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QFileInfo>
#include "Common/ProgressInfo.h"

#define DB_PATH "./db/runtime.db"

class DataBaseCNC : public QObject
{
	Q_OBJECT

public:
	static DataBaseCNC* GetInstance();
	void AddOpenDraftRecord(int stationId,const QString& filePath);

private:
	DataBaseCNC();
	~DataBaseCNC();

private:
	QSqlDatabase m_db;
	static DataBaseCNC* s_instance;
};