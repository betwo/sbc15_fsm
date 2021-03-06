/// HEADER
#include "place_object.h"

/// COMPONENT
#include "../states/global_state.h"
#include <sbc15_msgs/PreplannedTrajectories.h>
#include <control_msgs/FollowJointTrajectoryResult.h>

PlaceObject::PlaceObject(State* parent, int retries)
  : MetaState(parent)
  , event_object_placed(this, "object is placed")
  ,

  place_object(this, sbc15_msgs::PreplannedTrajectoriesRequest::PRE_POSITION, retries)
  , open_gripper(this, sbc15_msgs::GripperServices::Request::OPEN_GRIPPER, 0)
  , pre_rest_position(this, sbc15_msgs::PreplannedTrajectoriesRequest::PLACE_CUP, retries)
  , close_gripper(this, sbc15_msgs::GripperServices::Request::SEMI_CLOSE, 0)
  , rest_position(this, sbc15_msgs::PreplannedTrajectoriesRequest::PLACE_ARM_FROM_CUP, retries)

{
    event_entry_meta >> place_object;

    place_object.event_done >> open_gripper;
    place_object.event_failure >> place_object;

    open_gripper.event_done >> pre_rest_position;

    pre_rest_position.event_done >> close_gripper;
    pre_rest_position.event_failure >> close_gripper;

    close_gripper.event_done >> rest_position;

    rest_position.event_done >> event_object_placed;
    rest_position.event_failure >> event_object_placed;
}
