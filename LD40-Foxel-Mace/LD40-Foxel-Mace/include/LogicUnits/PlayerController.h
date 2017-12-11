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
	StateAiming,
	StateDying,
	StateDead
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
	void takeDamage();
	PlayerState getPlayerState() const { return m_playerState; }
	int32 getAmountKilled() const { return m_amountKilled; }

private:

	bool isPlayerAlive() const { return m_playerHealth > 0; }

	const float MinimumTimeAimeBeforeFire = 0.09f;

	Vec2f getMoveDir();
	void updateAiming();
	void fireProjectile(float angle);
	void updatePlayerUI();

	bool loadAnimations();
	bool loadSounds();

	void checkForStateChange(const Vec2f& movVec);
	void changeState(PlayerState nextState);
	void handlePlayerShootingMechanics(const Vec2f& movVec);
	void checkInsideBounds(Vec2f& movVec);

	const float PlayerMoveSpeed;
	const float MaxRaycastDistance;

	const uint32 MaxPlayerHealth;

	MeshCollider* mp_meshCollider = nullptr;

	KGameObject* mp_muzzleFlashObj = nullptr;

	Animator* mp_RunAnimator = nullptr;
	Animator* mp_IdleAnimator = nullptr;
	Animator* mp_AimAnimator = nullptr;
	Animator* mp_DieAnimator = nullptr;
	Animator* mp_MuzzleAnim = nullptr;

	sf::Sound* mp_footStepSound;
	sf::Sound* mp_railgunSound;

	bool m_bIsMoving = false;
	bool m_bIsAiming = false;

	bool m_bHasJustFired = false;

	float m_minimumAimTimer = 0.0f;

	PlayerState m_playerState = PlayerState::StateIdle;

	uint32 m_playerHealth;
	int32 m_uiIndex = 0;
	uint32 m_amountKilled = 0;
};

#endif 
