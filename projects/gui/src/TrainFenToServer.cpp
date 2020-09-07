
#include "TrainFenToServer.h"

#include "mainwindow.h"
#include "moveevaluation.h"

#include <QSettings>

namespace Chess {

	CTrainFen::~CTrainFen()
	{
	}

	void CTrainFen::on_start2(CTrainFenMethod method, ChessGame* game)
	{
		_method = method;

		GetSetting();

		if (game != nullptr) {
			//ChessGame cgame = *game;
			//_game = &cgame;
			_startFen = game->startingFen();
			_fens = game->fens();                 // 所有Fen列表
			_scores = game->scores();
		}		
		this->start();
	}

	//void CTrainFen::RunAddFen()
	//{
	//	//emit SendSignal(1, "dddd");
	//}

	int CTrainFen::getWebInfoByQuery(QUrl url, QString& res)
	{
		QNetworkRequest req;
		req.setUrl(url);
		
		QTimer timer;
		timer.setInterval(2000);  // 设置超时时间 1000 ms
		timer.setSingleShot(true);  // 单次触发

		QNetworkAccessManager* manager = new QNetworkAccessManager();
		// 发送请求
		QNetworkReply* pReplay = manager->get(req);

		// 开启一个局部的事件循环，等待响应结束，退出
		QEventLoop eventLoop;
		connect(&timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
		QObject::connect(manager, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit);
		timer.start();
		eventLoop.exec();

		int nStatusCode = 400;
		if (timer.isActive()) {  // 处理响应
			timer.stop();
			if (pReplay->error() != QNetworkReply::NoError) {
				// 错误处理
				qDebug() << "Error String : " << pReplay->errorString();
			}
			else {
				QVariant variant = pReplay->attribute(QNetworkRequest::HttpStatusCodeAttribute);
				nStatusCode = variant.toInt();
				// 根据状态码做进一步数据处理
				//QByteArray bytes = pReply->readAll();
				//qDebug() << "Status Code : " << nStatusCode;
				if (nStatusCode == 200) {
					res = pReplay->readAll();
				}				
			}
		}
		else {  // 处理超时
			disconnect(pReplay, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
			pReplay->abort();
			pReplay->deleteLater();
			qDebug() << "Timeout";
		}
		return nStatusCode;
	}

	bool CTrainFen::FenAddLostGame()
	{

		//this->addOneFenToWEB("2bakN3/4aR3/4b4/1C1R5/6n2/9/C6c1/2n1B3c/4A3r/4KA3 w - - 0 1", 0, true);		

		// 得到当前的所有棋步，和得分，分别上传

		//ChessGame* game = this->pMain->GetCurrentChessGame();  // _game 

		//ChessGame* game = _game;
		//QString startFen = game->startingFen();
		//QString startFen = game->board()->startingFenString();
		int numUp = 0;
		bool bHaveFirst = false;
		if (_startFen != "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1") {
			this->addOneFenToWEB(_startFen, 0, true);
			numUp++;
			bHaveFirst = true;
		}

		//******************* 准备测试数据
		//for (int i = 0; i < 100; i++) {

		//}


		// 先找出负分，向前推10个局面就行
		int startPly = 400;
		bool findStart = false;
		int endPly = 0;
		int score = 0;
		for (int ply = 0; ply < _fens.count(); ply++) {

			//QString fen = _fens[ply];
			if (_scores.contains(ply)) {
				score = _scores[ply];
				if (score == MoveEvaluation::NULL_SCORE) {
					score = -1;
				}
			}
			else {
				score = -1;       // 只上传算过的分数
			}
			if (!findStart) {
				if (score < (-_minScore)) {
					startPly = ply;
					findStart = true;
				}				
			}

			if (score < (-_maxScore) ) {  // we find end move
				endPly = ply;
				break;
			}
		}
		if (endPly > _maxSteps) {
			endPly = _maxSteps;
		}

		if (startPly >= endPly) {
			startPly = endPly - 1;
		}
		startPly -= _minSteps;
		if (startPly <= 0) {
			startPly = 0;
		}
		

		
		for(int ply= startPly; ply < endPly; ply++){

			QString fen = _fens[ply];
			if (_scores.contains(ply)) {
				score = _scores[ply];
				if (score == MoveEvaluation::NULL_SCORE) {
					score = -1;
				}
			}
			else {
				score = -1;   // 只上传算过的分数
			}	
			
			ply += _stepsGap;
						
			this->addOneFenToWEB(fen, score, !bHaveFirst);
			bHaveFirst = true;
			numUp++;
		}


		QString info = "本次共上传 " + QString::number(numUp) + " 个局面";
		emit SendSignal(3,info);
		return true;
	}

	// 高分和棋
	bool CTrainFen::FenAddDrawTooHigh()
	{
		// 

	
		bool bHaveFirst = false;
		int numUp = 0;  // 上传的局面数
	
		int HIGH_SCORE = _drawHighScore;
		int HIGH_TIMES = _drawHignScoreNum;
		// 先找出高分

		//int hign_num = 0;
		bool find = false;
		int endPly = _fens.count() - 1;
		int startPly = endPly - HIGH_TIMES;
		if (startPly < 0) {
			startPly = 0;
		}
	
		int score = 0;
		int totalScore = 0;


		for (int ply = startPly; ply < endPly; ply++) {
			//QString fen = _fens[ply];
			if (_scores.contains(ply)) {
				score = _scores[ply];
				if (score == MoveEvaluation::NULL_SCORE) {
					score =  -1;
				}
			}
			else {
				score = -1;       // 只上传算过的分数
			}
			totalScore += abs(score);   // 这儿得算是绝对分数
		}

		if (totalScore / HIGH_TIMES > HIGH_SCORE) {
			find = true;
		}

		if (find == false) {

			emit SendSignal(3, "本对局不是高分和棋！");
			return false;
		}


		for (int ply = startPly; ply < endPly; ply++) {

			QString fen = _fens[ply];
			if (_scores.contains(ply)) {
				score = _scores[ply];
				if (score == MoveEvaluation::NULL_SCORE) {
					score = -1;
				}
			}
			else {
				score = -1;   // 
			}

			//ply += _stepsGap; 这个不用

			this->addOneFenToWEB(fen, score, !bHaveFirst);
			bHaveFirst = true;
			numUp++;
		}


		QString info = "本次共上传 " + QString::number(numUp) + " 个局面";
		emit SendSignal(3, info);
		return true;
	}

	bool CTrainFen::FenDelete()
	{
		QString fen;
		int num = 0;
		int total = 500;
		while (GetAndDelOneFenFromSetting(fen) && total >= 0) {
			num++;
			delOneFenFromWEB(fen);
			msleep(100);
			total--;
		}
		emit SendSignal(1, "共从网站删除 " + QString::number(num) + " 个FEN");		
		return true;
	}

	// 增加一些常用的FEN
	bool CTrainFen::FenCommon()
	{		

		QStringList fenList;

		fenList.append("rnbakabnr/9/1c5c1/p1p1p1p1p/9/6P2/P1P1P3P/1C5C1/9/RNBAKABNR b - - 0 1");
		fenList.append("rnbakabnr/9/1c5c1/p1p1p1p1p/9/P8/2P1P1P1P/1C5C1/9/RNBAKABNR b - - 0 1");
		fenList.append("rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1CN4C1/9/R1BAKABNR b - - 0 1");
		fenList.append("rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C2B2C1/9/RN1AKABNR b - - 0 1");
		fenList.append("rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C4C2/9/RNBAKABNR b - - 0 1");
		fenList.append("rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C3C3/9/RNBAKABNR b - - 0 1");
		fenList.append("rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5CN/9/RNBAKAB1R b - - 0 1");
		fenList.append("rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/4A4/RNBAK1BNR b - - 0 1");
		fenList.append("rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/5C1C1/9/RNBAKABNR b - - 0 1");
		fenList.append("4kab2/4a1c2/2n1b4/p1p1nRP1p/4p4/2P3P2/Pcr4rP/R1N1CAN2/6C2/2BAK1B2 w - - 0 1");  // 急进中兵对攻均势局面
		fenList.append("rnbak1C2/2r1a4/1c4nc1/p3p3p/6p2/2p6/P3P1P1P/1CN3N2/8R/R1BAKAB2 w - - 0 1");     // 对攻
		//fenList.append("");
		//fenList.append(""); rnbak1C2/2r1a4/1c4nc1/p3p3p/6p2/2p6/P3P1P1P/1CN3N2/8R/R1BAKAB2 w - - 0 7
		int num = 0;
		for (auto fen : fenList) {
			this->addOneFenToWEB(fen);
			this->msleep(10);
			num++;
		}
		emit emit SendSignal(3, "本次上传 " + QString::number(num) + " 个局面");
		return true;
	}

	bool CTrainFen::FenRemoveAll()
	{
		QUrl url(this->_WebAddress);

		QUrlQuery query; // key-value 对
		query.addQueryItem("user", _userName);
		query.addQueryItem("pass", _passWord);
		query.addQueryItem("method", "clear");
		query.addQueryItem("trainID", QString::number(_trainID));		
		query.addQueryItem("first", "no");
		
		query.addQueryItem("fen", "no_tfen");
		query.addQueryItem("side", "white");
		url.setQuery(query);

		QString res;
		int stat = this->getWebInfoByQuery(url, res);

		
		// 发送信息到status上
		if (stat == 200) {
			emit SendSignal(1, "清除所有FEN 成功 ");
		}
		else {
			emit SendSignal(1, "清除所有FEN 失败 ");
		}
		emit SendSignal(2, "");

		/// <summary>
		///  清空本地 Fen
		/// </summary>
		/// <returns></returns>
		QSettings s;
		QStringList fenlist;

		// 再写入数组
		s.beginWriteArray("UploadFenArray");
		s.endArray();

		if (stat != 200) return false;
		return true;
	}

	bool CTrainFen::GetTfenTsideFromFEN(QString fen, QString& tfen, QString& tside) {
		
		// 分隔一下FEN 
		QStringList list = fen.split(" ");
		if (list.count() != 6) {			
			emit SendSignal(1, "出错了，FEN 格式不准确, 不是6空格");			
			emit SendSignal(2, fen);
			return false;
		}

		tfen = list[0];
		if (list[1] == "w") {
			tside = "white";
		}
		else if (list[1] == "b") {
			tside = "black";
		}
		else {
			emit SendSignal(1, "出错了，FEN 格式不准确，side");
			emit SendSignal(2, fen);
			return false;
		}
		return true;
	}

	bool CTrainFen::addOneFenToWEB(QString fen, int score, bool first)
	{		
		QString tfen;
		QString tside;
		if (GetTfenTsideFromFEN(fen, tfen, tside) == false) {
			return false;
		}
		
		QUrl url(this->_WebAddress);

		QUrlQuery query; // key-value 对
		query.addQueryItem("user", _userName);
		query.addQueryItem("pass", _passWord);
		query.addQueryItem("method", "add");
		query.addQueryItem("trainID", QString::number(_trainID));
		if (first) {
			query.addQueryItem("first", "yes");
		}
		else {
			query.addQueryItem("first", "no");
		}
		query.addQueryItem("fen", tfen);
		query.addQueryItem("side", tside);
		url.setQuery(query);

		QString res;
		int stat = this->getWebInfoByQuery(url, res);		

		QString info = "评分: " + QString::number(score); //  +" FEN: " + fen;
		// 发送信息到status上
		if (stat == 200) {			
			emit SendSignal(1, "上传成功 " + info);
		}	
		else {
			emit SendSignal(1, "上传失败 " + info);
		}
		emit SendSignal(2, res);
		//emit SendSignal(3, info);
		
		if (stat != 200) return false;

		// 保存到fen数组中，以便后面可删除
		AddOneFenToSetting(fen);

		return true;
	}

	bool CTrainFen::delOneFenFromWEB(QString fen)
	{
		QString tfen;
		QString tside;
		if (GetTfenTsideFromFEN(fen, tfen, tside) == false) {
			return false;
		}

		QUrl url(this->_WebAddress);

		QUrlQuery query; // key-value 对
		query.addQueryItem("user", _userName);
		query.addQueryItem("pass", _passWord);
		query.addQueryItem("method", "del");
		query.addQueryItem("trainID", QString::number(_trainID));
		query.addQueryItem("first", "no");
		//if (first) {
		//	query.addQueryItem("first", "yes");
		//}
		//else {
		//	query.addQueryItem("first", "no");
		//}
		query.addQueryItem("fen", tfen);
		query.addQueryItem("side", tside);
		url.setQuery(query);

		QString res;
		int stat = this->getWebInfoByQuery(url, res);
	
		// 发送信息到status上
		if (stat == 200) {
			emit SendSignal(1, "删除成功 " + fen);
		}
		else {
			emit SendSignal(1, "删除失败 " + fen);
		}
		emit SendSignal(2, res);

		if (stat != 200) return false;	

		return true;		
	}

	bool CTrainFen::AddOneFenToSetting(QString fen)
	{
		QSettings s;
		QStringList fenlist;

		fenlist.append(fen);

		//读取数组
		int size = s.beginReadArray("UploadFenArray");             
		for (int i = 0; i < size; i++) {
			s.setArrayIndex(i);
			fenlist.append(s.value("FEN").toString());			
		}
		s.endArray();

		if (_saveBlind) {
			// 再写入数组
			s.beginWriteArray("UploadFenArray");
			for (int i = 0; i < fenlist.size(); i++) {
				s.setArrayIndex(i);
				s.setValue("FEN", fenlist[i]);
			}
			s.endArray();
		}

		//emit SendSignal(1, "成功");
		//QString info = "本机已上传 " + QString::number(fenlist.size()) + " 个局面";
		//emit SendSignal(3,info);

		return true;
	}

	bool CTrainFen::GetAndDelOneFenFromSetting(QString& fen)
	{
		QSettings s;
		QStringList fenlist;


		bool find = false;

		//读取数组
		int size = s.beginReadArray("UploadFenArray");
		for (int i = 0; i < size; i++) {
			s.setArrayIndex(i);
			fenlist.append(s.value("FEN").toString());
		}
		s.endArray();

		if (fenlist.size() >= 1) {
			fen = fenlist.first();
			fenlist.removeFirst();
			find = true;
		}

		// 再写入数组
		s.beginWriteArray("UploadFenArray");
		for (int i = 0; i < fenlist.size(); i++) {
			s.setArrayIndex(i);
			s.setValue("FEN", fenlist[i]);
		}
		s.endArray();	

		//int newsize = s.beginReadArray("UploadFenArray");
		//s.endArray();

		return find;
	}

	bool CTrainFen::GetSetting()
	{
		// 得到设置信息
		QSettings s;
		_userName = s.value("trainFen/UserName", "user").toString();
		_passWord = s.value("trainFen/Password", "password").toString();
		_WebAddress = "http://" + s.value("trainFen/WebAddress", "cdn.ggzero.cn").toString() + "/train_fen";
		_trainID = s.value("trainFen/trainID", 1).toInt();
		_maxScore = s.value("trainFen/MaxScore", 1000).toInt();
		_minScore = s.value("trainFen/MinScore", 0).toInt();
		_maxSteps = s.value("trainFen/MaxSteps", 400).toInt();
		_minSteps = s.value("trainFen/MinSteps", 0).toInt();
		_stepsGap = s.value("trainFen/StepsGap", 0).toInt();
		_saveBlind = s.value("trainFen/BlinDSave", false).toBool();


		_drawHighScore = s.value("trainFen/DrawHighScore", 200).toInt();
		_drawHignScoreNum = s.value("trainFen/DrawHighScoreNum", 30).toInt();

		return true;
	}


	CTrainFen::CTrainFen(QObject* parent)	:QThread(parent),
		pMain((MainWindow*)(parent))
	{
		GetSetting();

		connect(this, SIGNAL(SendSignal(int, QString)),
			parent, SLOT(slotDisplayStatus(int, QString)));
		
	}

	void CTrainFen::run() {

		//while (true)
		//{
			switch (_method) {
			case CTrainFenMethod::ADD_FEN:
				FenAddLostGame();
				break;
			case CTrainFenMethod::ADD_Draw_Too_High:
				FenAddDrawTooHigh();
				break;
			case CTrainFenMethod::DEL_FEN:
				FenDelete();
				break;
			case CTrainFenMethod::COMMON_FEN:
				FenCommon();
				break;
			case CTrainFenMethod::REMOVE_FEN:
				FenRemoveAll();
				break;
			default:
				break;
			}
			
			//this->msleep(1000);

			//emit SendSignal(1, "dddd");
			//QString s = "addd";
			//emit SendSignal(1, s);

		//}
	}
}



// emit SendSignal(1, "dddd");