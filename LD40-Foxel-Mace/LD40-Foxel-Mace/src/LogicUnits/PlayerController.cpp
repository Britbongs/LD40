#include "LogicUnits\PlayerController.h"
#include "LogicUnits\WorldCollisions.h"
#include "LogicUnits\AIBehaviour.h"
#include "GameStates\PlayState.h"

#include <Input\KInput.h>
#include <KApplication.h>

#include <SFML\Audio.hpp>

using namespace Krawler::SLU;
using namespace Krawler::Maths;
using namespace Krawler::Input;

#define PLAYER_ANIM_SIZE 24

PlayerController::PlayerController(KGameObject * pObj, SLU::KStateLogicUnitAdministrator & rAdmin)
	:SLU::KGameObjectLogicUnit(CLASS_NAME_TO_TAG(PlayerController), rAdmin),
	PlayerMoveSpeed(175.0f), MaxRaycastDistance(500), MaxPlayerHealth(8), m_playerHealth(MaxPlayerHealth)
{
	setGameObject(pObj);
}

PlayerController::~PlayerController()
{
}

KInitStatus PlayerController::initialiseUnit()
{
	auto renderer = KApplication::getApp()->getRenderer();
	KCHECK(renderer);
	mp_muzzleFlashObj = getStateAdmin()->getLogicState().addGameObject(Vec2f(32, 48), false);
	KCHECK(mp_muzzleFlashObj);
	mp_muzzleFlashObj->setRenderLayer(6);
	mp_muzzleFlashObj->setOrigin(mp_muzzleFlashObj->getHalfLocalBounds());
	bool result = loadAnimations();
	if (!result)
		return KInitStatus::MissingResource;
	changeState(PlayerState::StateIdle);
	result = loadSounds();

	if (!result)
		return KInitStatus::MissingResource;

	getGameObj()->setRenderLayer(5);

	auto& asset = KAssetLoader::getAssetLoader();
	sf::Font* const pFont = asset.loadFont(KTEXT("seriphim.ttf"));
	sf::Text pText = sf::Text(KTEXT("HP: "), *pFont);
	pText.setCharacterSize(32);
	m_uiIndex = KApplication::getApp()->getRenderer()->addTextToScreen(pText, Vec2i(10, 10));

	return KInitStatus::Success;
}

void PlayerController::cleanupUnit()
{
}

void PlayerController::tickUnit()
{
	auto go = getGameObj();
	auto collider = getStateAdmin()->getStateLogicUnit<WorldCollisions>(KTEXT("WorldCollisions"));

	Vec2f movDir = getMoveDir();
	updateAiming();
	checkForStateChange(movDir);

	if (!mp_MuzzleAnim->isAnimationPlaying())
	{
		mp_muzzleFlashObj->setObjectInactive();
	}
	float deltaTime = KApplication::getApp()->getDeltaTime();
	switch (m_playerState)
	{
	case PlayerState::StateRunning:
	{
		float angle = Degrees(atan2(movDir.y, movDir.x)) - 180.0f;
		checkInsideBounds(movDir);
		go->move(Normalise(movDir) * PlayerMoveSpeed * deltaTime);
		go->setRotation(angle);

		if (m_footStepSound.getStatus() != sf::Sound::Playing && mp_RunAnimator->getCurrentFrame() == 1)
		{
			m_footStepSound.play();
		}
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
	updatePlayerUI();
}

void PlayerController::setMeshCollider(MeshCollider * const pMesh)
{
	mp_meshCollider = pMesh;
}

void PlayerController::takeDamage()
{
	if (m_playerHealth > 0)
		--m_playerHealth;

	m_playerHitSound.play();
}

Vec2f PlayerController::getMoveDir()
{
	if (m_playerState == PlayerState::StateDying || m_playerState == PlayerState::StateDead)
		return Vec2f();

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

	m_railgunSound.play();
	mp_MuzzleAnim->play();
	mp_muzzleFlashObj->setRotation(getGameObj()->getRotation() - 90.0f);
	mp_muzzleFlashObj->setPosition(getGameObj()->getPosition() + Vec2f(0.0f, 24.0f + sinf(mp_muzzleFlashObj->getRotation())));
	mp_muzzleFlashObj->setObjectActive();

	if (raycastResult.bDidCollide)
	{
		SLU::KGameObjectLogicUnit* aiScript = getStateAdmin()->getGameLogicUnitByGameObjectName(raycastResult.collidedWithName);
		AIBehaviour* aiInstance = dynamic_cast<AIBehaviour*> (aiScript);

		aiInstance->setState(AIState::Shot);
		++m_amountKilled;

	}
}

void PlayerController::updatePlayerUI()
{
	std::wstring str = KTEXT("HP: ") + std::to_wstring(m_playerHealth);
	KApplication::getApp()->getRenderer()->getTextByIndex(m_uiIndex).setString(str);
}

bool PlayerController::loadAnimations()
{
	auto& assetLoader = KAssetLoader::getAssetLoader();
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

	{//Die anim
		sf::Texture* pPlayerDieAnim = assetLoader.loadTexture(KTEXT("player_death.png"));

		mp_DieAnimator = new Animator(pPlayerDieAnim, *getStateAdmin());

		mp_DieAnimator->setGameObject(getGameObj());
		mp_DieAnimator->setFrameTime(sf::milliseconds(150).asSeconds());
		mp_DieAnimator->setTileDimension(Vec2i(PLAYER_ANIM_SIZE, PLAYER_ANIM_SIZE));
		mp_DieAnimator->addKeyFrame(sf::IntRect(0, 0, 1, 1));
		mp_DieAnimator->addKeyFrame(sf::IntRect(1, 0, 1, 1));
		mp_DieAnimator->addKeyFrame(sf::IntRect(2, 0, 1, 1));
		mp_DieAnimator->addKeyFrame(sf::IntRect(3, 0, 1, 1));
		mp_DieAnimator->addKeyFrame(sf::IntRect(4, 0, 1, 1));
		mp_DieAnimator->addKeyFrame(sf::IntRect(5, 0, 1, 1));
		mp_DieAnimator->setLooping(false);

		getStateAdmin()->addUnit(mp_DieAnimator);
	}

	{//muzzle flash
		sf::Texture* pMuzzleFlash = assetLoader.loadTexture(KTEXT("muzzle_flash.png"));
		mp_MuzzleAnim = new Animator(pMuzzleFlash, *getStateAdmin());
		mp_MuzzleAnim->setGameObject(mp_muzzleFlashObj);
		mp_MuzzleAnim->setFrameTime(sf::milliseconds(100).asSeconds());
		mp_MuzzleAnim->setTileDimension(Vec2i(16, 24));
		mp_MuzzleAnim->addKeyFrame(sf::IntRect(0, 0, 1, 1));
		mp_MuzzleAnim->addKeyFrame(sf::IntRect(1, 0, 1, 1));
		mp_MuzzleAnim->addKeyFrame(sf::IntRect(2, 0, 1, 1));
		getStateAdmin()->addUnit(mp_MuzzleAnim);
	}

	return true;
}

bool PlayerController::loadSounds()
{
	auto& assetLoader = KAssetLoader::getAssetLoader();
	assetLoader.setRootFolder(KTEXT("res\\"));

	sf::SoundBuffer* footstepBuffer = assetLoader.loadSoundBuffer(KTEXT("footstep.ogg"));
	sf::SoundBuffer* hitSound = assetLoader.loadSoundBuffer(KTEXT("player_hit.ogg"));
	sf::SoundBuffer* railgunBuffer = assetLoader.loadSoundBuffer(KTEXT("railgun.ogg"));
	sf::SoundBuffer* playerDie = assetLoader.loadSoundBuffer(KTEXT("player_death.ogg"));

	if (!footstepBuffer)
		return false;

	if (!railgunBuffer)
		return false;

	if (!hitSound)
		return false;

	if (!playerDie)
		return false;

	m_footStepSound.setBuffer(*footstepBuffer);
	m_playerHitSound.setBuffer(*hitSound);
	m_railgunSound.setBuffer(*railgunBuffer);
	m_playerDeathSound.setBuffer(*playerDie);

	m_railgunSound.setVolume(30);

	return true;
}

void PlayerController::checkForStateChange(const Vec2f & movVec)
{
	if (m_playerState == PlayerState::StateDead)
	{
		return;
	}
	if (m_playerState != PlayerState::StateDying && !isPlayerAlive())
	{
		changeState(PlayerState::StateDying);
		return;
	}
	if (m_playerState != PlayerState::StateDying)
	{
		handlePlayerShootingMechanics(movVec);
	}
	else
	{
		if (!mp_DieAnimator->isAnimationPlaying())
			changeState(PlayerState::StateDead);
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
	case PlayerState::StateDying:
		mp_DieAnimator->setFrame(0, true);
		mp_DieAnimator->play();
		mp_currentAnimator = mp_AimAnimator;
		m_playerDeathSound.play();
		m_playerState = PlayerState::StateDying;
		break;
	case PlayerState::StateDead:
		m_playerState = PlayerState::StateDead;
		break;
	}
}

void PlayerController::handlePlayerShootingMechanics(const Vec2f& movVec)
{
	if (!m_bIsAiming)
	{
		if (movVec == Vec2f(0.0f, 0.0f))
		{
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

				changeState(PlayerState::StateRunning);
			}
		}
	}
	else
	{
		if (m_playerState != PlayerState::StateAiming)
		{
			changeState(PlayerState::StateAiming);
		}
	}
}

void PlayerController::checkInsideBounds(Vec2f & movVec)
{
	Vec2f centrePos = getGameObj()->getCentrePosition();
	const float halfWidth = getGameObj()->getFixedGlobalBounds().width / 2.0f;
	const float halfHeight = getGameObj()->getFixedGlobalBounds().height / 2.0f;

	float dt = KApplication::getApp()->getDeltaTime();
	centrePos += movVec * dt * PlayerMoveSpeed;

	if (centrePos.x + halfWidth > MAP_WIDTH || centrePos.x - halfWidth < 0)
	{
		movVec.x *= -1.0f;
	}

	if (centrePos.y + halfHeight > MAP_HEIGHT || centrePos.y - halfHeight < 0)
	{
		movVec.y *= -1.0f;
	}
}
