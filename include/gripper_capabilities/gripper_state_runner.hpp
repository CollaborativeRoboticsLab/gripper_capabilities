#pragma once

#include <chrono>
#include <mutex>
#include <string>
#include <thread>

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>

#include <capabilities2_events/event_parameters.hpp>
#include <capabilities2_runner/threadtrigger_runner.hpp>

#include <gripper_capabilities/gripper_runner_utils.hpp>

namespace capabilities2_runner
{
/**
 * @brief Read the current gripper articulation state from the backend-owned gripper joint-state topic.
 */
class GripperStateRunner : public ThreadTriggerRunner
{
public:
  /**
   * @brief Default constructor required for pluginlib.
   */
  GripperStateRunner()
  : ThreadTriggerRunner()
  {
  }

  /**
   * @brief Subscribe to joint_states and emit the started event.
   *
   * @param node Shared ROS node.
   * @param run_config Runner configuration.
   * @param bond_id Runner bond identifier.
   */
  void start(rclcpp::Node::SharedPtr node, const runner_opts & run_config, const std::string & bond_id) override
  {
    init_base(node, run_config);

    std::string topic_name = "/joint_states";
    try
    {
      topic_name = get_topic_name_by_type("sensor_msgs::msg::JointState");
    }
    catch (const runner_exception &)
    {
    }

    joint_state_subscription_ = node_->create_subscription<sensor_msgs::msg::JointState>(
      topic_name,
      10,
      std::bind(&GripperStateRunner::joint_state_callback, this, std::placeholders::_1));
    emit_started(bond_id, "", param_on_started());
  }

  /**
   * @brief Emit the stopped event for this runner.
   *
   * @param bond_id Runner bond identifier.
   * @param instance_id Optional capability instance identifier.
   */
  void stop(const std::string & bond_id, const std::string & instance_id = "") override
  {
    if (!node_ || !joint_state_subscription_)
    {
      throw runner_exception("cannot stop runner that was not started");
    }
    emit_stopped(bond_id, instance_id, param_on_stopped());
  }

protected:
  /**
  * @brief Return the latest gripper joint-state sample.
   *
  * @param parameters Event parameters containing timeout.
   * @param thread_id Thread identifier for the running trigger instance.
   */
  void execution(capabilities2_events::EventParameters parameters, const std::string & thread_id) override
  {
    const std::string bond_id = ThreadTriggerRunner::bond_from_thread_id(thread_id);
    const std::string instance_id = ThreadTriggerRunner::instance_from_thread_id(thread_id);

    const double timeout_sec = std::any_cast<double>(parameters.get_value("timeout_sec", 2.0));
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::duration<double>(timeout_sec);
    while (std::chrono::steady_clock::now() < deadline)
    {
      bool has_joint_state = false;
      {
        std::scoped_lock lock(joint_state_mutex_);
        has_joint_state = static_cast<bool>(latest_joint_state_);
      }
      if (has_joint_state)
      {
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    sensor_msgs::msg::JointState latest_joint_state;
    {
      std::scoped_lock lock(joint_state_mutex_);
      if (!latest_joint_state_)
      {
        status_message_ = "No joint-state message received before timeout.";
        emit_failed(bond_id, instance_id, param_on_failure());
        return;
      }
      latest_joint_state = *latest_joint_state_;
    }

    last_joint_names_ = latest_joint_state.name;
    last_joint_positions_.assign(latest_joint_state.position.begin(), latest_joint_state.position.end());
    last_joint_count_ = static_cast<int>(last_joint_names_.size());
    emit_succeeded(bond_id, instance_id, param_on_success());
  }

  /**
   * @brief Emit the selected gripper joint set.
   *
   * @return Event parameters for the success event.
   */
  capabilities2_events::EventParameters param_on_success() override
  {
    capabilities2_events::EventParameters parameters;
    parameters.set_value("joint_names", last_joint_names_, capabilities2_events::OptionType::VECTOR_STRING);
    parameters.set_value("joint_positions", last_joint_positions_, capabilities2_events::OptionType::VECTOR_DOUBLE);
    parameters.set_value("joint_count", last_joint_count_, capabilities2_events::OptionType::INT);
    return parameters;
  }

  /**
   * @brief Emit the current gripper state read error.
   *
   * @return Event parameters for the failure event.
   */
  capabilities2_events::EventParameters param_on_failure() override
  {
    capabilities2_events::EventParameters parameters;
    parameters.set_value("message", status_message_, capabilities2_events::OptionType::STRING);
    return parameters;
  }

private:
  /**
   * @brief Cache the latest incoming joint-state message.
   *
   * @param msg Latest joint-state sample.
   */
  void joint_state_callback(const sensor_msgs::msg::JointState::SharedPtr msg)
  {
    std::scoped_lock lock(joint_state_mutex_);
    latest_joint_state_ = msg;
  }

  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joint_state_subscription_;
  std::mutex joint_state_mutex_;
  sensor_msgs::msg::JointState::SharedPtr latest_joint_state_;
  std::vector<std::string> last_joint_names_;
  std::vector<double> last_joint_positions_;
  std::string status_message_;
  int last_joint_count_{0};
};
}  // namespace capabilities2_runner