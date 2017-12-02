#include "LogicUnits\WorldCollisions.h"
#include "LogicState\KLogicState.h"

WorldCollisions::WorldCollisions(std::vector<MeshCollider*>& objectList, SLU::KStateLogicUnitAdministrator& rAdmin)
	: KStateLogicUnit(CLASS_NAME_TO_TAG(WorldCollisions), rAdmin)
{
	m_allMeshColliders = objectList;
	mp_RaycastObj = rAdmin.getLogicState().addGameObject(Vec2f(1.0f, 1.0f), true);
}

WorldCollisions::~WorldCollisions()
{
}

CollisionData WorldCollisions::CheckCollision(const MeshCollider& rMesh)
{
	CollisionData Data;
	Data.bDidCollide = false;

	AABBCollisionCheck(rMesh);

	if (m_collidersToCheck.size() != 0)
	{
		MeshCollisionCheck(rMesh);

		if (m_collidersToCheck.size() != 0)
		{
			Data.bDidCollide = true;
		}
	}
	return Data;
}

//Refactor to take data structure
bool WorldCollisions::DidOBBRaycastHit(Vec2f size, float rotation, const Vec2f& startPos, const Vec2f& endPos, MeshCollider* pIgnoreMesh)
{
	(*mp_RaycastObj) = KGameObject(size);

	std::vector<Vec2f> points(4);

	points[0] = Vec2f(0.0f, 0.0f);
	points[1] = Vec2f(size.x, 0.0f);
	points[2] = Vec2f(size.x, size.y);
	points[3] = Vec2f(0.0f, size.y);

	m_raycastMesh = MeshCollider(points, mp_RaycastObj);

	for (int32 i = 0; i < 100; ++i)
	{
		const float t = (float)i / 100.0f;
		Vec2f pos = Maths::Lerp(startPos, endPos, t);
		mp_RaycastObj->setPosition(pos);
		m_raycastMesh.UpdatMeshCollider();

		AABBCollisionCheck(m_raycastMesh);
		auto findPos = std::find(m_collidersToCheck.begin(), m_collidersToCheck.end(), pIgnoreMesh);

		if (findPos != m_collidersToCheck.end())
		{
			m_collidersToCheck.erase(findPos);
		}

		if (m_collidersToCheck.size() != 0)
		{

			MeshCollisionCheck(m_raycastMesh);

			if (m_collidersToCheck.size() != 0)
			{
				return true;
			}
		}

	}
	return false;
}

void WorldCollisions::MeshCollisionCheck(const MeshCollider& meshA)
{
	//TODO catch multiple collision resolution 
	//-----------------------------------------
	auto DoProjectionsOverlap = [=](const Projection& ProjA, const Projection& ProjB)
	{
		return ProjB.Max >= ProjA.Min && ProjB.Min <= ProjA.Max;
	};
	std::vector<MeshCollider*> TempColliderVector = m_collidersToCheck;
	m_collidersToCheck.clear();

	Projection ProjectionA, ProjectionB;
	for (auto ObjectB : TempColliderVector)
	{
		bool EarlyOut = false;
		int32 i = 0;
		while (i < meshA.GetNormalListSize() && !EarlyOut)
		{
			ProjectionA = GetProjection(meshA, meshA.GetNormal(i));
			ProjectionB = GetProjection((*ObjectB), meshA.GetNormal(i));
			if (!DoProjectionsOverlap(ProjectionA, ProjectionB))
			{
				EarlyOut = true;
			}
			++i;
		}

		if (EarlyOut)
			continue;
		i = 0;

		while (i < meshA.GetNormalListSize() && !EarlyOut)
		{
			ProjectionA = GetProjection(meshA, ObjectB->GetNormal(i));
			ProjectionB = GetProjection((*ObjectB), ObjectB->GetNormal(i));
			if (!DoProjectionsOverlap(ProjectionA, ProjectionB))
			{
				EarlyOut = true;
			}
			++i;
		}

		if (EarlyOut)
			continue;

		m_collidersToCheck.push_back(ObjectB);
	}

	return;
}

WorldCollisions::Projection WorldCollisions::GetProjection(const MeshCollider & Collider, const Vec2f & EdgeNormal) const
{
	Projection Proj;

	Proj.Max = Proj.Min = DotProduct(EdgeNormal, Collider.GetTransformedPoint(0));

	for (int32 i = 1; i < Collider.GetPointCount(); ++i)
	{
		double DP = DotProduct(Krawler::Vec2d(EdgeNormal), Vec2d(Collider.GetTransformedPoint(i)));
		Proj.Max = Maths::Max(DP, Proj.Max);
		Proj.Min = Maths::Min(DP, Proj.Min);
	}

	return Proj;
}

void WorldCollisions::AABBCollisionCheck(const MeshCollider& rMesh)
{
	m_collidersToCheck.clear();

	for (auto o : m_allMeshColliders)
	{

		if (o == &rMesh)
		{//Same object, continue 
			continue;
		}


		if (!o->getAABB().intersects(rMesh.getAABB()))
		{//No intersection, continue 
			continue;
		}

		//add it to the list
		m_collidersToCheck.push_back(o);
	}
}

