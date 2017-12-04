#include "GameStates\PlayState.h"

#include <AssetLoader\KAssetLoader.h>
#include <Input\KInput.h>
#include <KApplication.h>
#include <Renderer\KRenderer.h>

#include "LogicUnits\Camera.h"
#include "LogicUnits\WorldCollisions.h"
#include "LogicUnits\AIBehaviour.h"
#include "LogicUnits\Animator.h"
#include "LogicUnits\PlayerController.h"

#include <Physics\KPhysicsScene.h>

using namespace Krawler::Input;
using namespace sf;

#define TILE_SIZE 32
#define MAP_WIDTH (TILE_SIZE * 30)
#define MAP_HEIGHT (TILE_SIZE * 20)
#define CHARACTER_SIZE 48.0f

PlayState::PlayState()
	: PlayerMoveSpeed(250.0f), MaxRaycastDistance(350)
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

	auto pPlayer = addGameObject(Vec2f(CHARACTER_SIZE, CHARACTER_SIZE));
	pPlayer->setOrigin(pPlayer->getHalfLocalBounds());
	pPlayer->setPosition(Vec2f(100, 100));
	pPlayer->setName(KTEXT("player"));

	mp_playerObj = pPlayer;


	std::vector<Vec2f> points(4);
	points[0] = Vec2f(0.0f, 0.0f);
	points[1] = Vec2f(CHARACTER_SIZE, 0.0f);
	points[2] = Vec2f(CHARACTER_SIZE, CHARACTER_SIZE);
	points[3] = Vec2f(0.0f, CHARACTER_SIZE);

	mp_playerMesh = new MeshCollider(points, pPlayer);
	m_meshColliders.push_back(mp_playerMesh);

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
	m_tiledMap.cleanupTiledMap();
	KLogicState::cleanupState();
}

void PlayState::fixedTick()
{
	KLogicState::fixedTick();
}

void PlayState::tick()
{
	for (auto& pMesh : m_meshColliders)
	{
		pMesh->UpdateMeshCollider();
	}
	KLogicState::tick();
	handleAI();
	if (KInput::JustPressed(KKey::Escape))
		KApplication::getApplicationInstance()->closeApplication();
}

void PlayState::registerLogicUnits()
{
	mp_slAdmin->addUnit(new Camera(*mp_slAdmin, mp_playerObj, Vec2f(MAP_WIDTH, MAP_HEIGHT) * 2.0f));
	mp_slAdmin->addUnit(new WorldCollisions(m_meshColliders, *mp_slAdmin));
	mp_slAdmin->addUnit(new PlayerController(mp_playerObj, *mp_slAdmin));

	mp_slAdmin->getStateLogicUnit<PlayerController>()->setMeshCollider(mp_playerMesh);

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
	meshColliderVertices[1] = Vec2f(CHARACTER_SIZE, 0.0f);
	meshColliderVertices[2] = Vec2f(CHARACTER_SIZE, CHARACTER_SIZE);
	meshColliderVertices[3] = Vec2f(0.0f, CHARACTER_SIZE);

	int32 idx = 0;
	const int32 originalSizeObjectList = (signed)m_gameObjects.size();
	const int32 originalSizeMeshList = (signed)m_meshColliders.size();
	m_firstAiIdx = originalSizeObjectList;
	m_firstAIMeshIdx = originalSizeMeshList;

	for (auto& pObj : vecAIPtrs)
	{
		pObj = addGameObject(Vec2f(CHARACTER_SIZE, CHARACTER_SIZE), true);
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
