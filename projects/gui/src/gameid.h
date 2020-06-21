#pragma once

#include <QtCore>

typedef quint32 GameId;
static const GameId InvalidGameId = static_cast<GameId>(-1);
#define VALID_INDEX(index) (index!=InvalidGameId)
