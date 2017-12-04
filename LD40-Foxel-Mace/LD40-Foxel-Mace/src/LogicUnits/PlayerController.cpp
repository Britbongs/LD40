#include "LogicUnits\PlayerController.h"
#include <Input\KInput.h>
#include "LogicUnits\WorldCollisions.h"
#include <KApplication.h>

#include <SFML\Audio.hpp>

using namespace Krawler::SLU;
using namespace Krawler::Maths;
using namespace Krawler::Input;

#define PLAYER_ANIM_SIZE 24

PlayerController::PlayerController(KGameObject * pObj, SLU::KStateLogicUnitAdministrator & rAdmin)
	:SLU::KGameObjectLogicUnit(CLASS_NAME_TO_TAG(PlayerController), rAdmin),
	PlayerMoveSpeed(250.0f), MaxRaycastDistance(180)
{
	setGameObject(pObj);
}

PlayerController::~PlayerController()
{
}

KInitStatus PlayerController::initialiseUnit()
{
	bool result = loadAnimations();
	if (!result)
		return KInitStatus::MissingResource;
	changeState(PlayerState::StateIdle);
	result = loadSounds();

	if (!result)
		return KInitStatus::MissingResource;

	return KInitStatus::Success;
}

void PlayerController::cleanupUnit()
{
	KFREE(m_footStepSound);
}

void PlayerController::tickUnit()
{
	auto go = getGameObj();
	auto collider = getStateAdmin()->getStateLogicUnit<WorldCollisions>(KTEXT("WorldCollisions"));

	Vec2f movDir = getMoveDir();
	updateAiming();
	checkForStateChange(movDir);

	float deltaTime = KApplication::getApplicationInstance()->getDeltaTime();
	switch (m_playerState)
	{
	case PlayerState::StateRunning:
	{
		go->move(Normalise(movDir) * PlayerMoveSpeed * deltaTime);
		float angle = Degrees(atan2(movDir.y, movDir.x)) - 180.0f;
		go->setRotation(angle);
	}
	break;

	case PlayerState::StateAiming:
	{//rotation and shooting
		const Vec2f mousePos = KInput::GetMouseWorldPosition();
		const Vec2f playerCentre = go->getCentrePosition();
		float angle = atan2(mousePos.y - playerCentre.y, mousePos.x - playerCentre.x);
		if (m_minimumAimTimer < MinimumTimeAimeBeforeFire)
		{
			m_minimumAimTimer += deltaTime;
		}
		else
		{
			if (KInput::MouseJustPressed(sf::Mouse::Left))
				fireProjectile(Degrees(angle));
		}
		go->setRotation(Maths::Degrees(angle) - 180.0f);

	}
	default:
	case PlayerState::StateIdle:
		break;
	}
}

void PlayerController::setMeshCollider(MeshCollider * const pMesh)
{
	mp_meshCollider = pMesh;
}

Vec2f PlayerController::getMoveDir()
{
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
	return movDir;
}

void PlayerController::updateAiming()
{
	if (KInput::MousePressed(sf::Mouse::Right))
	{
		m_bIsAiming = true;
	}
	else
	{
		m_bIsAiming = false;
	}
}

void PlayerController::fireProjectile(float angle)
{
	auto collider = getStateAdmin()->getStateLogicUnit<WorldCollisions>(KTEXT("WorldCollisions"));

	const Vec2f mousePos = KInput::GetMouseWorldPosition();
	const Vec2f playerCentre = getGameObj()->getCentrePosition();

	const Vec2f dir(cosf(Radians(angle)), sinf(Radians(angle)));
	const Vec2f size(16.0f, 16.0f);
	CollisionData raycastResult = collider->DidOBBRaycastHit(size, angle, playerCentre, playerCentre + (dir * MaxRaycastDistance), mp_meshCollider);

	if (raycastResult.bDidCollide)
	{
		auto playState = getStateAdmin()->getLogicState();
		auto enemyObj = playState.getGameObjectByName(raycastResult.collidedWithName);
		if (enemyObj)
		{
			enemyObj->setObjectInactive();
		}
	}

}

bool PlayerController::loadAnimations()
{
	auto assetLoader = KAssetLoader::getAssetLoader();
	assetLoader.setRootFolder(KTEXT("res\\"));

	{//running anim load
		sf::Texture* pPlayerRunAnim = assetLoader.loadTexture(KTEXT("player_run.png"));

		mp_RunAnimator = new Animator(pPlayerRunAnim, *getStateAdmin());

		mp_RunAnimator->setGameObject(getGameObj());
		mp_RunAnimator->setFrameTime(150.0f / 1000.0f);
		mp_RunAnimator->setTileDimension(Vec2i(PLAYER_ANIM_SIZE, PLAYER_ANIM_SIZE));
		mp_RunAnimator->addKeyFrame(sf::IntRect(0, 0, 1, 1));
		mp_RunAnimator->addKeyFrame(sf::IntRect(1, 0, 1, 1));
		mp_RunAnimator->addKeyFrame(sf::IntRect(2, 0, 1, 1));
		mp_RunAnimator->addKeyFrame(sf::IntRect(3, 0, 1, 1));
		mp_RunAnimator->setLooping(true);

		getStateAdmin()->addUnit(mp_RunAnimator);
	}

	{//idle anim
		sf::Texture* pPlayerIdleAnim = assetLoader.loadTexture(KTEXT("player_idle.png"));

		mp_IdleAnimator = new Animator(pPlayerIdleAnim, *getStateAdmin());

		mp_IdleAnimator->setGameObject(getGameObj());
		mp_IdleAnimator->setFrameTime(150.0f / 1000.0f);
		mp_IdleAnimator->setTileDimension(Vec2i(PLAYER_ANIM_SIZE, PLAYER_ANIM_SIZE));
		mp_IdleAnimator->addKeyFrame(sf::IntRect(0, 0, 1, 1));
		mp_IdleAnimator->setLooping(true);

		getStateAdmin()->addUnit(mp_IdleAnimator);
	}


	{//Aim anim
		sf::Texture* pPlayerAimAnim = assetLoader.loadTexture(KTEXT("player_aim.png"));

		mp_AimAnimator = new Animator(pPlayerAimAnim, *getStateAdmin());

		mp_AimAnimator->setGameObject(getGameObj());
		mp_AimAnimator->setFrameTime(150.0f / 1000.0f);
		mp_AimAnimator->setTileDimension(Vec2i(PLAYER_ANIM_SIZE, PLAYER_ANIM_SIZE));
		mp_AimAnimator->addKeyFrame(sf::IntRect(0, 0, 1, 1));
		mp_AimAnimator->setLooping(true);

		getStateAdmin()->addUnit(mp_AimAnimator);
	}
	return true;
}

bool PlayerController::loadSounds()
{
	auto assetLoader = KAssetLoader::getAssetLoader();
	assetLoader.setRootFolder(KTEXT("res\\"));
	sf::SoundBuffer* footstepBuffer = assetLoader.loadSoundBuffer(KTEXT("footstep.wav"));

	if (!footstepBuffer)
		return false;

	m_footStepSound = new sf::Sound(*footstepBuffer);
	m_footStepSound->setLoop(true);
	m_footStepSound->setVolume(25);
	return true;
}

void PlayerController::checkForStateChange(const Vec2f & movVec)
{
	if (!m_bIsAiming)
	{
		if (movVec == Vec2f(0.0f, 0.0f))
		{
			//m_footStepSound.pause();
			m_bIsMoving = false;
			if (m_playerState != PlayerState::StateIdle)
			{
				changeState(PlayerState::StateIdle);
			}
		}
		else
		{
			m_bIsMoving = true;
			if (m_playerState != PlayerState::StateRunning)
			{
				if (m_footStepSound->getStatus() != sf::Sound::Playing)
				{
				}
				changeState(PlayerState::StateRunning);
			}
		}
	}
	else
	{
		//m_footStepSound.pause();

		if (m_playerState != PlayerState::StateAiming)
		{
			changeState(PlayerState::StateAiming);
		}
	}
}

void PlayerController::changeState(PlayerState nextState)
{

	static Animator* mp_currentAnimator = nullptr;
	if (mp_currentAnimator)
	{
		mp_currentAnimator->stop();
	}

	switch (nextState)
	{
	case PlayerState::StateIdle:
		mp_IdleAnimator->setFrame(0, true);
		mp_IdleAnimator->play();
		mp_currentAnimator = mp_IdleAnimator;
		m_playerState = PlayerState::StateIdle;
		break;
	case PlayerState::StateRunning:
		mp_RunAnimator->setFrame(0, true);
		mp_RunAnimator->play();
		mp_currentAnimator = mp_RunAnimator;
		m_playerState = PlayerState::StateRunning;
		break;
	case PlayerState::StateAiming:
		mp_AimAnimator->setFrame(0, true);
		mp_AimAnimator->play();
		mp_currentAnimator = mp_AimAnimator;
		m_playerState = PlayerState::StateAiming;
		break;
	}
}
