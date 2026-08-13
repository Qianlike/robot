#include "parse_robot_params.h"
#include <yaml-cpp/yaml.h>

#include <sstream>
#include <cstdlib>
#include <string>

namespace
{
std::string formatMark(const YAML::Mark &mark)
{
    if (mark.line < 0)
    {
        return "unknown location";
    }

    std::ostringstream oss;
    oss << "line " << (mark.line + 1)
        << ", column " << (mark.column + 1);
    return oss.str();
}

std::string makeFullKey(const std::string &path, const std::string &key)
{
    if (path.empty())
    {
        return key;
    }
    return path + "." + key;
}

std::string nodeToString(const YAML::Node &node)
{
    std::ostringstream oss;
    oss << node;
    return oss.str();
}

template <typename T>
void readConfigParam(const YAML::Node &node,
                     const std::string &key,
                     T &value,
                     const std::string &path = "")
{
    const std::string full_key = makeFullKey(path, key);
    const YAML::Node value_node = node[key];

    if (value_node)
    {
        try
        {
            value = value_node.as<T>();
        }
        catch (const YAML::BadConversion &e)
        {
            PRINT_ERROR("Error: Failed to convert '%s' to the required type at %s. %s",
                        full_key.c_str(),
                        formatMark(e.mark).c_str(),
                        e.what());
            std::exit(-1);
        }
        catch (const YAML::Exception &e)
        {
            PRINT_ERROR("Error: YAML exception on '%s' at %s. %s",
                        full_key.c_str(),
                        formatMark(e.mark).c_str(),
                        e.what());
            std::exit(-1);
        }
    }
    else
    {
        PRINT_ERROR("Error: '%s' is missing in configuration (near %s).",
                    full_key.c_str(),
                    formatMark(node.Mark()).c_str());
        std::exit(-1);
    }
}

template <typename T>
void readConfigParamOptional(const YAML::Node &node,
                             const std::string &key,
                             T &value,
                             const T &default_value,
                             const std::string &path = "")
{
    const std::string full_key = makeFullKey(path, key);
    const YAML::Node value_node = node[key];

    if (value_node)
    {
        try
        {
            value = value_node.as<T>();
        }
        catch (const YAML::BadConversion &e)
        {
            PRINT_ERROR("Error: Failed to convert '%s' to the required type at %s. %s",
                        full_key.c_str(),
                        formatMark(e.mark).c_str(),
                        e.what());
            std::exit(-1);
        }
        catch (const YAML::Exception &e)
        {
            PRINT_ERROR("Error: YAML exception on '%s' at %s. %s",
                        full_key.c_str(),
                        formatMark(e.mark).c_str(),
                        e.what());
            std::exit(-1);
        }
    }
    else
    {
        value = default_value;
    }
}
std::string get_dirname(const std::string &path)
{
    const size_t pos = path.find_last_of('/');
    if (pos == std::string::npos)
    {
        return "";
    }
    return path.substr(0, pos);
}
} // namespace

RobotParams parse_robot_params()
{
    return parse_robot_params("../robot_param/robot_config.yaml");
}

RobotParams parse_robot_params(const std::string &config_path)
{
    const std::string config_dir = get_dirname(config_path);

    std::string param_file = YAML::LoadFile(config_path)["param_file"].as<std::string>();
    // param_file 为相对路径时，相对 config 文件所在目录解析（原实现相对 CWD；
    // 对默认路径 ../robot_param/robot_config.yaml 二者归一化后结果一致）
    if (!param_file.empty() && param_file[0] != '/' && !config_dir.empty())
    {
        param_file = config_dir + "/" + param_file;
    }
    PRINT_INFO("param file: %s", param_file.c_str());

    auto config = YAML::LoadFile(param_file.c_str());
    RobotParams params;

    // YAML::Node 不能直接给 printf，先转成 string
    PRINT_INFO("%s", nodeToString(config).c_str());

    if (config["robot"])
    {
        YAML::Node robot_node = config["robot"];
        const std::string robot_path = "robot";

        readConfigParam(robot_node, "robot_name", params.robot_name, robot_path);
        readConfigParam(robot_node, "canport_num", params.can_port_num, robot_path);

        if (robot_node["canport"])
        {
            YAML::Node can_port_node = robot_node["canport"];
            int port_num = 0;

            for (YAML::const_iterator it = can_port_node.begin();
                 it != can_port_node.end() && port_num < params.can_port_num;
                 ++it, ++port_num)
            {
                std::string port_name = it->first.as<std::string>();
                YAML::Node port_node = it->second;

                const std::string port_path = robot_path + ".canport." + port_name;

                CanPortParams port;
                readConfigParam(port_node, "canport_id", port.can_port_id, port_path);
                readConfigParam(port_node, "motor_num", port.motor_num, port_path);

                if (port_node["motor"])
                {
                    YAML::Node motor_node = port_node["motor"];
                    int motor_idx = 0;

                    for (YAML::const_iterator motor_it = motor_node.begin();
                         motor_it != motor_node.end() && motor_idx < port.motor_num;
                         ++motor_it, ++motor_idx)
                    {
                        std::string motor_key = motor_it->first.as<std::string>();
                        YAML::Node motor_data = motor_it->second;

                        const std::string motor_path = port_path + ".motor." + motor_key;

                        MotorParams motor;
                        readConfigParam(motor_data, "id", motor.id, motor_path);
                        readConfigParam(motor_data, "name", motor.name, motor_path);

                        port.motors.push_back(motor);
                    }
                }

                params.can_ports.push_back(port);
            }
        }

        PRINT_INFO_G("Load robot params success: robot_name=%s, canport_num=%d",
                     params.robot_name.c_str(),
                     params.can_port_num);
    }
    else
    {
        PRINT_ERROR("Error: 'robot' is missing in configuration file.");
        std::exit(-1);
    }

    return params;
}
