#ifndef PLAY_STATE_H
#define PLAY_STATE_H

#include <LogicState\KLogicState.h>
#include "Collisions\MeshCollider.h"
#include <TiledMap\KTiledMap.h>

using namespace Krawler;
using namespace Krawler::LogicState;

#define MAX_AI_COUNT 100
#define AMOUNT_TO_SPAWN 5

class PlayState : public KLogicState
{
public:
	PlayState();
	~PlayState() = default;

	virtual KInitStatus setupState(const KLogicStateInitialiser&  initaliser) override;
	virtual void cleanupState() override;
	virtual void fixedTick() override;
	virtual void tick() override;

private:

	void handlePlayerControls();
	void registerLogicUnits();

	//AI related functionality
	void setupAI();
	void handleAI();
	void spawnAI();


	TiledMap::KTiledMap m_tiledMap;

	KGameObject* mp_playerObj = nullptr;
	KGameObject* mp_backgroundObj = nullptr;

	const float PlayerMoveSpeed;
	const Vec2f PlayerNormal;
	const float MaxRaycastDistance;

	std::vector<MeshCollider*> m_meshColliders;

	int32 m_firstAiIdx = -1;
	int32 m_firstAIMeshIdx = -1;

	float m_aiTimer = 0.0f;
};

#endif