#ifndef WIN_STATE_H
#define WIN_STATE_H

#include <Krawler.h>
#include <LogicState\KLogicState.h>

using namespace Krawler;

class WinState : public LogicState::KLogicState
{
public:

	virtual Krawler::KInitStatus setupState(const LogicState::KLogicStateInitialiser&  initialiser)override;
	virtual void cleanupState() override;
	virtual void tick()override;

private:
	sf::Text m_winText;
	bool m_bHasStateTicked = false;

};

#endif
