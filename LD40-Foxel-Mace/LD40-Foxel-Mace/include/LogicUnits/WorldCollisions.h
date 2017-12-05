#ifndef COLLISIONS_H
#define COLLISIONS_H

#include "Collisions\MeshCollider.h"

#include <Krawler.h>
#include <SLU\KStateLogicUnit.h>

//did collide & owner
struct CollisionData
{
	bool bDidCollide = false;
	std::wstring collidedWithName;
};

class WorldCollisions : public SLU::KStateLogicUnit
{
public:

	explicit WorldCollisions(std::vector<MeshCollider*>& objectList, SLU::KStateLogicUnitAdministrator&);

	~WorldCollisions();
	virtual void tickUnit() override {};


	CollisionData CheckCollision(MeshCollider& Collider, bool resolveCollisions = false);
	CollisionData DidOBBRaycastHit(Vec2f size, float rotation, const Vec2f& startPos, const Vec2f& endPos, MeshCollider* pIgnoreMesh);
private:
	using AABB = sf::FloatRect;

	struct Projection
	{
		double Max;
		double Min;

		Projection()
			:Max(DBL_MAX), Min(DBL_MIN)
		{

		}
		Projection(double max, double min)
			: Max(max), Min(min)
		{

		}

		bool operator ==(const Projection& rhs)
		{
			return Max == rhs.Max && Min == rhs.Min;
		}
	};

	//TODO Implement sat collision here
	void MeshCollisionCheck(MeshCollider& SATColider, Vec2f& mtv, bool bCheckCollisions);

	Projection GetProjection(const MeshCollider& Collider, const Vec2f& EdgeNormal) const;

	void AABBCollisionCheck(const MeshCollider& mesh);

	std::vector<MeshCollider*> m_collidersToCheck;

	std::vector<MeshCollider*> m_allMeshColliders;

	KGameObject* mp_RaycastObj;
	MeshCollider m_raycastMesh;

};

#endif
