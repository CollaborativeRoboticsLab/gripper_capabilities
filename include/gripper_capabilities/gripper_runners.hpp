#pragma once

#include <any>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <control_msgs/action/gripper_command.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>

#include <capabilities2_events/event_parameters.hpp>
#include <capabilities2_runner/action_runner.hpp>
#include <capabilities2_runner/threadtrigger_runner.hpp>

namespace capabilities2_runner
{
    namespace gripper_detail
    {
        inline std::string trim(const std::string &value)
        {
            const auto begin = value.find_first_not_of(" \t\n\r");
            if (begin == std::string::npos)
            {
                return "";
            }
            const auto end = value.find_last_not_of(" \t\n\r");
            return value.substr(begin, end - begin + 1);
        }

        inline std::vector<std::string> split_csv(const std::string &text)
        {
            std::vector<std::string> values;
            std::stringstream stream(text);
            std::string item;
            while (std::getline(stream, item, ','))
            {
                item = trim(item);
                if (!item.empty())
                {
                    values.push_back(item);
                }
            }
            return values;
        }

        inline std::string join_csv(const std::vector<std::string> &values)
        {
            std::ostringstream stream;
            for (size_t index = 0; index < values.size(); ++index)
            {
                if (index > 0)
                {
                    stream << ",";
                }
                stream << values[index];
            }
            return stream.str();
        }

        inline std::string join_csv(const std::vector<double> &values)
        {
            std::ostringstream stream;
            stream.setf(std::ios::fixed);
            stream.precision(6);
            for (size_t index = 0; index < values.size(); ++index)
            {
                if (index > 0)
                {
                    stream << ",";
                }
                stream << values[index];
            }
            return stream.str();
        }

        inline std::vector<std::string> any_to_string_vector(const std::any &value)
        {
            if (value.type() == typeid(std::string))
            {
                return split_csv(std::any_cast<std::string>(value));
            }
            if (value.type() == typeid(std::vector<std::string>))
            {
                return std::any_cast<std::vector<std::string>>(value);
            }
            throw runner_exception("expected a string or string vector parameter");
        }
    } // namespace gripper_detail

    class GripperCommandRunner : public ActionRunner<control_msgs::action::GripperCommand>
    {
    public:
        GripperCommandRunner() : ActionRunner()
        {
        }

        void start(rclcpp::Node::SharedPtr node, const runner_opts &run_config, const std::string &bond_id) override
        {
            init_action(node, run_config, "/gripper_command", "control_msgs::action::GripperCommand");
            emit_started(bond_id, "", param_on_started());
        }

    protected:
        control_msgs::action::GripperCommand::Goal generate_goal(capabilities2_events::EventParameters &parameters) override
        {
            control_msgs::action::GripperCommand::Goal goal_msg;
            goal_msg.command.position = std::any_cast<double>(parameters.get_value("position", 0.0));
            goal_msg.command.max_effort = std::any_cast<double>(parameters.get_value("max_effort", 0.0));
            return goal_msg;
        }

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

        void process_result(control_msgs::action::GripperCommand::Result::SharedPtr result) override
        {
            last_position_ = result->position;
            last_effort_ = result->effort;
            last_stalled_ = result->stalled;
            last_reached_goal_ = result->reached_goal;
        }

        capabilities2_events::EventParameters param_on_success() override
        {
            capabilities2_events::EventParameters parameters;
            parameters.set_value("position", last_position_, capabilities2_events::OptionType::DOUBLE);
            parameters.set_value("effort", last_effort_, capabilities2_events::OptionType::DOUBLE);
            parameters.set_value("stalled", last_stalled_, capabilities2_events::OptionType::BOOL);
            parameters.set_value("reached_goal", last_reached_goal_, capabilities2_events::OptionType::BOOL);
            return parameters;
        }

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

    class GripperStateRunner : public ThreadTriggerRunner
    {
    public:
        GripperStateRunner() : ThreadTriggerRunner()
        {
        }

        void start(rclcpp::Node::SharedPtr node, const runner_opts &run_config, const std::string &bond_id) override
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

        void stop(const std::string &bond_id, const std::string &instance_id = "") override
        {
            if (!node_ || !joint_state_subscription_)
            {
                throw runner_exception("cannot stop runner that was not started");
            }
            emit_stopped(bond_id, instance_id, param_on_stopped());
        }

    protected:
        void execution(capabilities2_events::EventParameters parameters, const std::string &thread_id) override
        {
            const std::string bond_id = ThreadTriggerRunner::bond_from_thread_id(thread_id);
            const std::string instance_id = ThreadTriggerRunner::instance_from_thread_id(thread_id);

            const double timeout_sec = std::any_cast<double>(parameters.get_value("timeout_sec", 2.0));
            std::vector<std::string> requested_joint_names;
            try
            {
                requested_joint_names = gripper_detail::any_to_string_vector(
                    parameters.get_value("joint_names_csv", std::string("")));
            }
            catch (const runner_exception &)
            {
                requested_joint_names.clear();
            }

            const auto deadline = std::chrono::steady_clock::now() + std::chrono::duration<double>(timeout_sec);
            while (std::chrono::steady_clock::now() < deadline)
            {
                std::scoped_lock lock(joint_state_mutex_);
                if (latest_joint_state_)
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

            std::vector<std::string> output_joint_names;
            std::vector<double> output_joint_positions;
            if (requested_joint_names.empty())
            {
                output_joint_names = latest_joint_state.name;
                output_joint_positions.assign(latest_joint_state.position.begin(), latest_joint_state.position.end());
            }
            else
            {
                for (const auto &requested_name : requested_joint_names)
                {
                    bool found = false;
                    for (size_t index = 0; index < latest_joint_state.name.size() && index < latest_joint_state.position.size(); ++index)
                    {
                        if (latest_joint_state.name[index] == requested_name)
                        {
                            output_joint_names.push_back(latest_joint_state.name[index]);
                            output_joint_positions.push_back(latest_joint_state.position[index]);
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        status_message_ = "Requested gripper joint not found in latest joint state: " + requested_name;
                        emit_failed(bond_id, instance_id, param_on_failure());
                        return;
                    }
                }
            }

            last_joint_names_csv_ = gripper_detail::join_csv(output_joint_names);
            last_joint_positions_csv_ = gripper_detail::join_csv(output_joint_positions);
            last_joint_count_ = static_cast<int>(output_joint_names.size());
            emit_succeeded(bond_id, instance_id, param_on_success());
        }

        capabilities2_events::EventParameters param_on_success() override
        {
            capabilities2_events::EventParameters parameters;
            parameters.set_value("joint_names_csv", last_joint_names_csv_, capabilities2_events::OptionType::STRING);
            parameters.set_value("joint_positions_csv", last_joint_positions_csv_, capabilities2_events::OptionType::STRING);
            parameters.set_value("joint_count", last_joint_count_, capabilities2_events::OptionType::INT);
            return parameters;
        }

        capabilities2_events::EventParameters param_on_failure() override
        {
            capabilities2_events::EventParameters parameters;
            parameters.set_value("message", status_message_, capabilities2_events::OptionType::STRING);
            return parameters;
        }

    private:
        void joint_state_callback(const sensor_msgs::msg::JointState::SharedPtr msg)
        {
            std::scoped_lock lock(joint_state_mutex_);
            latest_joint_state_ = msg;
        }

        rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joint_state_subscription_;
        std::mutex joint_state_mutex_;
        sensor_msgs::msg::JointState::SharedPtr latest_joint_state_;
        std::string last_joint_names_csv_;
        std::string last_joint_positions_csv_;
        std::string status_message_;
        int last_joint_count_{0};
    };
} // namespace capabilities2_runner