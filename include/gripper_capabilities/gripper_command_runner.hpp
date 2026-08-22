#pragma once

#include <string>

#include <control_msgs/action/gripper_command.hpp>

#include <capabilities2_events/event_parameters.hpp>
#include <capabilities2_runner/action_runner.hpp>

namespace capabilities2_runner
{
  /**
   * @brief Send position and effort commands through the gripper action interface.
   */
  class GripperCommandRunner : public ActionRunner<control_msgs::action::GripperCommand>
  {
  public:
    /**
     * @brief Default constructor required for pluginlib.
     */
    GripperCommandRunner()
        : ActionRunner()
    {
    }

    /**
     * @brief Initialize the action client and emit the started event.
     *
     * @param node Shared ROS node.
     * @param run_config Runner configuration.
     * @param bond_id Runner bond identifier.
     */
    void start(rclcpp::Node::SharedPtr node, const runner_opts &run_config, const std::string &bond_id) override
    {
      init_action(node, run_config, "/gripper_command", "control_msgs::action::GripperCommand");
      emit_started(bond_id, "", param_on_started());
    }

  protected:
    /**
     * @brief Convert event parameters into a GripperCommand goal.
     *
     * @param parameters Event parameters containing the position and optional effort.
     * @return Gripper action goal.
     */
    control_msgs::action::GripperCommand::Goal generate_goal(capabilities2_events::EventParameters &parameters) override
    {
      control_msgs::action::GripperCommand::Goal goal_msg;
      goal_msg.command.position = std::any_cast<double>(parameters.get_value("position", 0.0));
      goal_msg.command.max_effort = std::any_cast<double>(parameters.get_value("max_effort", 0.0));
      return goal_msg;
    }

    /**
     * @brief Format the live gripper feedback into a log string.
     *
     * @param msg Feedback message from the action server.
     * @return Human-readable feedback text.
     */
    std::string generate_feedback(const control_msgs::action::GripperCommand::Feedback::ConstSharedPtr msg) override
    {
      std::ostringstream stream;
      stream.setf(std::ios::fixed);
      stream.precision(4);
      stream << "position=" << msg->position << ", effort=" << msg->effort << ", stalled="
             << (msg->stalled ? "true" : "false") << ", reached_goal="
             << (msg->reached_goal ? "true" : "false");
      return stream.str();
    }

    /**
     * @brief Cache the final gripper action result.
     *
     * @param result Action result.
     */
    void process_result(control_msgs::action::GripperCommand::Result::SharedPtr result) override
    {
      last_position_ = result->position;
      last_effort_ = result->effort;
      last_stalled_ = result->stalled;
      last_reached_goal_ = result->reached_goal;
    }

    /**
     * @brief Emit the final gripper result values on success.
     *
     * @return Event parameters for the success event.
     */
    capabilities2_events::EventParameters param_on_success() override
    {
      capabilities2_events::EventParameters parameters;
      parameters.set_value("position", last_position_, capabilities2_events::OptionType::DOUBLE);
      parameters.set_value("effort", last_effort_, capabilities2_events::OptionType::DOUBLE);
      parameters.set_value("stalled", last_stalled_, capabilities2_events::OptionType::BOOL);
      parameters.set_value("reached_goal", last_reached_goal_, capabilities2_events::OptionType::BOOL);
      return parameters;
    }

    /**
     * @brief Reuse the final gripper result values on failure.
     *
     * @return Event parameters for the failure event.
     */
    capabilities2_events::EventParameters param_on_failure() override
    {
      return param_on_success();
    }

  private:
    double last_position_{0.0};
    double last_effort_{0.0};
    bool last_stalled_{false};
    bool last_reached_goal_{false};
  };
} // namespace capabilities2_runner