#ifndef AI_BEHAVIOUR_H
#define AI_BEHAVIOUR_H

#include <GameObject\KGameObject.h>
#include <SLU\KStateLogicUnit.h>
#include <Krawler.h>
#include "Collisions\MeshCollider.h"

#include "Animator.h"

using namespace Krawler;

enum AIState
{
	StateRun,
	StateAttack
};

class AIBehaviour : public SLU::KGameObjectLogicUnit
{
public:

	AIBehaviour(KGameObject* pObj, SLU::KStateLogicUnitAdministrator& rAdmin);
	~AIBehaviour();

	virtual KInitStatus initialiseUnit() override;

	virtual void tickUnit() override;
	void setPlayerPointer(KGameObject* pPlayerObj) { mp_playerObj = pPlayerObj; }
	void setMeshCollider(MeshCollider* const pMesh);


private:

	void resetAIState();

	bool loadAnimations();

	const float AIMoveSpeed;
	KGameObject* mp_playerObj;
	MeshCollider* mp_meshCollider;

	Animator* mp_AIRunAnimator = nullptr;
	Animator* mp_EnemyPunch = nullptr;
	Animator* mp_EnemyDieAnimator = nullptr;
	bool m_bWasJustAlive = false;
};

#endif 

