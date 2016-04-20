/**
 * @file cli_impl.hpp
 * @author Matthew Amidon
 *
 * Implementation of templated functions of the CLI class.
 *
 * This file is part of mlpack 1.0.12.
 *
 * mlpack is free software; you may redstribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#ifndef __MY_MLPACK_CORE_UTIL_CLI_IMPL_HPP
#define __MY_MLPACK_CORE_UTIL_CLI_IMPL_HPP

// In case it has not already been included.
#include "cli.hpp"

// Include option.hpp here because it requires CLI but is also templated.
#include "option.hpp"

namespace mips {

/**
 * @brief Adds a parameter to CLI, making it accessibile via GetParam &
 *     CheckValue.
 *
 * @tparam T The type of the parameter.
 * @param identifier The name of the parameter, eg foo in bar/foo.
 * @param description A string description of the parameter.
 * @param parent The name of the parent of the parameter,
 *   eg bar/foo in bar/foo/buzz.
 * @param required If required, the program will refuse to run
 *   unless the parameter is specified.
 */
template<typename T>
void CLI::Add(const std::string& path,
              const std::string& description,
              const std::string& alias,
              bool required)
{

  po::options_description& desc = CLI::GetSingleton().desc;
  // Must make use of boost syntax here.
  std::string progOptId = alias.length() ? path + "," + alias : path;

  // Add the alias, if necessary
  AddAlias(alias, path);

  // Add the option to boost program_options.
  desc.add_options()(progOptId.c_str(), po::value<T>(), description.c_str());

  // Make sure the appropriate metadata is inserted into gmap.
  gmap_t& gmap = GetSingleton().globalValues;

  ParamData data;
  T tmp = T();

  data.desc = description;
  data.name = path;
  data.tname = TYPENAME(T);
  data.value = boost::any(tmp);
  data.wasPassed = false;

  gmap[path] = data;

  // If the option is required, add it to the required options list.
  if (required)
    GetSingleton().requiredOptions.push_front(path);
}

// We specialize this in cli.cpp.
template<>
bool& CLI::GetParam<bool>(const std::string& identifier);

/**
 * @brief Returns the value of the specified parameter.
 *   If the parameter is unspecified, an undefined but
 *   more or less valid value is returned.
 *
 * @tparam T The type of the parameter.
 * @param identifier The full pathname of the parameter.
 *
 * @return The value of the parameter.  Use CLI::CheckValue to determine if it's
 *     valid.
 */
template<typename T>
T& CLI::GetParam(const std::string& identifier)
{
  // Used to ensure we have a valid value.
  T tmp = T();

  // Used to index into the globalValues map.
  std::string key = std::string(identifier);
  gmap_t& gmap = GetSingleton().globalValues;

  // Now check if we have an alias.
  amap_t& amap = GetSingleton().aliasValues;
  if (amap.count(key))
    key = amap[key];

  // What if we don't actually have any value?
  if (!gmap.count(key))
  {
    gmap[key] = ParamData();
    gmap[key].value = boost::any(tmp);
    *boost::any_cast<T>(&gmap[key].value) = tmp;
  }

  // What if we have meta-data, but no data?
  boost::any val = gmap[key].value;
  if (val.empty())
    gmap[key].value = boost::any(tmp);

  return *boost::any_cast<T>(&gmap[key].value);
}

}; // namespace mlpack

#endif