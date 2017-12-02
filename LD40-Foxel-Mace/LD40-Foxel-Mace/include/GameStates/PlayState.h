#ifndef PLAY_STATE_H
#define PLAY_STATE_H

#include <LogicState\KLogicState.h>

using namespace Krawler;
using namespace Krawler::LogicState;

class PlayState : public KLogicState
{
public:
	PlayState();
	~PlayState() = default;

	virtual KInitStatus setupState(const KLogicStateInitialiser&  initaliser) override;
	virtual void cleanupState() override;
	virtual void fixedTick() override;
	virtual void tick() override;

private:

};

#endif