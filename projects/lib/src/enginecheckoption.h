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

#ifndef ENGINECHECKOPTION_H
#define ENGINECHECKOPTION_H

#include "engineoption.h"

class LIB_EXPORT EngineCheckOption : public EngineOption
{
	public:
		EngineCheckOption(const QString& name,
		                  const QVariant& value = QVariant(),
		                  const QVariant& defaultValue = QVariant(),
		                  const QString& alias = QString());

		// Inherited from EngineOption
		virtual EngineOption* copy() const;
		virtual bool isValid(const QVariant& value) const;
		virtual QVariant toVariant() const;
};

#endif // ENGINECHECKOPTION_H
