#include "GameStates\PlayState.h"

PlayState::PlayState()
{

}

KInitStatus PlayState::setupState(const KLogicStateInitialiser & initaliser)
{
	auto result = KLogicState::setupState(initaliser);
	if (result != Success)
	{
		return result;
	}
	return KInitStatus::Success;
}

void PlayState::cleanupState()
{
}

void PlayState::fixedTick()
{
}

void PlayState::tick()
{
}
