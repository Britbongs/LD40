#include "Game\Game.h"
#include <assert.h>
#include "GameStates\PlayState.h"
#include "GameStates\WinState.h"

using namespace Krawler;

void Game::registerGameStates()
{
	KApplication* pApp = KApplication::getApplicationInstance();
	assert(pApp);
	KLogicStateDirector* pDirector = pApp->getLogicStateDirector();
	assert(pDirector);

	KLogicStateInitialiser stateInit;
	stateInit.bIsPhysicsEngineEnabled = false;
	stateInit.stateIdentifier = KTEXT("play");
	pDirector->registerLogicState(new PlayState, &stateInit);

	stateInit.stateIdentifier = KTEXT("winstate");
	pDirector->registerLogicState(new WinState, &stateInit);

}
