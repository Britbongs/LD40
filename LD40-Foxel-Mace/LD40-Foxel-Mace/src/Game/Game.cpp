#include "Game\Game.h"
#include <assert.h>

using namespace Krawler;

void Game::registerGameStates()
{
	KApplication* pApp = KApplication::getApplicationInstance();
	assert(pApp);
	KLogicStateDirector* pDirector = pApp->getLogicStateDirector();
	assert(pDirector);

	KLogicStateInitialiser stateInit;
	stateInit.bIsPhysicsEngineEnabled = true;
	stateInit.stateIdentifier = KTEXT("play");
	pDirector->registerLogicState(new PlayState, &stateInit);

}
