/// COMPONENT
#include "lightfsm/state_machine.h"
#include "lightfsm_ros/state_machine_ros_executor.h"
#include "lightfsm/state.h"
#include "lightfsm/meta_state.h"
#include "lightfsm/event.h"
#include "lightfsm/triggered_event.h"

#include "../global.h"

#include "../utils/map_explorer.h"
#include "../states/wait.h"
#include "../states/explore.h"
#include "../states/fetch_object.h"
#include "../states/select_task.h"

void tick(State* current_state)
{
    GlobalState::getInstance().update(current_state);
}

struct WaitForGo : public State
{
public:
    TriggeredEvent event_go;

public:
    WaitForGo(State* parent) : State(parent), event_go(this, "Go Signal")
    {
        sub = GlobalState::getInstance().nh.subscribe<std_msgs::Bool>(
            "/go", 1, std::bind(&WaitForGo::go, this, std::placeholders::_1));
    }

    void go(const std_msgs::BoolConstPtr&)
    {
        event_go.trigger();
    }

    void iteration()
    {
    }

private:
    ros::Subscriber sub;
};

int main(int argc, char* argv[])
{
    ros::init(argc, argv, "sbc15_state_machine_node", ros::InitOption::NoSigintHandler);
    ros::NodeHandle nh;
    ros::NodeHandle p_nh("~");

    sbc15_fsm_global::waitForRosTime();

    // STATES
    Wait wait(State::NO_PARENT, 10.0);
    Wait goal(State::NO_PARENT, 10.0);

    Explore explore(State::NO_PARENT);

    // ACTIONS
    wait.event_done >> explore;
    // explore.event_object_found >> explore;

    goal.event_done >> goal;

    StateMachine state_machine(&wait);

    StateMachineRosExecutor executor(state_machine);

    executor.run([](State* state) { tick(state); });

    return 0;
}
