#ifndef PLAY_STATE_H
#define PLAY_STATE_H

#include <LogicState\KLogicState.h>
#include "Collisions\MeshCollider.h"

using namespace Krawler;
using namespace Krawler::LogicState;

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

	KGameObject* mp_playerObj = nullptr;
	KGameObject* mp_backgroundObj = nullptr; 

	const float PlayerMoveSpeed;
	const Vec2f PlayerNormal;
	const float MaxRaycastDistance;
	MeshCollider m_meshArr[2];


};

#endif