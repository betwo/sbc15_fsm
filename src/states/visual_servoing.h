#ifndef VISUAL_SERVOING_H
#define VISUAL_SERVOING_H

/// COMPONENT
#include "../fsm/state.h"
#include "../fsm/triggered_event.h"

#include <actionlib/client/simple_action_client.h>
#include <sbc15_msgs/visual_servoingAction.h>

class VisualServoing: public State
{
public:
    TriggeredEvent event_object_gripped;
    TriggeredEvent event_failure;
public:
    VisualServoing(State* parent, int retries);

    void entryAction();
    void iteration();

//    void grapObject(const sbc15_msgs::visual_servoingGoal& goal, TriggeredEvent &event);
//    void grapObject(const sbc15_msgs::visual_servoingGoal& goal,TriggeredEvent &event, boost::function<void(const sbc15_msgs::visual_servoingFeedbackConstPtr&)> cb);
//    void grapObject(int object, double phi, double theta, ros::Duration t, TriggeredEvent &event);
//    void grapObject(int object, double phi, double theta, ros::Duration t,  TriggeredEvent &event, boost::function<void(const sbc15_msgs::visual_servoingFeedbackConstPtr&)> cb);

private:
    int retries_;
    int retries_left_;

    sbc15_msgs::visual_servoingGoal goal_;
    actionlib::SimpleActionClient<sbc15_msgs::visual_servoingAction> client_;

    void doneCb(const actionlib::SimpleClientGoalState& state,
           const sbc15_msgs::visual_servoingResultConstPtr& result);


};

#endif // VISUAL_SERVOING_H
