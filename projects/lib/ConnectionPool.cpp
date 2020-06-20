//#include "ConnectionPool.h"


#include "ConnectionPool.h"
#include <QDebug>
//#include "CGobal.h"

QMutex ConnectionPool::mutex;
QWaitCondition ConnectionPool::waitConnection;
ConnectionPool* ConnectionPool::instance = NULL;

ConnectionPool::ConnectionPool() {
	// �������ݿ����ӵ���Щ��Ϣ��ʵ�ʿ�����ʱ����Ҫͨ����ȡ�����ļ��õ���
	// ����Ϊ����ʾ��������д�����˴����
	//hostName = "127.0.0.1";
	//databaseName = "qt";
	//databaseName = QCoreApplication::applicationDirPath() + "/" + GOB::SET->value("/sqldb/gmailDbName").toString();
	//username = "root";
	//password = "root";


	//databaseName = dName;

	//databaseType = "QMYSQL";
	// QCoreApplication::applicationDirPath() + "/" + gmailDbName
	databaseType = "QSQLITE"; //  GOB::SET->value("/sqldb/gmailDbType").toString();
	testOnBorrow = false;
	testOnBorrowSql = "SELECT 1";

	maxWaitTime = 1000; // GOB::SET->value("/sqldb/gmailDbMaxWaitTime").toInt();
	waitInterval = 200; // GOB::SET->value("/sqldb/gmailDbMaxwaitInterval").toInt();
	maxConnectionCount = 10; // GOB::SET->value("/sqldb/gmailDbMaxConnect").toInt();
}

ConnectionPool::~ConnectionPool() {
	// �������ӳص�ʱ��ɾ�����е�����
	foreach(QString connectionName, usedConnectionNames) {
		QSqlDatabase::removeDatabase(connectionName);
	}

	foreach(QString connectionName, unusedConnectionNames) {
		QSqlDatabase::removeDatabase(connectionName);
	}
}

ConnectionPool& ConnectionPool::getInstance() {
	if (NULL == instance) {
		QMutexLocker locker(&mutex);

		if (NULL == instance) {
			instance = new ConnectionPool();
		}
	}

	return *instance;
}

void ConnectionPool::release() {
	QMutexLocker locker(&mutex);
	delete instance;
	instance = NULL;
}

QSqlDatabase ConnectionPool::openConnection(QString dbName) {
	ConnectionPool& pool = ConnectionPool::getInstance();
	QString connectionName;

	if (dbName != nullptr) {
		pool.databaseName = dbName;
	}

	QMutexLocker locker(&mutex);

	// �Ѵ���������
	int connectionCount = pool.unusedConnectionNames.size() + pool.usedConnectionNames.size();

	// ��������Ѿ����꣬�ȴ� waitInterval ���뿴���Ƿ��п������ӣ���ȴ� maxWaitTime ����
	for (int i = 0;
		i < pool.maxWaitTime
		&& pool.unusedConnectionNames.size() == 0 && connectionCount == pool.maxConnectionCount;
		i += pool.waitInterval) {
		waitConnection.wait(&mutex, pool.waitInterval);

		// ���¼����Ѵ���������
		connectionCount = pool.unusedConnectionNames.size() + pool.usedConnectionNames.size();
	}

	if (pool.unusedConnectionNames.size() > 0) {
		// ���Ѿ����յ����ӣ���������
		connectionName = pool.unusedConnectionNames.dequeue();
	}
	else if (connectionCount < pool.maxConnectionCount) {
		// û���Ѿ����յ����ӣ�����û�дﵽ������������򴴽��µ�����
		connectionName = QString("Connection-%1").arg(connectionCount + 1);
	}
	else {
		// �Ѿ��ﵽ���������
		qDebug() << "Cannot create more connections.";
		return QSqlDatabase();
	}

	// ��������
	QSqlDatabase db = pool.createConnection(connectionName);

	// ��Ч�����Ӳŷ��� usedConnectionNames
	if (db.isOpen()) {
		pool.usedConnectionNames.enqueue(connectionName);
	}

	return db;
}

void ConnectionPool::closeConnection(QSqlDatabase connection) {
	ConnectionPool& pool = ConnectionPool::getInstance();
	QString connectionName = connection.connectionName();

	// ��������Ǵ��������ӣ��� used ��ɾ�������� unused ��
	if (pool.usedConnectionNames.contains(connectionName)) {
		QMutexLocker locker(&mutex);
		pool.usedConnectionNames.removeOne(connectionName);
		pool.unusedConnectionNames.enqueue(connectionName);
		waitConnection.wakeOne();
	}
}

QSqlDatabase ConnectionPool::createConnection(const QString& connectionName) {
	// �����Ѿ��������ˣ������������������´���
	if (QSqlDatabase::contains(connectionName)) {
		QSqlDatabase db1 = QSqlDatabase::database(connectionName);

		if (testOnBorrow) {
			// ��������ǰ�������ݿ⣬������ӶϿ������½�������
			qDebug() << "Test connection on borrow, execute:" << testOnBorrowSql << ", for" << connectionName;
			QSqlQuery query(testOnBorrowSql, db1);

			if (query.lastError().type() != QSqlError::NoError && !db1.open()) {
				qDebug() << "Open datatabase error:" << db1.lastError().text();
				return QSqlDatabase();
			}
		}

		return db1;
	}

	// ����һ���µ�����
	QSqlDatabase db = QSqlDatabase::addDatabase(databaseType, connectionName);
	//db.setHostName(hostName);
	db.setDatabaseName(databaseName);
	//db.setUserName(username);
	//db.setPassword(password);

	if (!db.open()) {
		qDebug() << "Open datatabase error:" << db.lastError().text();
		return QSqlDatabase();
	}
	else {

	}

	return db;
}