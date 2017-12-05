#include "LogicUnits\WorldCollisions.h"
#include "LogicState\KLogicState.h"

#include "Maths\KMaths.hpp"

WorldCollisions::WorldCollisions(std::vector<MeshCollider*>& objectList, SLU::KStateLogicUnitAdministrator& rAdmin)
	: KStateLogicUnit(CLASS_NAME_TO_TAG(WorldCollisions), rAdmin)
{
	m_allMeshColliders = objectList;
	mp_RaycastObj = rAdmin.getLogicState().addGameObject(Vec2f(1.0f, 1.0f), false);
}

WorldCollisions::~WorldCollisions()
{
}

CollisionData WorldCollisions::CheckCollision(MeshCollider& rMesh, bool resolveCollisions)
{
	CollisionData Data;
	Data.bDidCollide = false;

	AABBCollisionCheck(rMesh);

	if (m_collidersToCheck.size() != 0)
	{
		Vec2f mtv;
		MeshCollisionCheck(rMesh, mtv, resolveCollisions);

		if (m_collidersToCheck.size() != 0)
		{
			Data.bDidCollide = true;
			Data.collidedWithName = m_collidersToCheck[0]->getGameObjectTag();
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
	const float STEP_MAX = 200;
	for (int32 i = 0; i < STEP_MAX; ++i)
	{
		const float t = (float)i / STEP_MAX;
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
			MeshCollisionCheck(m_raycastMesh, mtv, false);

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

void WorldCollisions::MeshCollisionCheck(MeshCollider& meshA, Vec2f& mtv, bool bResolveCollisions)
{
	//TODO catch multiple collision resolution 
	//-----------------------------------------
	auto DoProjectionsOverlap = [](const Projection& ProjA, const Projection& ProjB)
	{
		return ProjB.Max >= ProjA.Min && ProjB.Min <= ProjA.Max;
	};
	auto DoesContain = [](const Projection& ProjA, const Projection& ProjB)
	{
		return ProjB.Min >= ProjA.Min && ProjB.Max <= ProjA.Max;
	};
	auto GetOverlapAmount = [](const Projection& ProjA, const Projection& ProjB)-> double
	{
		return Krawler::Maths::Max(0.0, Krawler::Maths::Min(ProjA.Max, ProjB.Max) - Krawler::Maths::Max(ProjA.Min, ProjB.Min));
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
		while (i < meshA.GetNormalListSize())
		{
			ProjectionA = GetProjection(meshA, meshA.GetNormal(i));
			ProjectionB = GetProjection((*meshB), meshA.GetNormal(i));
			if (!DoProjectionsOverlap(ProjectionB, ProjectionA))
			{
				bEarlyOut = true;
				break;
			}
			else
			{
				double o = GetOverlapAmount(ProjectionB, ProjectionA);

				if (DoesContain(ProjectionA, ProjectionB) || DoesContain(ProjectionB, ProjectionA))
				{
					double mins = fabs(ProjectionA.Min - ProjectionB.Min);
					double maxs = fabs(ProjectionA.Max - ProjectionB.Max);

					if (mins < maxs)
					{
						o += mins;
					}
					else
					{
						o += maxs;
					}
				}
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

		while (i < meshA.GetNormalListSize())
		{
			ProjectionA = GetProjection(meshA, meshB->GetNormal(i));
			ProjectionB = GetProjection((*meshB), meshB->GetNormal(i));
			if (!DoProjectionsOverlap(ProjectionA, ProjectionB))
			{
				bEarlyOut = true;
				break;
			}
			else
			{
				double o = GetOverlapAmount(ProjectionA, ProjectionB);
				if (DoesContain(ProjectionA, ProjectionB) || DoesContain(ProjectionB, ProjectionA))
				{
					double mins = fabs(ProjectionB.Min - ProjectionA.Min);
					double maxs = fabs(ProjectionB.Max - ProjectionA.Max);
					if (mins < maxs)
					{
						o += mins;
					}
					else
					{
						o += maxs;
					}
				}
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

		/*
		//Calculate slope of axis
		double angle = atan(axis.y / axis.x);

		//Calculate change along x and y axes
		sf::Vector2f result(cos(angle)*magnitude, sin(angle)*magnitude);

		//Check to make sure MTV isn't pointing towards a polygon
		if (result.x*directionVector.x + result.y * directionVector.y < 0)
		{
		result.x = -result.x;
		result.y = -result.y;
		}*/

		float angle = atan(smallestAxis.y / smallestAxis.x);
		Vec2f direc = meshA.getCentrePos() - meshB->getCentrePos();
		mtv = -smallestAxis * ((float)overlap);

		if (DotProduct(mtv, direc) < 0.0f)
		{
			mtv = -mtv;
		}
		m_collidersToCheck.push_back(meshB);
		if (bResolveCollisions)
		{
			meshA.resolve(-mtv);
			meshB->resolve(mtv);
		}
		//KPrintf(L"Overlap: %f\n", (float)overlap);
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

