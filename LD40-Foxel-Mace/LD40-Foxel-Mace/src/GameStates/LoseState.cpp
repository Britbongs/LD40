#include "GameStates\LoseState.h"
#include <AssetLoader\KAssetLoader.h>
#include <KApplication.h>

using namespace Krawler;

KInitStatus LoseState::setupState(const LogicState::KLogicStateInitialiser & initialiser)
{
	KINIT_CHECK(KLogicState::setupState(initialiser));

	m_loseText.setCharacterSize(64);
	m_loseText.setStyle(sf::Text::Underlined);
	m_loseText.setString(KTEXT("The evil MoreBots have \n bested you in combat!"));
	m_loseText.setFont(*KAssetLoader::getAssetLoader().loadFont(KTEXT("res\\seriphim.ttf")));

	return KInitStatus::Success;
}

void LoseState::cleanupState()
{
	KLogicState::cleanupState();
}

void LoseState::tick()
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
		const auto globalBounds = m_loseText.getGlobalBounds();
		const Vec2i textPos = Vec2i(screenSize.x / 2, screenSize.y / 2) - Vec2i((int)globalBounds.width / 2.0f, (int)globalBounds.height / 2.0f);
		app->getRenderer()->addTextToScreen(m_loseText, textPos);
	}

#ifdef _DEBUG
	if (Input::KInput::JustPressed(Input::KKey::Escape))
	{
		app->closeApplication();
	}
#endif
}
