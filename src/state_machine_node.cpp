/// COMPONENT
#include "lightfsm/state_machine.h"
#include "lightfsm_ros/state_machine_ros_executor.h"
#include "lightfsm/state.h"
#include "lightfsm/meta_state.h"
#include "lightfsm/event.h"
#include "lightfsm/triggered_event.h"

#include "global.h"

#include "states/wait_for_go_signal.h"
#include "states/global_state.h"
#include "states/wait.h"
#include "states/select_task.h"
#include "states/goto_object.h"
#include "states/wait.h"
#include "states/explore.h"
#include "states/fetch_object.h"
#include "states/back_up.h"
#include "states/go_to_base.h"
#include "states/back_up.h"

/// SYSTEM
#include <ros/ros.h>
#include <sound_play/SoundRequest.h>

void tick(State* current_state)
{
    GlobalState::getInstance().update(current_state);
}

int main(int argc, char* argv[])
{
    ros::init(argc, argv, "sbc15_state_machine_node", ros::InitOption::NoSigintHandler);
    ros::NodeHandle nh;

    ROS_WARN("waiting for ros time");
    sbc15_fsm_global::waitForRosTime();

    // STATES
    ROS_INFO("creating states");

    WaitForGoSignal init(State::NO_PARENT);
    Wait error(State::NO_PARENT, 2);
    Wait goal(State::NO_PARENT, 2);
    Wait done_loop(State::NO_PARENT, 30);

    SelectTask select(State::NO_PARENT);

    BackUp forward(State::NO_PARENT, 1.0, 0.1);

    Explore explore(State::NO_PARENT);

    FetchObject fetch_object(State::NO_PARENT, true);

    GoToBase goto_base(State::NO_PARENT);

    // ACTIONS
    ROS_INFO("connecting events");
    init.event_done >> select;

    select.event_object_selected >> fetch_object;
    select.event_object_unknown >> explore;
    select.event_all_objects_collected >> goto_base;
    select.event_first_move >> forward;

    forward.event_positioned >> select;

    goto_base.event_base_unknown >> explore;

    fetch_object.event_object_unknown >> select;
    fetch_object.event_failure >> select;
    fetch_object.event_object_fetched >> select;

    explore.event_object_found >> select;

    goal.event_done >> done_loop;
    done_loop.event_done >> done_loop;

    // TRANSITIONS
    error.event_done >> goal;

    // TALKING
    using sbc15_fsm_global::action::say;
    init.action_entry << []() { say("Waiting for go signal!"); };
    init.action_exit << []() { say("It's show time!"); };
    fetch_object.goto_object.action_entry << []() { say("Going to the object"); };
    fetch_object.pickup_object.action_entry << []() { say("Collecting the object"); };

    ROS_INFO("starting state machine");
    StateMachine state_machine(&init);

    ros::Publisher state_pub = nh.advertise<std_msgs::String>("fsm_state", 1);

    ros::Time last_pub = ros::Time(0);
    ros::Duration state_pub_rate(1.0);

    std::function<void(const std_msgs::StringConstPtr&)> cb = [&](const std_msgs::StringConstPtr& cmd) {
        if (cmd->data == "fsm/reset") {
            sbc15_fsm_global::action::say("reset requested");
            state_machine.reset();

        } else if (cmd->data == "fsm/pickup/cup") {
            sbc15_fsm_global::action::say("pickup cup requested");
            sbc15_msgs::ObjectPtr cup = boost::make_shared<sbc15_msgs::Object>();
            cup->type = sbc15_msgs::Object::OBJECT_CUP;
            GlobalState::getInstance().setCurrentObject(cup);
            state_machine.gotoState(&fetch_object.pickup_object);

        } else if (cmd->data == "fsm/pickup/battery") {
            sbc15_fsm_global::action::say("pickup battery requested");
            sbc15_msgs::ObjectPtr battery = boost::make_shared<sbc15_msgs::Object>();
            battery->type = sbc15_msgs::Object::OBJECT_BATTERY;
            GlobalState::getInstance().setCurrentObject(battery);
            state_machine.gotoState(&fetch_object.pickup_object);

        } else if (cmd->data == "fsm/approach/cup") {
            sbc15_fsm_global::action::say("approach cup requested");
            sbc15_msgs::ObjectPtr cup = boost::make_shared<sbc15_msgs::Object>();
            cup->type = sbc15_msgs::Object::OBJECT_CUP;
            GlobalState::getInstance().setCurrentObject(cup);
            state_machine.gotoState(&fetch_object.approach);

        } else if (cmd->data == "fsm/approach/battery") {
            sbc15_fsm_global::action::say("approach battery requested");
            sbc15_msgs::ObjectPtr battery = boost::make_shared<sbc15_msgs::Object>();
            battery->type = sbc15_msgs::Object::OBJECT_BATTERY;
            GlobalState::getInstance().setCurrentObject(battery);
            state_machine.gotoState(&fetch_object.approach);
        }
    };

    ros::Subscriber sub_cmd = nh.subscribe<std_msgs::String>("/command", 100, cb);

    StateMachineRosExecutor executor(state_machine);

    executor.run([&](State* current_state) {
        tick(current_state);

        ros::Time now = ros::Time::now();
        if (now > last_pub + state_pub_rate) {
            last_pub = now;
            std_msgs::String msg;
            msg.data = state_machine.generateGraphDescription();
            state_pub.publish(msg);
        }
    });

    return 0;
}
