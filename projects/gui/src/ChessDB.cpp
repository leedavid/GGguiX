#include "ChessDB.h"

#include "linkboard.h"
#include "mainwindow.h"

namespace Chess {

	ChessDB::ChessDB(QObject* parent) :QThread(parent),
		pMain((MainWindow*)(parent)) {
		//this->pMain = parent;
		this->m_WebAddress = "http://www.chessdb.cn/chessdb.php";
		this->m_Endgame = false;
		m_findMove = false;
		m_bInputNewFen = false;
		m_bEndGameType = CHESSDB_ENDGAME_TYPE::DTM;
		m_bMustStop = false;

		connect(this, SIGNAL(SendSignalStatus(int, QString)),
			parent, SLOT(slotDisplayStatus(int, QString)));

		connect(this, SIGNAL(CapSendSignal(stCaptureMsg)),
			parent, SLOT(slotProcessCapMsg(stCaptureMsg)));


	}

	ChessDB::~ChessDB()
	{
	}

	// 主线程要求查询
	void ChessDB::FindChessDBmoveByFEN(const QString& fen) {
		this->m_bInputNewFen = true;
		this->m_newFen = fen;
	}

	int ChessDB::getWebInfoByQuery(QUrl url, QString& res)
	{
		QNetworkRequest req;
		req.setUrl(url);

		QNetworkAccessManager* manager = new QNetworkAccessManager();
		// 发送请求
		QNetworkReply* pReplay = manager->get(req);

		// 开启一个局部的事件循环，等待响应结束，退出
		QEventLoop eventLoop;
		QObject::connect(manager, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit);
		eventLoop.exec();

		// 获取响应信息
		res = pReplay->readAll();

		//获取http状态码

		 // QNetworkReply
	//  // attribute函数返回QVariant对象，使用value<T>()函数返回进行向下转型
		//qDebug() << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).value<int>();



		return pReplay->attribute(QNetworkRequest::HttpStatusCodeAttribute).value<int>();

		//QString s_date = bytes;
		//QStringList qlist = s_date.split('|');

		//return bytes;
	}

	int ChessDB::Query(const QString& tFen, QString& Res, CHESSDB_QUERY_TYPE qtype)
	{
		QUrl url(this->m_WebAddress);
		QUrlQuery query; // key-value 对

		// 转换一下fen
		QStringList fl = tFen.split(" ");
		if (fl.count() != 6) {
			emit SendSignalStatus(3, "Fen Error ChessDB");
			return 200+1;
		}
		//QString Fen = fl[0] + " " + fl[1];
		QString Fen = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w";
		qtype = CHESSDB_QUERY_TYPE::CHESSDB_QUERY_TYPE_ALL;

		switch (qtype)
		{
		case CHESSDB_QUERY_TYPE::CHESSDB_QUERY_TYPE_ALL:
			query.addQueryItem("action", "queryall");
			break;
		case CHESSDB_QUERY_TYPE::QUERY_TYPE_BEST:
			query.addQueryItem("action", "querybest");
			break;
		case CHESSDB_QUERY_TYPE::QUERY_TYPE_RANDOM:
			query.addQueryItem("action", "query");
			break;
		default:
			break;
		}

		query.addQueryItem("board", Fen);
		//query.addQueryItem("ban", m_BanMove);
		if (m_Endgame) {
			query.addQueryItem("endgame", "1");
			//
			if (m_bEndGameType == CHESSDB_ENDGAME_TYPE::DTM) {
				query.addQueryItem("egtbmetric", "dtm");
			}
			else {
				query.addQueryItem("egtbmetric", "dtc");
			}
		}

		url.setQuery(query);

		return getWebInfoByQuery(url, Res);

	}

	void ChessDB::QueryDB()
	{
		QString res;
		int r = this->Query(this->m_newFen, res);
		if (r == 200) {  // 可能有云库棋步

			QStringList MoveList = res.split("|");
			for(auto m : MoveList) {
				QStringList minfo = m.split(",");

				if (minfo.count() == 5) {
					ChessDBmove cm;

					// move
					QStringList cmlist = minfo[0].split(":");
					cm.move = cmlist[1];

					// score
					QStringList cmscore = minfo[1].split(":");
					cm.score = cmscore[1].toInt();

					break;  // 只发送一个棋步
				}				
				//int a = 0;
			}			
		}

	}

	void ChessDB::on_start()
	{
		this->start();
		emit SendSignalStatus(3, "云库线程已开启");
	}

	void ChessDB::run() {
		while (!m_bMustStop) {
			if (m_bInputNewFen) {
				m_bInputNewFen = false;
				QueryDB();                // 查询云库
			}
			this->msleep(100);   // 休息会
		}
	}

	ChessDBmove::ChessDBmove()
	{
		rank = 0;
		score = 0;
		winrate = 0.0f;
	}

}
