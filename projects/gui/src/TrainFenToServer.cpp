
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
			_fens = game->fens();
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

		QNetworkAccessManager* manager = new QNetworkAccessManager();
		// ��������
		QNetworkReply* pReplay = manager->get(req);

		// ����һ���ֲ����¼�ѭ�����ȴ���Ӧ�������˳�
		QEventLoop eventLoop;
		QObject::connect(manager, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit);
		eventLoop.exec();

		// ��ȡ��Ӧ��Ϣ
		res = pReplay->readAll();

		//��ȡhttp״̬��

		 // QNetworkReply
	//  // attribute��������QVariant����ʹ��value<T>()�������ؽ�������ת��
		//qDebug() << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).value<int>();



		return pReplay->attribute(QNetworkRequest::HttpStatusCodeAttribute).value<int>();

		//QString s_date = bytes;
		//QStringList qlist = s_date.split('|');

		//return bytes;
	}

	bool CTrainFen::FenAdd()
	{

		//this->addOneFenToWEB("2bakN3/4aR3/4b4/1C1R5/6n2/9/C6c1/2n1B3c/4A3r/4KA3 w - - 0 1", 0, true);		

		// �õ���ǰ�������岽���͵÷֣��ֱ��ϴ�

		//ChessGame* game = this->pMain->GetCurrentChessGame();  // _game 

		//ChessGame* game = _game;
		//QString startFen = game->startingFen();
		//QString startFen = game->board()->startingFenString();
		if (_startFen != "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1") {
			this->addOneFenToWEB(_startFen, 0, true);
		}

		//const QVector<Chess::Move>& moves() const;
		//const QMap<int, int>& scores() const;
		//const QVector<QString>& fens() const { return m_fens; };

		//auto fens = game->fens();
		//auto scores = game->scores();

		//int ply = 0;
		int score = 0;
		//for (auto fen : _fens) {
		for(int ply=0; ply < _fens.count(); ply++){

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

			if (abs(score) > _maxScore) {  // ���泬�������֣�����Ͳ��ϴ���
				break;
			}
			if (abs(score) < _minScore) {     // ���ֿⲻ�ϴ�ѵ��
				continue;
			}
			if (ply < _minSteps) {
				continue;
			}
			if (ply > _maxSteps) {
				continue;
			}

			ply += _stepsGap;

			this->addOneFenToWEB(fen, score, false);
			
		}
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

		fenList.append("rnbakabnr/9/1c5c1/p1p1p1p1p/9/6P2/P1P1P3P/1C5C1/9/RNBAKABNR b - - 0 1");
		fenList.append("rnbakabnr/9/1c5c1/p1p1p1p1p/9/P8/2P1P1P1P/1C5C1/9/RNBAKABNR b - - 0 1");
		fenList.append("rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1CN4C1/9/R1BAKABNR b - - 0 1");
		fenList.append("rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C2B2C1/9/RN1AKABNR b - - 0 1");
		fenList.append("rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C4C2/9/RNBAKABNR b - - 0 1");
		fenList.append("rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C3C3/9/RNBAKABNR b - - 0 1");
		fenList.append("rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5CN/9/RNBAKAB1R b - - 0 1");
		fenList.append("rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/4A4/RNBAK1BNR b - - 0 1");
		fenList.append("rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/5C1C1/9/RNBAKABNR b - - 0 1");
		fenList.append("rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/6CC1/9/RNBAKABNR b - - 0 1");
		fenList.append("rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/C6C1/9/RNBAKABNR b - - 0 1");
		//fenList.append("");
		//fenList.append("");

		for (auto fen : fenList) {
			this->addOneFenToWEB(fen);
			this->msleep(100);
		}
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

		// ��д������
		s.beginWriteArray("UploadFenArray");
		for (int i = 0; i < fenlist.size(); i++) {
			s.setArrayIndex(i);
			s.setValue("FEN", fenlist[i]);
		}	
		s.endArray();

		//emit SendSignal(1, "�ɹ�");
		QString info = "�������ϴ� " + QString::number(fenlist.size()) + " ������";
		emit SendSignal(3,info);

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
				FenAdd();
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