#include <iostream>

#include <Krawler.h>
#include <KApplication.h>

#include <vld.h>

#include "Game\Game.h"	

extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

using namespace std;
using namespace Krawler;
#ifdef _DEBUG
int main(int argc, char argv[])
#else
int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
)
#endif
{

	Game game;

	KApplicationInitialise krawlerInitApplicationData(true);

	StartupEngine(&krawlerInitApplicationData);
	game.registerGameStates();
	KApplication* pApp = KApplication::getApp();
	pApp->getLogicStateDirector()->setActiveLogicState(KTEXT("play"));

	InitialiseSubmodules();
	RunApplication();

	ShutdownEngine();

	return 0;
}
