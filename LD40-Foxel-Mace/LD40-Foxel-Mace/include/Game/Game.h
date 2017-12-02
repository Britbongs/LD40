#ifndef GAME_H
#define GAME_H

#include <Krawler.h>
#include <KApplication.h>
#include <LogicState\KLogicStateDirector.h>
#include "GameStates\PlayState.h"

class Game
{
public:

	void registerGameStates();
	void run();

private:

};

#endif 
