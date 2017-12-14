#ifndef AI_BEHAVIOUR_H
#define AI_BEHAVIOUR_H

#include <GameObject\KGameObject.h>
#include <SLU\KStateLogicUnit.h>
#include <Krawler.h>
#include "Collisions\MeshCollider.h"
#include <SFML\Audio.hpp>

#include "Animator.h"

using namespace Krawler;

enum AIState
{
	Run,
	Attack,
	Shot,
	Dead
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

	void setState(AIState state);

private:

	const float AttackCooldownTime = sf::milliseconds(2000).asSeconds();
	const float AIMoveSpeed;

	void resetAIState();
	Vec2f getAvoidanceVector(const Vec2f& directionVector);
	bool loadAnimations();

	KGameObject* mp_playerObj;
	MeshCollider* mp_meshCollider;

	Animator* mp_AIRunAnimator = nullptr;
	Animator* mp_EnemyPunch = nullptr;
	Animator* mp_EnemyDieAnimator = nullptr;
	bool m_bWasJustAlive = false;

	AIState m_aiState;

	sf::Sound m_dieSound; 

	float m_attackCooldownTimer = 0.0f;
	bool m_bAttackOnCooldown = false;
};

#endif 

