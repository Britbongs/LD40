#include "GameStates\PlayState.h"

#include <AssetLoader\KAssetLoader.h>
#include <Input\KInput.h>

using namespace Krawler::Input;

PlayState::PlayState()
{

}

KInitStatus PlayState::setupState(const KLogicStateInitialiser & initaliser)
{
	auto result = KLogicState::setupState(initaliser);
	if (result != Success)
	{
		return result;
	}
	auto rAssetLoader = KAssetLoader::getAssetLoader();
	rAssetLoader.setRootFolder(KTEXT("res/"));
	auto pBackground = addGameObject(Vec2f(1024, 1024));
	pBackground->setRenderLayer(-2);
	auto t = rAssetLoader.loadTexture(KTEXT("grass.jpg"));
	pBackground->setTexture(t);

	auto pPlayer = addGameObject(Vec2f(64, 64));
	pPlayer->setFillColour(sf::Color::Red);
	pPlayer->setPosition(Vec2f(100, 100));


	return KInitStatus::Success;
}

void PlayState::cleanupState()
{
}

void PlayState::fixedTick()
{
}

void PlayState::tick()
{
	Vec2f movVec;
	if (KInput::JustPressed(sf::Keyboard::Key::Left))
	{

	}

	if (KInput::JustPressed(sf::Keyboard::Key::Left))
	{

	}

	if (KInput::JustPressed(sf::Keyboard::Key::Left))
	{

	}

	if (KInput::JustPressed(sf::Keyboard::Key::Left))
	{

	}
}
