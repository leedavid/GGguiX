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

#include "genericmove.h"

namespace Chess {

GenericMove::GenericMove()
	//: m_promotion(0)
{
}

GenericMove::GenericMove(const Square& sourceSquare,
			 const Square& targetSquare)
			// int promotion)
	: m_sourceSquare(sourceSquare),
	  m_targetSquare(targetSquare)
	//  m_promotion(promotion)
{
}

bool GenericMove::operator==(const GenericMove& other) const
{
	if (m_sourceSquare == other.m_sourceSquare
	&&  m_targetSquare == other.m_targetSquare)
	//&&  m_promotion == other.m_promotion)
		return true;
	return false;
}

bool GenericMove::operator!=(const GenericMove& other) const
{
	if (m_sourceSquare != other.m_sourceSquare
	||  m_targetSquare != other.m_targetSquare)
	//||  m_promotion != other.m_promotion)
		return true;
	return false;
}

bool GenericMove::isNull() const
{
	bool validSource = (m_sourceSquare.isValid()); // || m_promotion);
	return !(validSource && m_targetSquare.isValid());
}

void GenericMove::setNull() {
	m_sourceSquare.setFile(-1);
	m_targetSquare.setFile(-1);
}

Square GenericMove::sourceSquare() const
{
	return m_sourceSquare;
}

Square GenericMove::targetSquare() const
{
	return m_targetSquare;
}

//int GenericMove::promotion() const
//{
//	return m_promotion;
//}

void GenericMove::setSourceSquare(const Square& square)
{
	m_sourceSquare = square;
}

void GenericMove::setTargetSquare(const Square& square)
{
	m_targetSquare = square;
}

QString GenericMove::moveFenString()
{
	//int fx = m_sourceSquare.file();
	//int fy = 9-m_sourceSquare.rank();
	//int tx = m_targetSquare.file();
	//int ty = 9-m_targetSquare.rank();

	//std::string stfrom = 
	
	return QString(m_sourceSquare.as_qstring() + m_targetSquare.as_qstring());
}

//void GenericMove::setPromotion(int pieceType)
//{
//	m_promotion = pieceType;
//}

} // namespace Chess
