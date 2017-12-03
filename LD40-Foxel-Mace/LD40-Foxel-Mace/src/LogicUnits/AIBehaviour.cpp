#include "LogicUnits\AIBehaviour.h"
#include "KApplication.h"
#include "Collisions\MeshCollider.h"
#include "LogicUnits\WorldCollisions.h"

AIBehaviour::AIBehaviour(KGameObject * pObj, SLU::KStateLogicUnitAdministrator & rAdmin)
	: KGameObjectLogicUnit(CLASS_NAME_TO_TAG(AIBehaviour) + GenerateUUID(), rAdmin),
	AIMoveSpeed(80.0f)
{
	setGameObject(pObj);
}

AIBehaviour::~AIBehaviour()
{
}

void AIBehaviour::tickUnit()
{
	if (!getGameObj()->isGameObjectActive())
	{
		return;
	}
	const float dt = KApplication::getApplicationInstance()->getDeltaTime();
	assert(mp_playerObj);
	auto rObj = getGameObj();
	const Vec2f centre = rObj->getCentrePosition();
	const Vec2f playerCentre = mp_playerObj->getCentrePosition();
	const float angle = atan2f(playerCentre.y - centre.y, playerCentre.x - centre.x);
	const Vec2f movDir(cosf(angle), sinf(angle));

	rObj->setRotation(Maths::Degrees(angle));
	rObj->move(movDir * dt * AIMoveSpeed);

	auto collideCheck = getStateAdmin().getStateLogicUnit<WorldCollisions>();
	assert(collideCheck);
	auto collisionData = collideCheck->CheckCollision(*mp_meshCollider);
	if (collisionData.bDidCollide)
	{
		getGameObj()->move(collisionData.mtv);

	}

}

void AIBehaviour::setMeshCollider(MeshCollider * const pMesh)
{
	mp_meshCollider = pMesh;
}
