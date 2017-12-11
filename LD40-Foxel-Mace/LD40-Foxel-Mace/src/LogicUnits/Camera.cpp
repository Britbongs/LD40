#include "LogicUnits\Camera.h"

#include <KApplication.h>

using namespace Krawler;

Camera::Camera(Krawler::SLU::KStateLogicUnitAdministrator& rAdmin, KGameObject* pObj, const Vec2f& bounds)
	: KGameObjectLogicUnit(CLASS_NAME_TO_TAG(Camera), rAdmin), m_mapBounds(bounds)
{
	setGameObject(pObj);
}

void Camera::tickUnit()
{
	sf::View view = KApplication::getApp()->getRenderWindow()->getView();
	auto obj = getGameObj();
	view.setCenter(obj->getCentrePosition());

	const Vec2f centre = view.getCenter();
	const Vec2f halfSize = view.getSize() * 0.5f;

	const float farLeft = centre.x - halfSize.x;
	const float farRight = centre.x + halfSize.x;
	const float farTop = centre.y - halfSize.y;
	const float farBottom = centre.y + halfSize.y;

	bool isOnLeftBoundry = farLeft < 0.0f;
	bool isOnRightBoundry = farRight > m_mapBounds.x;
	bool isOnUpBoundry = farTop < 0.0f;
	bool isOnDownBoundry = farBottom > m_mapBounds.y;

	Vec2f adjusted(centre);

	if (isOnLeftBoundry)
	{
		adjusted.x = halfSize.x;
	}

	if (isOnRightBoundry)
	{
		adjusted.x = m_mapBounds.x - halfSize.x;
	}

	if (isOnUpBoundry)
	{
		adjusted.y = halfSize.y;
	}

	if (isOnDownBoundry)
	{
		adjusted.y = m_mapBounds.y - halfSize.y;
	}
	view.setCenter(adjusted);
	KApplication::getApp()->getRenderWindow()->setView(view);
}

