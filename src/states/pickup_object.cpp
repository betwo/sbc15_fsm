/// HEADER
#include "pickup_object.h"

/// COMPONENT
#include "../states/global_state.h"


PickupObject::PickupObject(State* parent, bool store,ArmGoal& armInterimPose)
    : MetaState(parent),

      plan_arm_motion(this,1),
      visual_servoing(this,2),
      pose_interim(this,2,armInterimPose),

      store_object(this,2),
      place_object(this, 1),
      open_gripper(this, sbc15_msgs::GripperServices::Request::OPEN_GRIPPER),

      pre_pos(this, sbc15_msgs::PreplannedTrajectories::Request::PRE_POSITION, 1),
      drive_forward(this, 0.02, 0.05),

      back_up(this, sbc15_msgs::PreplannedTrajectories::Request::PRE_POSITION, 1),
      drive_backward(this, 0.02, -0.05),

      abort(this, sbc15_msgs::GripperServices::Request::SEMI_CLOSE),
      abort2(this, sbc15_msgs::PreplannedTrajectories::Request::PLACE_ARM_FROM_FRONT, 1),

      event_object_pickedup(this, "The object has been reached"),
      event_object_failure(this, "The object pose is not known"),
      event_object_out_of_range(this, "Object is out of range"),
      event_servo_control_failed(this,"Control of arm failed"),
      event_planning_failed(this,"Planning was not possible."),

      store_(store)
{
    event_entry_meta >> plan_arm_motion;

    plan_arm_motion.event_done >> visual_servoing;

    if(store_) {
        visual_servoing.event_object_gripped >> pose_interim;
        pose_interim.event_done >>  store_object;
        store_object.object_stored >> event_object_pickedup;
        store_object.event_failure >> store_object;

    } else {
        visual_servoing.event_object_gripped >> place_object;
        place_object.event_object_placed >> open_gripper;
        open_gripper.event_done >> event_object_pickedup;
    }

    plan_arm_motion.event_failure >> event_object_failure;

    visual_servoing.event_out_of_range >> abort;

    visual_servoing.event_failure >> plan_arm_motion;
    visual_servoing.event_timeout >> visual_servoing;
    visual_servoing.event_servo_control_failed >> event_servo_control_failed;
    visual_servoing.event_no_object >> abort;

    abort.event_done >> abort2;
    abort2.event_done >> event_object_pickedup;


    pre_pos.event_done >> drive_forward;
    drive_forward.event_positioned >> visual_servoing;

    back_up.event_done >> drive_backward;
    drive_backward.event_positioned >> visual_servoing;

    pose_interim.event_failure >> event_planning_failed;
    pose_interim.event_planning_failed >> event_planning_failed;
    pose_interim.event_servo_control_failed >> event_servo_control_failed;
    pose_interim.event_timeout >> pose_interim;


}

double PickupObject::desiredFrequency() const
{
    return 1.0;
}
