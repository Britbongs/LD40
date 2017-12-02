#ifndef CAMERA_H
#define CAMERA_H

#include <Krawler.h>
#include <SLU\KStateLogicUnit.h>
#include <GameObject\KGameObject.h>

using namespace Krawler;

class Camera : public Krawler::SLU::KGameObjectLogicUnit
{

public:

	Camera(SLU::KStateLogicUnitAdministrator& rAdmin, KGameObject* pObj, const Vec2f& bounds);
	~Camera() = default;

	virtual void tickUnit();

private:

	const Vec2f m_mapBounds;
};

#endif