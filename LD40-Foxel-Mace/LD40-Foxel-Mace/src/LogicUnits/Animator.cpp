#include "LogicUnits\Animator.h"

#include <KApplication.h>

Animator::Animator(sf::Texture * pSpritesheet, SLU::KStateLogicUnitAdministrator & rAdmin)
	:SLU::KGameObjectLogicUnit(KTEXT("Animator") + Krawler::GenerateUUID(), rAdmin), mp_spriteSheet(pSpritesheet), m_bIsLooping(false), m_timer(0.0f),
	m_frameTime(0.0f), m_bIsPlaying(false), m_currentFrame(0)
{
}

Animator::~Animator()
{
	m_keyframes.clear();
}

void Animator::addKeyFrame(sf::IntRect keyFrame)
{
	keyFrame.left *= m_tileDim.x;
	keyFrame.top *= m_tileDim.y;
	keyFrame.width = m_tileDim.x;
	keyFrame.height = m_tileDim.y;

	m_keyframes.push_back(keyFrame);
}

void Animator::play()
{
	m_bIsPlaying = true;
	getGameObj()->setTexture(mp_spriteSheet);
	setFrame(m_currentFrame, false);
}

void Animator::pause()
{
	m_bIsPlaying = false;
}

void Animator::stop()
{
	m_bIsPlaying = false;
	setFrame(0, true);
}

void Animator::setFrame(uint32 frameIdx, bool resetAnim)
{
	getGameObj()->setTextureRect(m_keyframes[frameIdx]);
	m_currentFrame = frameIdx;
	if (resetAnim)
	{
		m_timer = 0.0f;
	}
}

void Animator::setFrameTime(float frameTime)
{
	m_frameTime = frameTime;
}

void Animator::tickUnit()
{
	if (!m_bIsPlaying)
	{
		return;
	}
	const float dt = KApplication::getApp()->getDeltaTime();

	m_timer += dt;

	if (m_timer > m_frameTime)
	{
		auto size = m_keyframes.size();
		++m_currentFrame;
		m_timer = fmod(m_timer, m_frameTime);

		if (m_currentFrame >= m_keyframes.size())
		{
			m_currentFrame = 0;
			if (m_bIsLooping == false)
			{
				m_bIsPlaying = false;
			}
		}
		setFrame(m_currentFrame, false);
	}
}
