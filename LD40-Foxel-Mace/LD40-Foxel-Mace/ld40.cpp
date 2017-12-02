#include <iostream>

#include <Krawler.h>
#include <KApplication.h>

#include <vld.h>

#include "Game\Game.h"	

using namespace std;
using namespace Krawler;

int main(int argc, char argv[])
{

	Game game;

	KApplicationInitialise krawlerInitApplicationData;
	krawlerInitApplicationData.width = 1024;
	krawlerInitApplicationData.height = 768;
	krawlerInitApplicationData.windowTitle = KTEXT("LD40 - Alien Thing!");
	krawlerInitApplicationData.windowStyle = KWindowStyle::Windowed_Fixed_Size;

	StartupEngine(&krawlerInitApplicationData);
	game.registerGameStates();
	KApplication* pApp = KApplication::getApplicationInstance();
	pApp->getLogicStateDirector()->setActiveLogicState(KTEXT("play"));

	InitialiseSubmodules();
	RunApplication();

	ShutdownEngine();

	return 0;
}
