#include "GameStates\PlayState.h"

#include <AssetLoader\KAssetLoader.h>
#include <Input\KInput.h>
#include <KApplication.h>

#include "LogicUnits\Camera.h"
#include "LogicUnits\WorldCollisions.h"

using namespace Krawler::Input;
using namespace sf;


PlayState::PlayState()
	: PlayerNormal(1.0f, 0.f), PlayerMoveSpeed(250.0f), MaxRaycastDistance(250)
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

	mp_backgroundObj = addGameObject(Vec2f(t->getSize()));

	mp_backgroundObj->setRenderLayer(-2);
	mp_backgroundObj->setTexture(t);

	auto pPlayer = addGameObject(Vec2f(64, 64));
	pPlayer->setFillColour(sf::Color::Red);
	pPlayer->setOrigin(pPlayer->getHalfLocalBounds());
	pPlayer->setPosition(Vec2f(100, 100));
	mp_playerObj = pPlayer;

	auto pEntity = addGameObject(Vec2f(64, 64));
	pEntity->setFillColour(sf::Color::Blue);
	pEntity->setOrigin(pPlayer->getHalfLocalBounds());
	pEntity->setPosition(Vec2f(300, 100));

	std::vector<Vec2f> points(4);
	points[0] = Vec2f(0.0f, 0.0f);
	points[1] = Vec2f(64.0f, 0.0f);
	points[2] = Vec2f(64.0f, 64.0f);
	points[3] = Vec2f(0.0f, 64.0f);

	m_meshArr[0] = MeshCollider(points, pPlayer);
	m_meshArr[1] = MeshCollider(points, pEntity);


	registerLogicUnits();

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
	if (KInput::JustPressed(KKey::Escape))
	{
		KApplication::getApplicationInstance()->closeApplication();
	}

	for (auto& mesh : m_meshArr)
	{
		mesh.UpdatMeshCollider();
	}


	mp_stateLogicAdmin->tickAllUnits();
	handlePlayerControls();

}

void PlayState::handlePlayerControls()
{
	auto collider = mp_stateLogicAdmin->getStateLogicUnit<WorldCollisions>(KTEXT("WorldCollisions"));

	if (collider->CheckCollision(m_meshArr[0]).bDidCollide)
	{
		mp_playerObj->setFillColour(Color::Cyan);
	}
	else
	{
		mp_playerObj->setFillColour(Color::Red);
	}

	{//rotation and shooting
		float rotAngle = 0.0f;
		const Vec2f mousePos = KInput::GetMouseWorldPosition();
		const Vec2f playerCentre = mp_playerObj->getCentrePosition();

		float angle = atan2(mousePos.y - playerCentre.y, mousePos.x - playerCentre.x);
		if (KInput::MouseJustPressed(Mouse::Left))
		{
			Vec2f dir(cosf(angle), sinf(angle));
			Vec2f size(10.0f, 10.0f);
			bool didHit = collider->DidOBBRaycastHit(size, Maths::Degrees(angle), mp_playerObj->getCentrePosition(), mp_playerObj->getCentrePosition() + (dir * MaxRaycastDistance), &m_meshArr[0]);
			KPrintf(KTEXT("Did hit = %s\n"), didHit ? KTEXT("true") : KTEXT("false"));
		}

		mp_playerObj->setRotation(Maths::Degrees(angle));
	}
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

	
	float deltaTime = KApplication::getApplicationInstance()->getDelta();
	mp_playerObj->move(Normalise(movDir) * PlayerMoveSpeed * deltaTime);
}

void PlayState::registerLogicUnits()
{

	std::vector<MeshCollider*> tempVec(2);
	tempVec[0] = &m_meshArr[0];
	tempVec[1] = &m_meshArr[1];

	mp_stateLogicAdmin->addUnit(new Camera(*mp_stateLogicAdmin, mp_playerObj, Vec2f(mp_backgroundObj->getFixedGlobalBounds().width, mp_backgroundObj->getFixedGlobalBounds().height)));
	mp_stateLogicAdmin->addUnit(new WorldCollisions(tempVec, *mp_stateLogicAdmin));


}
