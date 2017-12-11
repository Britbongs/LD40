#include "LogicUnits\AIBehaviour.h"
#include "KApplication.h"
#include "Collisions\MeshCollider.h"
#include "LogicUnits\WorldCollisions.h"
#include <AssetLoader\KAssetLoader.h>
#include "LogicUnits\PlayerController.h"

#define MINIMUM_ATTACK_DIST 40

AIBehaviour::AIBehaviour(KGameObject * pObj, SLU::KStateLogicUnitAdministrator & rAdmin)
	: KGameObjectLogicUnit(CLASS_NAME_TO_TAG(AIBehaviour) + GenerateUUID(), rAdmin),
	AIMoveSpeed(120.0f), mp_AIRunAnimator(nullptr), m_aiState(AIState::Dead)
{
	setGameObject(pObj);
}

AIBehaviour::~AIBehaviour()
{
	mp_AIRunAnimator = nullptr;
}

KInitStatus AIBehaviour::initialiseUnit()
{
	if (!loadAnimations())
		return KInitStatus::MissingResource;
	return Success;
}

void AIBehaviour::tickUnit()
{
	if (!getGameObj())
	{
		return;
	}
	const float dt = KApplication::getApp()->getDeltaTime();

	assert(mp_playerObj);
	switch (m_aiState)
	{
	case AIState::Shot:
		if (!mp_EnemyDieAnimator->isAnimationPlaying())
		{
			setState(AIState::Dead);
		}
		break;
	case AIState::Run:
	{
		auto rObj = getGameObj();
		const Vec2f centre = rObj->getCentrePosition();
		const Vec2f playerCentre = mp_playerObj->getCentrePosition();
		const float angle = atan2f(playerCentre.y - centre.y, playerCentre.x - centre.x);
		const Vec2f movDir(cosf(angle), sinf(angle));

		const float squareDist = GetSquareLength(playerCentre - centre);

		if (squareDist <= (MINIMUM_ATTACK_DIST * MINIMUM_ATTACK_DIST))
		{
			// change state here
			setState(AIState::Attack);
		}
		Vec2f avoid = getAvoidanceVector(movDir);
		rObj->setRotation(Maths::Degrees(angle) - 180);
		Vec2f vel = movDir*dt *AIMoveSpeed;
		vel += (avoid*dt);
		rObj->move(vel);
	}
	break;

	case AIState::Attack:
	{
		auto rObj = getGameObj();
		const Vec2f centre = rObj->getCentrePosition();
		const Vec2f playerCentre = mp_playerObj->getCentrePosition();
		const float angle = atan2f(playerCentre.y - centre.y, playerCentre.x - centre.x);
		const Vec2f movDir(cosf(angle), sinf(angle));

		const float squareDist = GetSquareLength(playerCentre - centre);
		if (squareDist > MINIMUM_ATTACK_DIST * MINIMUM_ATTACK_DIST)
		{
			// change state here
			setState(AIState::Run);
		}
		rObj->setRotation(Maths::Degrees(angle) - 180);

		auto player = getStateAdmin()->getStateLogicUnit<PlayerController>();
		auto collisionCheck = getStateAdmin()->getStateLogicUnit<WorldCollisions>();

		auto collisionResult = collisionCheck->CheckCollision(*mp_meshCollider);

		if (collisionResult.bDidCollide)
		{
			if (collisionResult.collidedWithName == KTEXT("player"))
			{
				if (!m_bAttackOnCooldown)
				{
					player->takeDamage();
					m_bAttackOnCooldown = true;
				}
				else
				{
					m_attackCooldownTimer += dt;
					if (m_attackCooldownTimer > AttackCooldownTime)
					{
						m_bAttackOnCooldown = false;
						m_attackCooldownTimer = 0.0f;
					}
				}
			}
		}
	}
	break;
	}

}

void AIBehaviour::setMeshCollider(MeshCollider * const pMesh)
{
	mp_meshCollider = pMesh;
}

void AIBehaviour::setState(AIState state)
{
	switch (state)
	{
	case AIState::Run:
		mp_AIRunAnimator->play();
		mp_EnemyPunch->stop();
		mp_EnemyDieAnimator->stop();
		break;
	case AIState::Attack:
		mp_AIRunAnimator->stop();
		mp_EnemyDieAnimator->stop();
		mp_EnemyPunch->play();
		break;
	case AIState::Dead:
		getGameObj()->setObjectInactive();
		break;
	case AIState::Shot:
		mp_AIRunAnimator->stop();
		mp_EnemyPunch->stop();
		mp_EnemyDieAnimator->play();

		break;
	}
	m_aiState = state;

}

void AIBehaviour::resetAIState()
{
	mp_EnemyDieAnimator->setFrame(0, true);
	m_bWasJustAlive = false;
	mp_EnemyDieAnimator->play();
	getGameObj()->setObjectActive();
}

Vec2f AIBehaviour::getAvoidanceVector(const Vec2f& directionVector)
{
	const float MAX_AVOID_FORCE(1e10);
	const float MAX_AHEAD(24);
	struct CircleData
	{
		float radiusSquared = 0.0f;
		Vec2f centre;
	};

	const auto lineIntersectCircle = [](const Vec2f& ahead, const Vec2f& ahead2, const CircleData& circle)->bool
	{
		return (GetSquareLength(circle.centre - ahead) <= circle.radiusSquared) || (GetSquareLength(circle.centre - ahead2) <= circle.radiusSquared);
	};

	auto rAdmin = getStateAdmin();
	auto aiList = rAdmin->getStateLogicUnitsList<AIBehaviour>();

	AIBehaviour* highestPriority = nullptr;
	const Vec2f centrePos = getGameObj()->getCentrePosition();
	float highestPriorityDistSquare = FLT_MAX;
	Vec2f ahead, ahead2;
	const float velLength = GetLength(directionVector * AIMoveSpeed) / AIMoveSpeed;

	ahead = centrePos + (directionVector * velLength);
	ahead2 = centrePos + (directionVector * (velLength / 2.0f));

	for (auto aiLU : aiList)
	{
		assert(aiLU);
		if (aiLU == this)
			continue;

		const Vec2f objCentre = aiLU->getGameObj()->getCentrePosition();


		CircleData circle;

		circle.centre = objCentre;
		circle.radiusSquared = aiLU->getGameObj()->getFixedGlobalBounds().width / 2.0f;
		circle.radiusSquared *= circle.radiusSquared;

		const bool collision = lineIntersectCircle(ahead, ahead2, circle);
		const float distSquare = GetSquareLength(objCentre - centrePos);
		if (collision && (!highestPriority || distSquare < highestPriorityDistSquare))
		{
			highestPriority = aiLU;
			highestPriorityDistSquare = distSquare;
		}
	}
	if (!highestPriority)
	{
		return Vec2f();
	}

	Vec2f avoid;
	avoid = ahead - highestPriority->getGameObj()->getCentrePosition();
	avoid = Normalise(avoid);

	avoid *= AIMoveSpeed;
	return avoid;
}

bool AIBehaviour::loadAnimations()
{
	auto assetLoader = KAssetLoader::getAssetLoader();
	assetLoader.setRootFolder(KTEXT("res\\"));
	sf::Texture* pRunTexture = assetLoader.loadTexture(KTEXT("enemy_run.png"));
	sf::Texture* pDieTexture = assetLoader.loadTexture(KTEXT("enemy_explode.png"));
	sf::Texture* pAttackTexture = assetLoader.loadTexture(KTEXT("enemy_punch.png"));

	if (!pRunTexture)
	{
		return false;
	}
	auto rAdmin = getStateAdmin();

	mp_AIRunAnimator = new Animator(pRunTexture, *rAdmin);
	mp_AIRunAnimator->setGameObject(getGameObj());
	mp_AIRunAnimator->setFrameTime(150.0f / 1000.0f);
	mp_AIRunAnimator->setTileDimension(Vec2i(24, 24));

	mp_AIRunAnimator->addKeyFrame(sf::IntRect(0, 0, 1, 1));
	mp_AIRunAnimator->addKeyFrame(sf::IntRect(1, 0, 1, 1));
	mp_AIRunAnimator->addKeyFrame(sf::IntRect(2, 0, 1, 1));
	mp_AIRunAnimator->addKeyFrame(sf::IntRect(3, 0, 1, 1));
	mp_AIRunAnimator->setLooping(true);
	//mp_AIRunAnimator->stop();
	rAdmin->addUnit(mp_AIRunAnimator);

	mp_EnemyDieAnimator = new Animator(pDieTexture, *rAdmin);
	mp_EnemyDieAnimator->setGameObject(getGameObj());
	mp_EnemyDieAnimator->setFrameTime(sf::milliseconds(80).asSeconds());
	mp_EnemyDieAnimator->setTileDimension(Vec2i(24, 24));

	mp_EnemyDieAnimator->addKeyFrame(sf::IntRect(0, 0, 1, 1));
	mp_EnemyDieAnimator->addKeyFrame(sf::IntRect(1, 0, 1, 1));
	mp_EnemyDieAnimator->addKeyFrame(sf::IntRect(2, 0, 1, 1));
	mp_EnemyDieAnimator->addKeyFrame(sf::IntRect(3, 0, 1, 1));
	mp_EnemyDieAnimator->addKeyFrame(sf::IntRect(4, 0, 1, 1));
	mp_EnemyDieAnimator->addKeyFrame(sf::IntRect(5, 0, 1, 1));
	mp_EnemyDieAnimator->setLooping(false);
	//mp_AIRunAnimator->stop();
	rAdmin->addUnit(mp_EnemyDieAnimator);


	mp_EnemyPunch = new Animator(pAttackTexture, *rAdmin);
	mp_EnemyPunch->setGameObject(getGameObj());
	mp_EnemyPunch->setFrameTime(sf::milliseconds(100).asSeconds());
	mp_EnemyPunch->setTileDimension(Vec2i(24, 24));

	mp_EnemyPunch->addKeyFrame(sf::IntRect(0, 0, 1, 1));
	mp_EnemyPunch->addKeyFrame(sf::IntRect(1, 0, 1, 1));
	mp_EnemyPunch->addKeyFrame(sf::IntRect(2, 0, 1, 1));
	mp_EnemyPunch->addKeyFrame(sf::IntRect(3, 0, 1, 1));
	mp_EnemyPunch->setLooping(true);
	//mp_AIRunAnimator->stop();
	rAdmin->addUnit(mp_EnemyPunch);

	return true;

}
