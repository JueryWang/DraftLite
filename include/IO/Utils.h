#pragma once

#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QDir>
#include <QMessageBox>
static QString SplitFileFromPath(const QString& path)
{
	QFileInfo fileInfo(path);
	QString suffix = fileInfo.suffix();
	return fileInfo.baseName() + "." + suffix;
}

static QString FileNameFromPath(const QString& path)
{
	QFileInfo fileInfo(path);
	return fileInfo.baseName();
}

static bool MoveFileQt(const QString& srcPath, const QString& destDir) {
	QFileInfo srcFileInfo(srcPath);
	if (!srcFileInfo.exists() || !srcFileInfo.isFile()) {
		QMessageBox::warning(nullptr, QStringLiteral("错误"), QStringLiteral("源文件不存在或不是文件。"));
		return false;
	}

	QDir destDirectory(destDir);
	if (!destDirectory.exists())
	{
		if (!destDirectory.mkpath(destDir)) {
			QMessageBox::warning(nullptr, QStringLiteral("错误"), QStringLiteral("目标目录创建失败。"));
			return false;
		}
	}

	QString destPath = destDirectory.absoluteFilePath(srcFileInfo.fileName());

	QFile destFile(destPath);
	if (destFile.exists()) {
		if (!destFile.remove()) {
			QMessageBox::warning(nullptr, QStringLiteral("错误"), QStringLiteral("无法删除已存在的目标文件。"));
			return false;
		}
	}

	QFile srcFile(srcPath);
	if (srcFile.rename(destPath)) {
		return true;
	}
	else {
		QMessageBox::warning(nullptr, QStringLiteral("错误"), QStringLiteral("文件移动失败。"));
		return false;
	}
}

static bool copyFileToDir(const QString& srcFilePath, const QString& destDirPath) {
	// 1. 检查源文件是否存在
	QFile srcFile(srcFilePath);
	if (!srcFile.exists()) {
		qDebug() << "源文件不存在：" << srcFilePath;
		return false;
	}

	// 2. 确保目标目录存在（不存在则创建）
	QDir destDir(destDirPath);
	if (!destDir.exists()) {
		if (!destDir.mkpath(destDirPath)) {
			qDebug() << "目标目录创建失败：" << destDirPath;
			return false;
		}
	}

	// 3. 拼接目标文件的完整路径（保留原文件名）
	QString fileName = SplitFileFromPath(srcFilePath); // 获取源文件名
	QString destFilePath = destDir.filePath(fileName); // 目标目录 + 文件名

	// 4. 执行拷贝（默认不覆盖，若要覆盖需先删除目标文件）
	bool success = srcFile.copy(destFilePath);
	if (!success) {
		QMessageBox::warning(nullptr,"文件操作错误",QString("文件拷贝失败：%1").arg(srcFile.errorString()));
		return false;
	}

	qDebug() << "文件拷贝成功：" << srcFilePath << "→" << destFilePath;
	return true;
}
static bool CompressProject(const QString& sourceDir, const QString& targetProj)
{
	QString dirName = SplitFileFromPath(targetProj);
	QFileInfo sourceInfo(sourceDir);
	// 获取绝对路径，确保后续操作准确
	QString absoluteSource = sourceInfo.absoluteFilePath();
	// 获取父目录路径 (相当于执行了 cd ..)
	QString parentDir = sourceInfo.absolutePath();

	QProcess tar;
	QProcess pigz;

	tar.setWorkingDirectory(parentDir);
	tar.setProgram("tar");
	tar.setArguments({ "-cvf","-",dirName });

	pigz.setProgram("pigz");
	tar.setStandardOutputProcess(&pigz);
	pigz.setStandardOutputFile(targetProj);

	tar.start();
	pigz.start();

	if (!tar.waitForFinished())
	{
		QMessageBox::information(nullptr, QString("错误"), QString("无法打开指定文件,请检查文件是否在使用中"));
		return false;
	}
	else
	{
		int exitCode = tar.exitCode();
		QByteArray stderrData = tar.readAllStandardError();
		if (exitCode)
		{
			QMessageBox::warning(nullptr, QString("警告"), QString("解析工程文件执行出错:%1").arg(QString::fromLocal8Bit(stderrData)));
			return false;
		}
	}

	if (!pigz.waitForFinished())
	{
		QMessageBox::information(nullptr, QString("错误"), QString("无法打开指定文件,请检查文件是否在使用中"));
		return false;
	}
	else
	{
		int exitCode = pigz.exitCode();
		QByteArray stderrData = pigz.readAllStandardError();
		if (exitCode)
		{
			QMessageBox::warning(nullptr, QString("警告"), QString("解析工程文件执行出错:%1").arg(QString::fromLocal8Bit(stderrData)));
			return false;
		}
	}

	return true;
}

static bool DecompressProject(const QString& sourceProj, const QString& targetDir)
{
	QProcess tar;

	tar.setProgram("tar");
	tar.setArguments({ "-xf",sourceProj ,"-C", targetDir });

	tar.start();
	if (!tar.waitForFinished())
	{
		QMessageBox::warning(nullptr, QString("警告"), QString("工程文件解压失败,请检查是否被其他程序占用"));
		return false;
	}
	int exitCode = tar.exitCode();
	QByteArray stderrData = tar.readAllStandardError();
	if (exitCode)
	{
		QMessageBox::warning(nullptr, QString("警告"), QString("解析工程文件执行出错:%1").arg(QString::fromLocal8Bit(stderrData)));
		return false;
	}

	return true;
}