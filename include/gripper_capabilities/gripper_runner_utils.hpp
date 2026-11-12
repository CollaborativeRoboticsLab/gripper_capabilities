#pragma once

#include <any>
#include <sstream>
#include <string>
#include <vector>

#include <capabilities2_runner/runner_base.hpp>

namespace capabilities2_runner
{
namespace gripper_detail
{
/**
 * @brief Trim leading and trailing whitespace from a string.
 *
 * @param value Input text.
 * @return Trimmed text.
 */
inline std::string trim(const std::string & value)
{
  const auto begin = value.find_first_not_of(" \t\n\r");
  if (begin == std::string::npos)
  {
    return "";
  }
  const auto end = value.find_last_not_of(" \t\n\r");
  return value.substr(begin, end - begin + 1);
}

/**
 * @brief Split a comma-separated string into trimmed tokens.
 *
 * @param text Input CSV text.
 * @return Token list with empty entries removed.
 */
inline std::vector<std::string> split_csv(const std::string & text)
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

/**
 * @brief Join string values into a comma-separated representation.
 *
 * @param values Values to join.
 * @return CSV text.
 */
inline std::string join_csv(const std::vector<std::string> & values)
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

/**
 * @brief Join numeric values into a comma-separated representation.
 *
 * @param values Values to join.
 * @return CSV text with fixed precision.
 */
inline std::string join_csv(const std::vector<double> & values)
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

/**
 * @brief Convert a string or vector-valued parameter into a string list.
 *
 * @param value Event parameter payload.
 * @return Parsed string list.
 */
inline std::vector<std::string> any_to_string_vector(const std::any & value)
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
}  // namespace gripper_detail
}  // namespace capabilities2_runner