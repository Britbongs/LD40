#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

#include <GameObject\KGameObject.h>
#include <SLU\KStateLogicUnit.h>
#include <Krawler.h>
#include "Collisions\MeshCollider.h"
#include <AssetLoader\KAssetLoader.h>

#include "Animator.h"

#include <SFML\Audio\Sound.hpp>

enum PlayerState
{
	StateIdle,
	StateRunning,
	StateAiming
};

using namespace Krawler;

class PlayerController : public SLU::KGameObjectLogicUnit
{
public:

	PlayerController(KGameObject* pObj, SLU::KStateLogicUnitAdministrator& rAdmin);
	~PlayerController();

	virtual KInitStatus initialiseUnit() override;
	virtual void cleanupUnit() override;
	virtual void tickUnit() override;
	void setMeshCollider(MeshCollider* const pMesh);

private:

	const float MinimumTimeAimeBeforeFire = 0.09f;
	Vec2f getMoveDir();
	void updateAiming();
	void fireProjectile(float angle);

	bool loadAnimations();
	bool loadSounds(); 

	void checkForStateChange(const Vec2f& movVec);
	void changeState(PlayerState nextState);
	void tickAiming();


	const float PlayerMoveSpeed;
	const float MaxRaycastDistance;

	MeshCollider* mp_meshCollider = nullptr;

	Animator* mp_RunAnimator = nullptr;
	Animator* mp_IdleAnimator = nullptr;
	Animator* mp_AimAnimator = nullptr;

	sf::Sound* m_footStepSound;

	bool m_bIsMoving = false;
	bool m_bIsAiming = false;

	bool m_bHasJustFired = false;

	float m_minimumAimTimer = 0.0f;

	PlayerState m_playerState = PlayerState::StateIdle;
};


#endif 
