#include "LogicUnits\AIBehaviour.h"
#include "KApplication.h"
#include "Collisions\MeshCollider.h"
#include "LogicUnits\WorldCollisions.h"
#include <AssetLoader\KAssetLoader.h>

#define MINIMUM_ATTACK_DIST 10

AIBehaviour::AIBehaviour(KGameObject * pObj, SLU::KStateLogicUnitAdministrator & rAdmin)
	: KGameObjectLogicUnit(CLASS_NAME_TO_TAG(AIBehaviour) + GenerateUUID(), rAdmin),
	AIMoveSpeed(40.0f), mp_AIRunAnimator(nullptr)
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
	if (!getGameObj()->isGameObjectActive())
	{
		if (m_bWasJustAlive)
		{
			if (!mp_EnemyDieAnimator->isAnimationPlaying() && m_bWasJustAlive)
				resetAIState();
			else
				m_bWasJustAlive = false;
		}
		return;
	}

	if (!m_bWasJustAlive)
	{
		m_bWasJustAlive = true;
	}

	if (!mp_AIRunAnimator->isAnimationPlaying())
		mp_AIRunAnimator->play();
	//mp_EnemyDieAnimator->play();
	const float dt = KApplication::getApplicationInstance()->getDeltaTime();

	assert(mp_playerObj);

	auto rObj = getGameObj();
	const Vec2f centre = rObj->getCentrePosition();
	const Vec2f playerCentre = mp_playerObj->getCentrePosition();
	const float angle = atan2f(playerCentre.y - centre.y, playerCentre.x - centre.x);
	const Vec2f movDir(cosf(angle), sinf(angle));

	const float squareDist = GetSquareLength(playerCentre - centre);

	if (squareDist < MINIMUM_ATTACK_DIST)
	{
		// change state here
	}

	rObj->setRotation(Maths::Degrees(angle)-180);
	rObj->move(movDir * dt * AIMoveSpeed);

	auto collideCheck = getStateAdmin()->getStateLogicUnit<WorldCollisions>();
	assert(collideCheck);
	auto collisionData = collideCheck->CheckCollision(*mp_meshCollider);

}

void AIBehaviour::setMeshCollider(MeshCollider * const pMesh)
{
	mp_meshCollider = pMesh;
}

void AIBehaviour::resetAIState()
{
	mp_EnemyDieAnimator->setFrame(0, true);
	mp_EnemyDieAnimator->play();
}

bool AIBehaviour::loadAnimations()
{
	auto assetLoader = KAssetLoader::getAssetLoader();
	assetLoader.setRootFolder(KTEXT("res\\"));
	sf::Texture* pRunTexture = assetLoader.loadTexture(KTEXT("enemy_run.png"));
	sf::Texture* pDieTexture = assetLoader.loadTexture(KTEXT("enemy_explode.png"));
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
	mp_EnemyDieAnimator->setFrameTime(100.0f / 1000.0f);
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


	return true;

}
