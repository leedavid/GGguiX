/*
    This file is part of GGzero Chess.
    Copyright (C) 2008-2018 GGzero Chess authors

    GGzero Chess is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GGzero Chess is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GGzero Chess.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma execution_character_set("utf-8")

#include "tournamentresultsdlg.h"

#include <limits>

#include <QPlainTextEdit>
#include <QBoxLayout>
#include <QFont>

#include <tournament.h>

TournamentResultsDialog::TournamentResultsDialog(QWidget* parent)
	: QDialog(parent)
{
	setWindowTitle(tr("Tournament Results"));

	m_resultsEdit = new QPlainTextEdit(this);
	m_resultsEdit->setReadOnly(true);

	QFont font("Courier New");
	font.setStyleHint(QFont::Monospace);
	font.setPointSize(font.pointSize() - 1);
	m_resultsEdit->document()->setDefaultFont(font);

	auto layout = new QBoxLayout(QBoxLayout::TopToBottom);
	layout->addWidget(m_resultsEdit);
	layout->setContentsMargins(0, 0, 0, 0);

	setLayout(layout);
	resize(700, 400);
}

TournamentResultsDialog::~TournamentResultsDialog()
{
}

void TournamentResultsDialog::setTournament(Tournament* tournament)
{
	setWindowTitle(tournament->name());

	// 实始化一下
	QString text;
   if (tournament->playerCount() == 2) {
		TournamentPlayer fcp = tournament->playerAt(0);
		TournamentPlayer scp = tournament->playerAt(1);
		int totalResults = fcp.gamesFinished();
		double scoreRatio = std::numeric_limits<double>::quiet_NaN();
		if (totalResults > 0)
			scoreRatio = double(fcp.score()) / (totalResults * 2);
		//text = tr("Score of %1 vs %2: %3 - %4 - %5 [%6]\n")
		text = tr("Score of %1 vs %2: 胜 %3 负 %4 和 %5 [%6]\n")
			.arg(fcp.name())
			.arg(scp.name())
			.arg(0)
			.arg(0)
			.arg(0)
			.arg(0.0, 0, 'f', 3);
	}


	m_resultsEdit->setPlainText(text + tournament->results());
}

void TournamentResultsDialog::update()
{
	auto tournament = qobject_cast<Tournament*>(QObject::sender());
	Q_ASSERT(tournament != nullptr);
	QString text;

	// A quick fix, copied from the CLI side.
	if (tournament->playerCount() == 2 && tournament->type() != "knockout")
	{
		TournamentPlayer fcp = tournament->playerAt(0);
		TournamentPlayer scp = tournament->playerAt(1);
		int totalResults = fcp.gamesFinished();
		double scoreRatio = std::numeric_limits<double>::quiet_NaN();
		if (totalResults > 0)
			scoreRatio = double(fcp.score()) / (totalResults * 2);
		text = tr("Score of %1 vs %2: 胜 %3 负 %4 和 %5 [%6]\n")
		       .arg(fcp.name())
		       .arg(scp.name())
		       .arg(fcp.wins())
		       .arg(scp.wins())
		       .arg(fcp.draws())
		       .arg(scoreRatio, 0, 'f', 3);
	}

	text += tournament->results();
	text += tr("\n\n%1 of %2 games finished.")
		.arg(tournament->finishedGameCount())
		.arg(tournament->finalGameCount());
	m_resultsEdit->setPlainText(text);
}
