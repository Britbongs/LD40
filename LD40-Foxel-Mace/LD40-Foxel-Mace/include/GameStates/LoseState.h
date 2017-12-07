#ifndef LOSE_STATE_H
#define LOSE_STATE_H

#include <Krawler.h>
#include <LogicState\KLogicState.h>

using namespace Krawler;

class LoseState : public LogicState::KLogicState
{
public:

	virtual Krawler::KInitStatus setupState(const LogicState::KLogicStateInitialiser&  initaliser)override;
	virtual void cleanupState() override;
	virtual void tick()override;

private:
	sf::Text m_loseText;
	bool m_bHasStateTicked = false;

};

#endif