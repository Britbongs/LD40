#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <Krawler.h>
#include <GameObject\KGameObject.h>
#include <LogicState\KLogicState.h>

using namespace Krawler;

class Animator : public SLU::KGameObjectLogicUnit
{

public:

	Animator(sf::Texture* pSpritesheet, SLU::KStateLogicUnitAdministrator& rAdmin);
	~Animator();

	void addKeyFrame(sf::IntRect keyFrame);
	void play();
	void pause();
	void stop();

	bool isAnimationPlaying()const { return m_bIsPlaying; }

	void setFrame(uint32 frameIdx, bool resetAnim = false);
	//set how long each frame executes for in seconds
	void setFrameTime(float frameTime);

	void setTileDimension(Vec2i tileDim) { m_tileDim = tileDim; }

	bool isLooping() const { return m_bIsLooping; }
	void setLooping(bool bIsLooping) { m_bIsLooping = bIsLooping; }

	virtual KInitStatus initialiseUnit() override { return Success; }
	virtual void tickUnit() override;

private:

	std::vector<sf::IntRect> m_keyframes;

	sf::Texture* mp_spriteSheet;

	Vec2i m_tileDim;

	float m_timer;
	float m_frameTime;

	bool m_bIsLooping;
	bool m_bIsPlaying = false;

	uint32 m_currentFrame;
};

#endif
