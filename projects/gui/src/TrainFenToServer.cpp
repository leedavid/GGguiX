
#include "TrainFenToServer.h"

#include "mainwindow.h"
#include "moveevaluation.h"

#include <QSettings>

namespace Chess {

	CTrainFen::~CTrainFen()
	{
	}
	int CTrainFen::ServerFENnum = CTrainFen::MaxFEN;

	CTrainFen::CTrainFen(QObject* parent) :QThread(parent),
		pMain((MainWindow*)(parent))
	{
		GetSetting();

		connect(this, SIGNAL(SendSignal(int, QString)),
			parent, SLOT(slotDisplayStatus(int, QString)));

		//this->_timerDeltime = 1000 * 60;  // 1����ɾ��һ��
		//this->_timerDeltime = 1000 ;  // 1����ɾ��һ��
		//_timerOn = false;
		this->_timer = nullptr;

	}

	void CTrainFen::on_start2(CTrainFenMethod method, ChessGame* game)
	{
		_method = method;

		GetSetting();

		if (method == CTrainFenMethod::TIMER_DEL_FEN_ON) {
			TimerFenDelOneON();
		}
		else if (method == CTrainFenMethod::TIMER_DEL_FEN_OFF) {
			TimerFenDelOneOFF();
		}
		else {

			if (game != nullptr) {
				//ChessGame cgame = *game;
				//_game = &cgame;
				_startFen = game->startingFen();
				_fens = game->fens();                 // ����Fen�б�
				_scores = game->scores();
			}
			this->start();
		}
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
		timer.setInterval(2000);  // ���ó�ʱʱ�� 1000 ms
		timer.setSingleShot(true);  // ���δ���

		QNetworkAccessManager* manager = new QNetworkAccessManager();
		// ��������
		QNetworkReply* pReplay = manager->get(req);

		// ����һ���ֲ����¼�ѭ�����ȴ���Ӧ�������˳�
		QEventLoop eventLoop;
		connect(&timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
		QObject::connect(manager, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit);
		timer.start();
		eventLoop.exec();

		int nStatusCode = 400;
		if (timer.isActive()) {  // ������Ӧ
			timer.stop();
			if (pReplay->error() != QNetworkReply::NoError) {
				// ������
				qDebug() << "Error String : " << pReplay->errorString();
			}
			else {
				QVariant variant = pReplay->attribute(QNetworkRequest::HttpStatusCodeAttribute);
				nStatusCode = variant.toInt();
				// ����״̬������һ�����ݴ���
				//QByteArray bytes = pReply->readAll();
				//qDebug() << "Status Code : " << nStatusCode;
				if (nStatusCode == 200) {
					res = pReplay->readAll();

					// �����һ�·�������ʣ���FEN���� PreFEN:   86  CurFEN:   85
					if (res.contains("CurFEN")) {
						QStringList token = res.split(" ");
						if (token.count() >= 4) {
							int num = token[3].toInt();
							if (num >= 1 && num <= CTrainFen::MaxFEN) {
								CTrainFen::ServerFENnum = num;
							}
						}	

					}
				}				
			}
		}
		else {  // ����ʱ
			disconnect(pReplay, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
			pReplay->abort();
			pReplay->deleteLater();
			qDebug() << "Timeout";
		}
		return nStatusCode;
	}

	bool CTrainFen::FenAddLostGame()
	{

		static int totalNum = 0;
		//this->addOneFenToWEB("2bakN3/4aR3/4b4/1C1R5/6n2/9/C6c1/2n1B3c/4A3r/4KA3 w - - 0 1", 0, true);		

		// �õ���ǰ�������岽���͵÷֣��ֱ��ϴ�

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

		//******************* ׼����������
		//for (int i = 0; i < 100; i++) {

		//}


		// ���ҳ����֣���ǰ��10���������
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
				score = -1;       // ֻ�ϴ�����ķ���
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
		startPly -= _minSteps;   // ��ǰ��������
		if (startPly <= 0) {
			startPly = 0;
		}
		
		// �����ϴ���������
		if (endPly > startPly + 60) {
			endPly = startPly + 60;
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
				score = -1;   // ֻ�ϴ�����ķ���
			}	
			
			ply += _stepsGap;
						
			this->addOneFenToWEB(fen, score, !bHaveFirst);
			bHaveFirst = true;
			numUp++;
		}


		QString info = "���ι��ϴ� " + QString::number(numUp) + " ������";
		emit SendSignal(3,info);

		totalNum += numUp; 
		emit SendSignal(1, "��ǰ���ϴ���:" + QString::number(totalNum) + " ������");

		return true;
	}

	// �߷ֺ���
	bool CTrainFen::FenAddDrawTooHigh()
	{
		// 

	
		bool bHaveFirst = false;
		int numUp = 0;  // �ϴ��ľ�����
	
		int HIGH_SCORE = _drawHighScore;
		int HIGH_TIMES = _drawHignScoreNum;
		// ���ҳ��߷�

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
				score = -1;       // ֻ�ϴ�����ķ���
			}
			totalScore += abs(score);   // ��������Ǿ��Է���
		}

		if (totalScore / HIGH_TIMES > HIGH_SCORE) {
			find = true;
		}

		if (find == false) {

			emit SendSignal(3, "���Ծֲ��Ǹ߷ֺ��壡");
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

			//ply += _stepsGap; �������

			this->addOneFenToWEB(fen, score, !bHaveFirst);
			bHaveFirst = true;
			numUp++;
		}


		QString info = "���ι��ϴ� " + QString::number(numUp) + " ������";
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
		emit SendSignal(1, "������վɾ�� " + QString::number(num) + " ��FEN");		
		return true;
	}

	// ����һЩ���õ�FEN
	bool CTrainFen::FenCommon()
	{		

		QStringList fenList;

		fenList.append("rnbak1C2/2r1a4/1c4nc1/p3p3p/6p2/2p6/P3P1P1P/1CN3N2/8R/R1BAKAB2 w - - 0 1");
		fenList.append("4kab2/4a1c2/2n1b4/p1p1nRP1p/4p4/2P3P2/Pcr4rP/R1N1CAN2/6C2/2BAK1B2 w - - 0 1");
		fenList.append("r2akabr1/4n1c2/4b1c2/pC2C3p/2P3p2/4P4/P5P1P/6N2/9/RNBAKAB2 w - - 0 1");
		fenList.append("2rakabr1/9/1c2b1n1c/p3p3p/3R2p2/9/P1p1P1P1P/C2CB1N2/9/RN1AKAB2 b - - 0 1");
		fenList.append("rnbakab2/9/1c4n1c/p1P1p1p1p/9/6P2/P3P3P/2C1C1N2/9/RNrAKAB1R b - - 0 1");
		fenList.append("rnbak1C2/2r1a4/1c4nc1/p3p3p/6p2/2p6/P3P1P1P/C1N3N2/9/R1BAKAB1R w - - 0 1");
		fenList.append("rn1akabnr/9/2c1b2c1/p3p1p1p/9/9/P3P1P1P/BCp1C1N2/3N5/R2AKABR1 w - - 0 1");
		fenList.append("rnbakabnr/9/1c7/p1p1p3p/6p2/2P3P2/P3c3P/1C4C2/9/RNBAKABNR w - - 0 1");
		fenList.append("rn1akabnr/9/4b2c1/p3p1p1p/2P6/9/P3P1P1P/1Cc1C4/9/R1BAKABNR w - - 0 1");
		fenList.append("rn1akab1r/9/1c2b2c1/p3p3p/1CP3pn1/9/P3P1P1P/2N2C2N/9/R1BAKAB1R b - - 0 1");  // �����б��Թ����ƾ���
		fenList.append("r3kabn1/4a4/1c2b4/p3p1p1p/1np6/5NP2/P3P3P/N1C1C4/3RA4/2c1KAB2 w - - 0 1");     // �Թ�
		fenList.append("r1bakabr1/6c2/1cn6/p1p1pR2p/6pn1/2P1P4/P5P1P/1C2C1N2/9/RNBAKAB2 w - - 0 1");    
		fenList.append("r1b1kabr1/4a1c2/1cn3n2/p1p1pR2p/3NP4/2P6/P5p1P/1C2C4/9/RNBAKAB2 b - - 0 1");
		fenList.append("2bakabr1/r5c2/1cn3n2/p1p1pR2p/4P1p2/2P6/P5P1P/1C2C1N2/9/RNBAKAB2 b - - 0 1");
		fenList.append("rnbakabnr/9/2c4c1/p3p1p1p/2p6/2P6/P3P1P1P/1CN1C4/9/R1BAKABNR b - - 0 1");
		fenList.append("rn1akab2/2r6/1c2b1nc1/p3p1C1p/2p3p2/2P6/P3P1P1P/1CN3N2/7R1/R1BAKAB2 b - - 0 1");
		fenList.append("rnbaka3/2r6/1c2b1nc1/p1p1p1C1p/6p2/2P6/P3P1P1P/1CN3N2/8R/R1BAKAB2 b - - 0 1");
		fenList.append("rnbak2C1/4a3r/1c5c1/p1p1p3p/9/2P2np2/P3P3P/1CN5N/9/R1BAKAB1R w - - 0 1");
		fenList.append("rnbakabr1/9/2c1c1n2/2p1p1p1p/9/pNP6/4P1P1P/1C4NC1/9/R1BAKABR1 w - - 0 1");
		fenList.append("rnbakabnr/9/2c4c1/p3p1p1p/9/2p3P2/P3P3P/NC4NC1/9/R1BAKAB1R b - - 0 1");
		fenList.append("rnbakabnr/9/1c5c1/p1p1p3p/9/2P3p2/P3P3P/1C5C1/9/RNBAKABNR w - - 0 1");
		fenList.append("1rbakab1r/9/c1n4c1/p1p1p3p/5np2/2P6/P3P1P1P/2N1C1C1N/9/R1BAKABR1 w - - 0 1");
		//fenList.append("");
		//fenList.append("");
		//fenList.append(""); rnbak1C2/2r1a4/1c4nc1/p3p3p/6p2/2p6/P3P1P1P/1CN3N2/8R/R1BAKAB2 w - - 0 7
		int num = 0;
		for (auto fen : fenList) {
			this->addOneFenToWEB(fen);
			this->msleep(10);
			num++;
		}
		emit emit SendSignal(3, "�����ϴ� " + QString::number(num) + " ������");
		return true;
	}

	bool CTrainFen::FenRemoveAll()
	{
		QUrl url(this->_WebAddress);

		QUrlQuery query; // key-value ��
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

		
		// ������Ϣ��status��
		if (stat == 200) {
			emit SendSignal(1, "�������FEN �ɹ� ");
		}
		else {
			emit SendSignal(1, "�������FEN ʧ�� ");
		}
		emit SendSignal(2, "");

		/// <summary>
		///  ��ձ��� Fen
		/// </summary>
		/// <returns></returns>
		QSettings s;
		QStringList fenlist;

		// ��д������
		s.beginWriteArray("UploadFenArray");
		s.endArray();

		if (stat != 200) return false;
		return true;
	}

	bool CTrainFen::FenLast50()
	{
		static int totalNum = 0;
		//this->addOneFenToWEB("2bakN3/4aR3/4b4/1C1R5/6n2/9/C6c1/2n1B3c/4A3r/4KA3 w - - 0 1", 0, true);		

		// �õ���ǰ�������岽���͵÷֣��ֱ��ϴ�

		//ChessGame* game = this->pMain->GetCurrentChessGame();  // _game 

		//ChessGame* game = _game;
		//QString startFen = game->startingFen();
		//QString startFen = game->board()->startingFenString();
		int numUp = 0;
		bool bHaveFirst = false;
		//if (_startFen != "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1") {
		//	this->addOneFenToWEB(_startFen, 0, true);
		//	numUp++;
		//	bHaveFirst = true;
		//}

		//******************* ׼����������
		//for (int i = 0; i < 100; i++) {

		//}


		// ���ҳ����֣���ǰ��10���������
		//int startPly = 400;
		//bool findStart = false;
		int endPly = _fens.count() - 1;
		int score = 0;
		int Start = _fens.count() - _drawHignScoreNum;
		if (Start < 0) Start = 0;

		for (int ply = Start ; ply < _fens.count(); ply++) {

			//QString fen = _fens[ply];
			if (_scores.contains(ply)) {
				score = _scores[ply];
				if (score == MoveEvaluation::NULL_SCORE) {
					score = -1;
				}
			}
			else {
				score = -1;       // ֻ�ϴ�����ķ���
			}
			if (score < (-_maxScore)) {  // we find end move
				endPly = ply;
				break;
			}
		}


		for (int ply = Start; ply < endPly; ply++) {

			QString fen = _fens[ply];
			if (_scores.contains(ply)) {
				score = _scores[ply];
				if (score == MoveEvaluation::NULL_SCORE) {
					score = -1;
				}
			}
			else {
				score = -1;   // ֻ�ϴ�����ķ���
			}

			ply += _stepsGap;

			this->addOneFenToWEB(fen, score, !bHaveFirst);
			bHaveFirst = true;
			numUp++;
		}


		QString info = "���ι��ϴ� " + QString::number(numUp) + " ������";
		emit SendSignal(3, info);

		totalNum += numUp;
		emit SendSignal(1, "��ǰ���ϴ���:" + QString::number(totalNum) + " ������");

		return true;
	}



	void CTrainFen::TimerFenDelOneON()
	{
		// https://blog.csdn.net/hanzhen7541/article/details/78831424		
		if (this->_timer == nullptr) {
			this->_timer = new QTimer(this);			
			this->_timerThread = new QThread();

			this->_timer->moveToThread(this->_timerThread);

			connect(this->_timer, SIGNAL(timeout()),
				this, SLOT(handleTimeout()), Qt::DirectConnection);

		}
		
		int delayms = getDelDelayMs();

		this->_timer->start(delayms);

		emit SendSignal(3, "��ʱ " + QString::number(delayms /1000) + " ��ɾ��һ����ʼ��");
	}

	void CTrainFen::TimerFenDelOneOFF()
	{		
		this->_timer->stop();
		emit SendSignal(3, "��ʱɾ��һ��Fen ֹͣ��");
	}

	bool CTrainFen::GetTfenTsideFromFEN(QString fen, QString& tfen, QString& tside) {
		
		// �ָ�һ��FEN 
		QStringList list = fen.split(" ");
		if (list.count() != 6) {			
			emit SendSignal(1, "�����ˣ�FEN ��ʽ��׼ȷ, ����6�ո�");			
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
			emit SendSignal(1, "�����ˣ�FEN ��ʽ��׼ȷ��side");
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

		QUrlQuery query; // key-value ��
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

		QString info = "����: " + QString::number(score); //  +" FEN: " + fen;
		// ������Ϣ��status��
		if (stat == 200) {			
			emit SendSignal(1, "�ϴ��ɹ� " + info);
		}	
		else {
			emit SendSignal(1, "�ϴ�ʧ�� " + info);
		}
		emit SendSignal(2, res);
		//emit SendSignal(3, info);
		
		if (stat != 200) return false;

		// ���浽fen�����У��Ա�����ɾ��
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

		QUrlQuery query; // key-value ��
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
	
		// ������Ϣ��status��
		if (stat == 200) {
			emit SendSignal(1, "ɾ���ɹ� " + fen);
		}
		else {
			emit SendSignal(1, "ɾ��ʧ�� " + fen);
		}
		emit SendSignal(2, res);

		if (stat != 200) return false;	

		return true;		
	}

	bool CTrainFen::delFirstOneFenFrom()
	{


		QUrl url(this->_WebAddress);

		QUrlQuery query; // key-value ��
		query.addQueryItem("user", _userName);
		query.addQueryItem("pass", _passWord);
		query.addQueryItem("method", "remove_one");
		query.addQueryItem("trainID", QString::number(_trainID));	
		query.addQueryItem("first", "no");
		url.setQuery(query);

		QString res;
		int stat = this->getWebInfoByQuery(url, res);
		emit SendSignal(2, res);
		// ������Ϣ��status��
		if (stat == 200) {
			emit SendSignal(3, "��ʱɾ��һ��Fen�ɹ�");;
		}
		else {
			emit SendSignal(3, "��ʱɾ��һ��Fenʧ��");;
		}
		return true;
	}

	bool CTrainFen::AddOneFenToSetting(QString fen)
	{
		QSettings s;
		QStringList fenlist;

		fenlist.append(fen);

		//��ȡ����
		int size = s.beginReadArray("UploadFenArray");             
		for (int i = 0; i < size; i++) {
			s.setArrayIndex(i);
			fenlist.append(s.value("FEN").toString());			
		}
		s.endArray();

		if (_saveBlind) {
			// ��д������
			s.beginWriteArray("UploadFenArray");
			for (int i = 0; i < fenlist.size(); i++) {
				s.setArrayIndex(i);
				s.setValue("FEN", fenlist[i]);
			}
			s.endArray();
		}

		//emit SendSignal(1, "�ɹ�");
		//QString info = "�������ϴ� " + QString::number(fenlist.size()) + " ������";
		//emit SendSignal(3,info);

		return true;
	}

	bool CTrainFen::GetAndDelOneFenFromSetting(QString& fen)
	{
		QSettings s;
		QStringList fenlist;


		bool find = false;

		//��ȡ����
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

		// ��д������
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
		// �õ�������Ϣ
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

	int CTrainFen::getDelDelayMs()
	{
		int minDelayMs = QSettings().value("trainFen/TimerDelOne", 20).toInt() * 1000;	

		int num = (CTrainFen::MaxFEN - ServerFENnum)/2;
		if (num < 0) {
			num = 0;
		}
		return minDelayMs + 1000 * num; // ÿ��һ��fen, �Ͷ�һ����ʱ
	}

	void CTrainFen::handleTimeout() {  //��ʱ������
		static int num = 1;
		delFirstOneFenFrom();			

		this->_timer->stop();

		int newDelay = getDelDelayMs();
		this->_timer->start(newDelay);

		emit SendSignal(1, "��ɾ��: " + QString::number(num++) + " FEN ��ʱ: " + QString::number(newDelay/1000) + " s" );
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
			case CTrainFenMethod::TIMER_DEL_FEN_ON:
				
				//TimerFenDelOneON();
				break;
			case CTrainFenMethod::TIMER_DEL_FEN_OFF:
				//TimerFenDelOneOFF();
				break;
			case CTrainFenMethod::LAST_50:
				FenLast50();
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