#include "Collisions/MeshCollider.h"
#include "KApplication.h"

using namespace sf;

MeshCollider::MeshCollider()
	: mp_obj(nullptr)
{

}

MeshCollider::MeshCollider(const std::vector<Vec2f>& VertArray, KGameObject* pObj)
	: mp_obj(pObj)
{
	InitialisePoints(VertArray);
}

MeshCollider::~MeshCollider()
{
	m_transformedPoints.clear();
	m_normalList.clear();
	m_localPoints.clear();
}

void MeshCollider::UpdateMeshCollider()
{
	//Update the transformed points list
	UpdateTransformedPoints(mp_obj->getTransform());

	//Update normal list
	UpdateNormals();
	m_aabb = mp_obj->getAlignedGlobalBounds();
}

Vec2f MeshCollider::GetNormal(int32 Index) const
{
	if (Index >= 0 && Index < (int32)m_normalList.size())
	{
		return m_normalList[Index];
	}
	return Normal(0.f, 0.f);
}

Vec2f MeshCollider::GetTransformedPoint(int32 Index) const
{
	if (Index > 0 && Index < (int32)m_transformedPoints.size())
	{
		return m_transformedPoints[Index];
	}
	return Normal(0.f, 0.f);
}

int32 MeshCollider::GetPointCount() const
{
	return (int32)m_transformedPoints.size();
}

int32 MeshCollider::GetNormalListSize() const
{
	return (int32)m_normalList.size();
}

void MeshCollider::SetPointsList(const std::vector<Vec2f>& VertArray)
{
	InitialisePoints(VertArray);
}

// Private Functions 

void MeshCollider::resolve(Vec2f mtv)
{
	float dt = KApplication::getApplicationInstance()->getDeltaTime();
	mp_obj->move(mtv*dt);
	UpdateMeshCollider();
}

void MeshCollider::InitialisePoints(const std::vector<Vec2f>&  VertArray)
{
	assert((signed)VertArray.size() <= 4);
	m_localPoints.clear();
	m_transformedPoints.clear();
	m_normalList.clear();
	for (auto& Vert : VertArray)
	{
		//Fill the local points vector
		m_localPoints.push_back(Vert);
	}

	//Initialise transformed points to have the same values as the local points until a transform is applied
	m_transformedPoints.resize(m_localPoints.size());

	for (int32 i = 0; i < (int32)m_transformedPoints.size(); ++i)
	{
		m_transformedPoints[i] = m_localPoints[i];
	}

	m_normalList.resize(m_transformedPoints.size());
}

void MeshCollider::UpdateTransformedPoints(const Transform & ObjectTransform)
{
	for (int32 PointIndex = 0; PointIndex < (int32)m_localPoints.size(); ++PointIndex)
	{//Update the transformed points array using the objects transform
		auto playerPos = ObjectTransform.transformPoint(Vec2f(0.0f, 48.0f));
		m_transformedPoints[PointIndex] = ObjectTransform.transformPoint(m_localPoints[PointIndex]);
	}
}

void MeshCollider::UpdateNormals()
{
	//TODO move to maths utility
	auto MakePerpendicular = [](const Normal& Edge)
	{
		return Normal(Edge.y, -Edge.x);
	};

	for (Int32 PointIndex = 0; PointIndex < (Int32)m_transformedPoints.size(); ++PointIndex)
	{
		Point PointA = m_transformedPoints[PointIndex];

		//if the another point exists after our current one, use it as pointb, otherwise use the first point
		int32 NextIndex = PointIndex + 1 < (Int32)m_transformedPoints.size() ? PointIndex + 1 : 0;

		Point PointB = m_transformedPoints[NextIndex];

		Normal Edge = PointA - PointB;

		m_normalList[PointIndex] = MakePerpendicular(Edge);
		m_normalList[PointIndex] = Normalise(m_normalList[PointIndex]);
	}
}
