#include "IO/DataBase.h"
#include "IO/Utils.h"
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
	createFileIfNotExists(DB_PATH);

	m_db.setDatabaseName(DB_PATH);

	if (!m_db.open()) {
		g_file_logger->critical("错误: 本地数据库连接失败");
	}

	QSqlQuery query(m_db);
	QString createTableSql = R"(
		CREATE TABLE IF NOT EXISTS DraftRecord(
		  station_id INTEGER NOT NULL,
		  file_path TEXT NOT NULL,
		  open_time DATETIME NOT NULL DEFAULT current_timestamp,
		  UNIQUE(station_id)
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

std::vector<std::tuple<int, QString>> DataBaseCNC::GetDraftOpenRecords()
{
	std::vector<std::tuple<int, QString>> res;

	QString extractDraftRecSql = "SELECT * FROM DraftRecord ORDER BY station_id ASC";
	QSqlQuery query(m_db);
	if (!query.exec(extractDraftRecSql))
	{
		g_file_logger->error("错误: 提取图纸数据失败! 执行sql语句:{}",extractDraftRecSql.toStdString());
	}

	while (query.next())
	{
		int stationId = query.value("station_id").toInt();
		QString filePath = query.value("file_path").toString();
		res.push_back({ stationId,filePath });
	}

	return res;
}

DataBaseCNC::~DataBaseCNC()
{

}
