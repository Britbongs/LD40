#include "GameStates\PlayState.h"

#include <AssetLoader\KAssetLoader.h>
#include <Input\KInput.h>
#include <KApplication.h>
#include <Renderer\KRenderer.h>

#include "LogicUnits\Camera.h"
#include "LogicUnits\WorldCollisions.h"
#include "LogicUnits\AIBehaviour.h"

#include <Physics\KPhysicsScene.h>

using namespace Krawler::Input;
using namespace sf;

#define TILE_SIZE 32
#define MAP_WIDTH (TILE_SIZE * 30)
#define MAP_HEIGHT (TILE_SIZE * 20)

PlayState::PlayState()
	: PlayerNormal(1.0f, 0.f), PlayerMoveSpeed(250.0f), MaxRaycastDistance(350)
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

	m_meshColliders.push_back(new MeshCollider(points, pPlayer));
	m_meshColliders.push_back(new MeshCollider(points, pEntity));

	m_tiledMap.setTexture(KTEXT("res/bg.png"));
	int* map = new int[MAP_WIDTH * MAP_HEIGHT];

	for (int i = 0; i < MAP_WIDTH * MAP_HEIGHT; ++i)
	{
		map[i] = rand() % 3;
	}
	m_tiledMap.setupTiledMapFromArray(map, Vec2i(MAP_WIDTH, MAP_HEIGHT), Vec2i(TILE_SIZE, TILE_SIZE));
	m_tiledMap.setScale(2.0f, 2.0f);
	KFREE_ARR(map);

	KApplication::getApplicationInstance()->getRenderer()->setActiveTiledMap(&m_tiledMap);

	setupAI();

	registerLogicUnits();

	mp_slAdmin->initAllUnits();

	mp_physicsScene->setPercentageCorrection(0.8f);

	return KInitStatus::Success;
}

void PlayState::cleanupState()
{
	for (auto& meshCol : m_meshColliders)
	{
		KFREE(meshCol);
	}
	m_meshColliders.clear();
	mp_slAdmin->cleanupAllUnits();
	m_tiledMap.cleanupTiledMap();
}

void PlayState::fixedTick()
{
	KLogicState::fixedTick();
}

void PlayState::tick()
{
	if (KInput::JustPressed(KKey::Escape))
	{
		KApplication::getApplicationInstance()->closeApplication();
	}

	for (auto& mesh : m_meshColliders)
	{
		mesh->UpdatMeshCollider();
	}


	mp_slAdmin->tickAllUnits();
	handlePlayerControls();
	handleAI();
}

void PlayState::handlePlayerControls()
{
	auto collider = mp_slAdmin->getStateLogicUnit<WorldCollisions>(KTEXT("WorldCollisions"));

	if (collider->CheckCollision(*m_meshColliders[0]).bDidCollide)
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
			const Vec2f dir(cosf(angle), sinf(angle));
			const Vec2f size(16.0f, 16.0f);
			const bool didHit = collider->DidOBBRaycastHit(size, Maths::Degrees(angle), mp_playerObj->getCentrePosition(), mp_playerObj->getCentrePosition() + (dir * MaxRaycastDistance), m_meshColliders[0]);
			//KPrintf(KTEXT("Did hit = %s\n"), didHit ? KTEXT("true") : KTEXT("false"));
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


	float deltaTime = KApplication::getApplicationInstance()->getDeltaTime();
	mp_playerObj->move(Normalise(movDir) * PlayerMoveSpeed * deltaTime);
}

void PlayState::registerLogicUnits()
{

	mp_slAdmin->addUnit(new Camera(*mp_slAdmin, mp_playerObj, Vec2f(MAP_WIDTH, MAP_HEIGHT) * 2.0f));
	mp_slAdmin->addUnit(new WorldCollisions(m_meshColliders, *mp_slAdmin));

	for (int32 i = 0; i < MAX_AI_COUNT; ++i)
	{
		const int32 transIndex = (i + m_firstAiIdx);
		const int32 transIndexMesh(i + m_firstAIMeshIdx);

		AIBehaviour* pAIBehaviour = new AIBehaviour(m_gameObjects[transIndex], *mp_slAdmin);

		pAIBehaviour->setMeshCollider(m_meshColliders[transIndexMesh]);
		mp_slAdmin->addUnit(pAIBehaviour);

		pAIBehaviour->setPlayerPointer(mp_playerObj);

	}
}

void PlayState::setupAI()
{
	std::vector<KGameObject*> vecAIPtrs(MAX_AI_COUNT, nullptr);
	std::vector<Vec2f>meshColliderVertices(4);

	meshColliderVertices[0] = Vec2f(0.0f, 0.0f);
	meshColliderVertices[1] = Vec2f(32.0f, 0.0f);
	meshColliderVertices[2] = Vec2f(32.0f, 32.0f);
	meshColliderVertices[3] = Vec2f(0.0f, 32.0f);

	int32 idx = 0;
	const int32 originalSizeObjectList = (signed)m_gameObjects.size();
	const int32 originalSizeMeshList = (signed)m_meshColliders.size();
	m_firstAiIdx = originalSizeObjectList;
	m_firstAIMeshIdx = originalSizeMeshList;

	for (auto& pObj : vecAIPtrs)
	{
		pObj = addGameObject(Vec2f(32.0f, 32.0f), true);
		pObj->setFillColour(sf::Color(84, 47, 163));
		pObj->setName(KTEXT("AI-") + GenerateUUID());
		pObj->setObjectInactive();

		m_meshColliders.push_back(new MeshCollider(meshColliderVertices, pObj));

	}
}

void PlayState::handleAI()
{
	if (m_aiTimer == 0.0f)
	{
		spawnAI();
	}
	m_aiTimer += KApplication::getApplicationInstance()->getDeltaTime();
}

void PlayState::spawnAI()
{
	const Vec2f screenSize(KApplication::getApplicationInstance()->getRenderWindow()->getSize());
	const Vec2f viewCentre(KApplication::getApplicationInstance()->getRenderWindow()->getView().getCenter());
	auto getSpareAI = [](std::vector<KGameObject*>& objList) -> KGameObject*
	{
		for (auto& member : objList)
		{
			if (member->isGameObjectActive())
			{
				continue;
			}

			auto isInString = member->getObjectName().find(KTEXT("AI"));
			if (isInString != std::wstring::npos)
			{
				return member;
			}
		}
		return nullptr;
	};
	auto contains = [](const FloatRect& rect, Vec2f point)
	{
		float minX = Maths::Min(rect.left, rect.left + rect.width);
		float maxX = Maths::Max(rect.left, rect.left + rect.width);
		float minY = Maths::Min(rect.top, rect.top + rect.height);
		float maxY = Maths::Max(rect.top, rect.top + rect.height);

		return (point.x >= minX) && (point.x < maxX) && (point.y >= minY) && (point.y < maxY);
	};

	KGameObject* pObjects[10];

	sf::FloatRect viewBounds;
	viewBounds.left = viewCentre.x - (screenSize.x / 2.0f);
	viewBounds.height = viewCentre.y - (screenSize.y / 2.0f);
	viewBounds.width = screenSize.x;
	viewBounds.height = screenSize.y;

	for (auto& pObj : pObjects)
	{
		pObj = getSpareAI(m_gameObjects);
		pObj->setObjectActive();

		Vec2f pos;
		do
		{
			pos.x = Maths::RandFloat(0.0f, MAP_WIDTH);
			pos.y = Maths::RandFloat(0.0f, MAP_HEIGHT);

		} while (contains(viewBounds, pos));

		//TODO stop enemies spawning slightly outside map

		pObj->setPosition(pos);
	}
}
