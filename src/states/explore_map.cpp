/// HEADER
#include "explore_map.h"

/// COMPONENT
#include "global_state.h"

ExploreMap::ExploreMap(State* parent) : State(parent), event_step(this, "one exploration step done")
{
}

void ExploreMap::entryAction()
{
    explorer_.startExploring();
}

void ExploreMap::iteration()
{
    if (!explorer_.isExploring()) {
        event_step.trigger();
    }
}

void ExploreMap::exitAction()
{
    explorer_.stopExploring();
}
