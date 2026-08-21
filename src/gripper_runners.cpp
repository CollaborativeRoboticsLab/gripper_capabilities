#include <pluginlib/class_list_macros.hpp>

#include <capabilities2_runner/runner_base.hpp>
#include <gripper_capabilities/gripper_runners.hpp>

PLUGINLIB_EXPORT_CLASS(capabilities2_runner::GripperCommandRunner, capabilities2_runner::RunnerBase)
PLUGINLIB_EXPORT_CLASS(capabilities2_runner::GripperStateRunner, capabilities2_runner::RunnerBase)