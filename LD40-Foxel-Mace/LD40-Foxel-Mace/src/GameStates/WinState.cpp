#include "GameStates\WinState.h"
#include "AssetLoader\KAssetLoader.h"
#include <KApplication.h>

using namespace Krawler::LogicState;

Krawler::KInitStatus WinState::setupState(const KLogicStateInitialiser & initaliser)
{
	KINIT_CHECK(KLogicState::setupState(initaliser));
	if (status != Success)
		return status;

	mp_winText = sf::Text();
	mp_winText.setCharacterSize(32);
	mp_winText.setString(KTEXT("You have defeated the evil morebots!"));
	mp_winText.setFont(*KAssetLoader::getAssetLoader().loadFont(L"res\\seriphim.ttf"));
	return Krawler::KInitStatus::Success;
}

void WinState::cleanupState()
{

	KLogicState::cleanupState();
}

void WinState::tick()
{
	KLogicState::tick();
	if (!m_bHasStateTicked)
	{
		m_bHasStateTicked = true;
		auto app = KApplication::getApplicationInstance();
		const auto screenSize = app->getRenderWindow()->getSize();
		app->getRenderer()->clearRenderQueue();
		auto view = app->getRenderWindow()->getView();
		view.setCenter(Vec2f(screenSize) * 0.5f);
		app->getRenderWindow()->setView(view);
		const auto globalBounds = mp_winText.getGlobalBounds();
		const Vec2i textPos = Vec2i(screenSize.x / 2, screenSize.y / 2) - Vec2i((int)globalBounds.width / 2.0f, (int)globalBounds.height / 2.0f);
		app->getRenderer()->addTextToScreen(&mp_winText, textPos);
	}
}
