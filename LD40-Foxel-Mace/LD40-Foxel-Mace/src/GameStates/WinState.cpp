#include "GameStates\WinState.h"
#include "AssetLoader\KAssetLoader.h"

#include <KApplication.h>
#include <Input\KInput.h>

using namespace Krawler::LogicState;

Krawler::KInitStatus WinState::setupState(const KLogicStateInitialiser & initialiser)
{
	KINIT_CHECK(KLogicState::setupState(initialiser));
	if (status != Success)
		return status;

	m_winText.setCharacterSize(48);
	m_winText.setString(KTEXT("You have defeated \n the evil MoreBots!"));
	m_winText.setFont(*KAssetLoader::getAssetLoader().loadFont(KTEXT("seriphim.ttf")));
	m_winText.setStyle(sf::Text::Underlined);

	return Krawler::KInitStatus::Success;
}

void WinState::cleanupState()
{

	KLogicState::cleanupState();
}

void WinState::tick()
{
	KLogicState::tick();
	auto app = KApplication::getApp();

	if (!m_bHasStateTicked)
	{
		m_bHasStateTicked = true;
		const auto screenSize = app->getRenderWindow()->getSize();
		app->getRenderer()->clearRenderQueue();
		auto view = app->getRenderWindow()->getView();
		view.setCenter(Vec2f(screenSize) * 0.5f);
		app->getRenderWindow()->setView(view);
		const auto globalBounds = m_winText.getGlobalBounds();
		const Vec2i textPos = Vec2i(screenSize.x / 2, screenSize.y / 2) - Vec2i((int)globalBounds.width / 2.0f, (int)globalBounds.height / 2.0f);
		app->getRenderer()->addTextToScreen(m_winText, textPos);
	}

#ifdef _DEBUG
	if (Input::KInput::JustPressed(Input::KKey::Escape))
	{
		app->closeApplication();
	}
#endif
}
