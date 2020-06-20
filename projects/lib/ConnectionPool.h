#pragma once
#include <QtSql>
#include <QQueue>
#include <QString>
#include <QMutex>
#include <QMutexLocker>

class ConnectionPool {
public:
	static void release(); // �ر����е����ݿ�����
	static QSqlDatabase openConnection(QString dbName = nullptr);                 // ��ȡ���ݿ�����
	static void closeConnection(QSqlDatabase connection); // �ͷ����ݿ����ӻ����ӳ�

	~ConnectionPool();

	

private:
	static ConnectionPool& getInstance();

	ConnectionPool();
	ConnectionPool(const ConnectionPool& other);
	ConnectionPool& operator=(const ConnectionPool& other);
	QSqlDatabase createConnection(const QString& connectionName); // �������ݿ�����

	QQueue<QString> usedConnectionNames;   // ��ʹ�õ����ݿ�������
	QQueue<QString> unusedConnectionNames; // δʹ�õ����ݿ�������

	// ���ݿ���Ϣ
	//QString hostName;
	
	//QString username;
	//QString password;
	QString databaseType;
	QString databaseName;

	bool    testOnBorrow;    // ȡ�����ӵ�ʱ����֤�����Ƿ���Ч
	QString testOnBorrowSql; // ���Է������ݿ�� SQL

	int maxWaitTime;  // ��ȡ�������ȴ�ʱ��
	int waitInterval; // ���Ի�ȡ����ʱ�ȴ����ʱ��
	int maxConnectionCount; // ���������

	static QMutex mutex;
	static QWaitCondition waitConnection;
	static ConnectionPool* instance;

};

