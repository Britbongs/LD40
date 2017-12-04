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

CollisionData WorldCollisions::CheckCollision(MeshCollider& rMesh)
{
	CollisionData Data;
	Data.bDidCollide = false;

	AABBCollisionCheck(rMesh);

	if (m_collidersToCheck.size() != 0)
	{
		Vec2f mtv;
		MeshCollisionCheck(rMesh, mtv);

		if (m_collidersToCheck.size() != 0)
		{
			Data.bDidCollide = true;
			Data.collidedWithName = m_collidersToCheck[0]->getGameObjectTag();
			Data.mtv = mtv;
		}
	}
	return Data;
}

//Refactor to take data structure
CollisionData WorldCollisions::DidOBBRaycastHit(Vec2f size, float rotation, const Vec2f& startPos, const Vec2f& endPos, MeshCollider* pIgnoreMesh)
{
	CollisionData data;
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
		m_raycastMesh.UpdateMeshCollider();

		AABBCollisionCheck(m_raycastMesh);
		auto findPos = std::find(m_collidersToCheck.begin(), m_collidersToCheck.end(), pIgnoreMesh);

		if (findPos != m_collidersToCheck.end())
		{
			m_collidersToCheck.erase(findPos);
		}

		if (m_collidersToCheck.size() != 0)
		{
			Vec2f mtv;
			MeshCollisionCheck(m_raycastMesh, mtv);

			if (m_collidersToCheck.size() != 0)
			{
				data.bDidCollide = true;
				data.collidedWithName = m_collidersToCheck[0]->getGameObjectTag();
				break;
			}
		}

	}


	return data;
}

void WorldCollisions::MeshCollisionCheck(MeshCollider& meshA, Vec2f& mtv)
{
	//TODO catch multiple collision resolution 
	//-----------------------------------------
	auto DoProjectionsOverlap = [=](const Projection& ProjA, const Projection& ProjB)
	{
		return ProjB.Max >= ProjA.Min && ProjB.Min <= ProjA.Max;
	};
	std::vector<MeshCollider*> TempColliderVector = m_collidersToCheck;
	m_collidersToCheck.clear();
	Vec2f smallestAxis;
	double overlap = DBL_MAX;

	Projection ProjectionA, ProjectionB;
	for (auto meshB : TempColliderVector)
	{
		if (&meshA == meshB)
		{
			return;
		}
		bool bEarlyOut = false;
		int32 i = 0;
		while (i < meshA.GetNormalListSize() && !bEarlyOut)
		{
			ProjectionA = GetProjection(meshA, meshA.GetNormal(i));
			ProjectionB = GetProjection((*meshB), meshA.GetNormal(i));
			if (ProjectionA == Projection(0.0, 0.0) && ProjectionB == Projection(0.0, 0.0))
			{
				++i;

				continue;
			}
			if (!DoProjectionsOverlap(ProjectionA, ProjectionB))
			{
				bEarlyOut = true;
			}
			else
			{
				const double o = (ProjectionA.Max - ProjectionB.Max) + (ProjectionA.Min - ProjectionB.Min);
				if (o < overlap)
				{
					smallestAxis = meshB->GetNormal(i);
					overlap = o;
				}
			}
			++i;
		}

		if (bEarlyOut)
			continue;
		i = 0;

		while (i < meshA.GetNormalListSize() && !bEarlyOut)
		{
			ProjectionA = GetProjection(meshA, meshB->GetNormal(i));
			ProjectionB = GetProjection((*meshB), meshB->GetNormal(i));
			if (ProjectionA == Projection(0.0, 0.0) && ProjectionB == Projection(0.0, 0.0))
			{
				++i;

				continue;
			}
			if (!DoProjectionsOverlap(ProjectionA, ProjectionB))
			{
				bEarlyOut = true;
			}
			else
			{
				const double o = (ProjectionA.Max - ProjectionB.Max) + (ProjectionA.Min - ProjectionB.Min);

				if (o < overlap)
				{
					smallestAxis = meshB->GetNormal(i);
					overlap = o;
				}
			}
			++i;
		}

		if (bEarlyOut)
			continue;

		m_collidersToCheck.push_back(meshB);
		mtv = -smallestAxis * ((float)overlap);
		meshA.resolve(-mtv);
		meshB->resolve(mtv);
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
		if (!o->isGameObjectActive())
		{
			continue;
		}

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

