#include "IO/DataBase.h"
#include <iostream>

DataBaseCNC* DataBaseCNC::s_instance = nullptr;

DataBaseCNC::DataBaseCNC()
{
	if (QSqlDatabase::contains("DraftConn"))
	{
		m_db = QSqlDatabase::database("DraftConn");
	}
	else
	{
		m_db = QSqlDatabase::addDatabase("QSQLITE", "DrafConn");
	}

	m_db.setDatabaseName("db/runtime.db");

	if (!m_db.open()) {
		qCritical() << "打开数据库失败" << m_db.lastError().text();
	}

	QSqlQuery query(m_db);
	QString createTableSql = R"(
		CREATE TABLE IF NOT EXISTS DraftRecord(
		  station_id INTEGER NOT NULL,
		  file_path TEXT NOT NULL,
		  open_time DATETIME NOT NULL DEFAULT current_timestamp,
		  UNIQUE(station_id,file_path)
		);
	)";

	if (!query.exec(createTableSql))
	{
		m_db.close();
	}
}

DataBaseCNC* DataBaseCNC::GetInstance()
{
	if (DataBaseCNC::s_instance == nullptr)
	{
		DataBaseCNC::s_instance = new DataBaseCNC();
	}

	return DataBaseCNC::s_instance;
}

void DataBaseCNC::AddOpenDraftRecord(int stationId, const QString& filePath)
{
	QSqlQuery query(m_db);
	QString sql = R"(
        INSERT OR REPLACE INTO DraftRecord 
        (station_id, file_path)
        VALUES (?, ?)
    )";
	query.prepare(sql);

	query.addBindValue(stationId);
	query.addBindValue(filePath);

	if (!query.exec())
	{
		g_file_logger->error("错误: 数据库插入图纸记录失败,插入参数: 工位号:{} 图纸来源:{}",stationId,std::string(filePath.toUtf8()));
	}
}

DataBaseCNC::~DataBaseCNC()
{

}
