#include "GameStates\PlayState.h"

#include <AssetLoader\KAssetLoader.h>
#include <Input\KInput.h>
#include <KApplication.h>

#include "LogicUnits\Camera.h"

using namespace Krawler::Input;
using namespace sf;


PlayState::PlayState()
	: PlayerNormal(1.0f, 0.f), PlayerMoveSpeed(5.0f)
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
	auto t = rAssetLoader.loadTexture(KTEXT("grass.jpg"));

	auto pBackground = addGameObject(Vec2f(t->getSize()));

	pBackground->setRenderLayer(-2);
	pBackground->setTexture(t);

	auto pPlayer = addGameObject(Vec2f(64, 64));
	pPlayer->setFillColour(sf::Color::Red);
	pPlayer->setOrigin(pPlayer->getHalfLocalBounds());
	pPlayer->setPosition(Vec2f(100, 100));
	mp_playerObj = pPlayer;

	registerLogicUnits();

	mp_stateLogicAdmin->addUnit(new Camera(*mp_stateLogicAdmin, pPlayer, Vec2f(t->getSize())));

	mp_stateLogicAdmin->initAllUnits();

	return KInitStatus::Success;
}

void PlayState::cleanupState()
{
	mp_stateLogicAdmin->cleanupAllUnits();
}

void PlayState::fixedTick()
{
}

void PlayState::tick()
{
	mp_stateLogicAdmin->tickAllUnits();
	float rotAngle = 0.0f;
	const Vec2f mousePos = KInput::GetMouseWorldPosition();
	const Vec2f playerCentre = mp_playerObj->getCentrePosition();

	float angle = atan2(mousePos.y - playerCentre.y, mousePos.x - playerCentre.x);
	mp_playerObj->setRotation(Maths::Degrees(angle));

	Vec2f movDir;

	if (KInput::Pressed(KKey::A))
	{
		movDir.x = -1.0f;
	}

	if (KInput::Pressed(KKey::D))
	{
		movDir.x = 1.0f;
	}

	if (KInput::Pressed(KKey::W))
	{
		movDir.y = -1.0f;
	}

	if (KInput::Pressed(KKey::S))
	{
		movDir.y = 1.0f;
	}

	if (KInput::JustPressed(KKey::Escape))
	{
		KApplication::getApplicationInstance()->closeApplication();
	}

	mp_playerObj->move(Normalise(movDir) * PlayerMoveSpeed);
}

void PlayState::registerLogicUnits()
{
}
