#ifndef AI_BEHAVIOUR_H
#define AI_BEHAVIOUR_H

#include <GameObject\KGameObject.h>
#include <SLU\KStateLogicUnit.h>
#include <Krawler.h>
#include "Collisions\MeshCollider.h"

using namespace Krawler;

class AIBehaviour : public SLU::KGameObjectLogicUnit
{
public:

	AIBehaviour(KGameObject* pObj, SLU::KStateLogicUnitAdministrator& rAdmin);
	~AIBehaviour();

	virtual void tickUnit() override;
	void setPlayerPointer(KGameObject* pPlayerObj) { mp_playerObj = pPlayerObj; }
	void setMeshCollider(MeshCollider* const pMesh);
	
private:
	const float AIMoveSpeed;
	KGameObject* mp_playerObj;
	MeshCollider* mp_meshCollider;
};

#endif 

