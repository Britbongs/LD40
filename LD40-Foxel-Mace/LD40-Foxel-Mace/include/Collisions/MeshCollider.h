#ifndef MESH_COLLIDER_H
#define MESH_COLLIDER_H

#include "Krawler.h"
#include <GameObject\KGameObject.h>
#include <vector>
#include <SFML\Graphics\Transform.hpp>
using namespace Krawler;

// TODO refactor to become a module of KRAWLER engine after LD40

class MeshCollider
{
public:

	MeshCollider();

	MeshCollider(const std::vector<Vec2f>& VertArray, KGameObject* pObj);

	~MeshCollider();

	void UpdateMeshCollider();

	Vec2f GetNormal(int32 Index) const;

	Vec2f GetTransformedPoint(int32 Index) const;

	int32 GetPointCount() const;

	int32 GetNormalListSize() const;

	void SetPointsList(const std::vector<Vec2f>& VertArray);

	sf::FloatRect getAABB() const { return m_aabb; }

	std::wstring getGameObjectTag() const { return mp_obj->getObjectName(); }

	void resolve(Vec2f mtv);

	bool isGameObjectActive() const { return mp_obj->isGameObjectActive(); }

	Vec2f getCentrePos() const { return mp_obj->getCentrePosition(); }

	bool isMeshActive() const { return mp_obj->isGameObjectActive(); }

private:

	void InitialisePoints(const std::vector<Vec2f>& VertArray);

	void UpdateTransformedPoints(const sf::Transform& ObjectTransform);

	void UpdateNormals();

	//using declarations to differentiate axes from points
	using Point = Vec2f;
	using Normal = Vec2f;

	//Local space coordinates 
	std::vector<Point> m_localPoints;

	//Global space coordinates
	std::vector<Point> m_transformedPoints;

	std::vector<Normal> m_normalList;

	sf::FloatRect m_aabb;

	KGameObject* mp_obj;
};

#endif
