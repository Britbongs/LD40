#ifndef COLLISIONS_H
#define COLLISIONS_H

#include "Collisions\MeshCollider.h"

#include <Krawler.h>
#include <SLU\KStateLogicUnit.h>

//did collide & owner
struct CollisionData
{
	bool bDidCollide = false;
};

class WorldCollisions : public SLU::KStateLogicUnit
{
public:

	explicit WorldCollisions(std::vector<MeshCollider*>& objectList, SLU::KStateLogicUnitAdministrator&);

	~WorldCollisions();
	virtual void tickUnit() override {};


	CollisionData CheckCollision(const MeshCollider& Collider);
	bool DidOBBRaycastHit(Vec2f size, float rotation, const Vec2f& startPos, const Vec2f& endPos, MeshCollider* pIgnoreMesh);
private:
	using AABB = sf::FloatRect;

	struct Projection
	{
		double Max = 0.f;
		double Min = 0.f;
	};

	//TODO Implement sat collision here
	void MeshCollisionCheck(const MeshCollider& SATColider);

	Projection GetProjection(const MeshCollider& Collider, const Vec2f& EdgeNormal) const;

	void AABBCollisionCheck(const MeshCollider& mesh);

	std::vector<MeshCollider*> m_collidersToCheck;

	std::vector<MeshCollider*> m_allMeshColliders;

	KGameObject* mp_RaycastObj;
	MeshCollider m_raycastMesh;

};

#endif
