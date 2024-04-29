#include "shell-client.h"

#include <locale>
#include <vector>

using namespace std::literals;

static void trim_left(std::string string)
{
    while (std::isspace(string[0]))
    {
        string.erase(0, 1);
    }
}

static bool contains(const std::string_view input, const char ch)
{
    for (const auto inp : input)
    {
        if (inp == ch)
            return true;
    }
    return false;
}

static std::vector<std::string> tokenize(const std::string_view input, std::string_view delim = " \t"sv)
{
    std::vector<std::string> result;
    size_t start = 0, current = 0;
    for (; current < input.size(); ++current)
    {
        if (contains(delim, input[current]))
        {
            if (start != current)
            {
                result.push_back(std::string(input.substr(start, current - start)));
                start = current;
            }
            ++start;
        }
    }
    if (start != current)
    {
        result.push_back(std::string(input.substr(start, current - start)));
    }
    return result;
}

ShellClient::ShellClient(std::istream &input, std::ostream &output) : m_input(input), m_output(output)
{
}

ShellClient::~ShellClient()
{
    delete m_socket;
}

void ShellClient::run()
{
    int errorCounter = 0;
    std::string command;
    for (;;)
    {
        if (m_socket != nullptr)
        {
            m_output << "#";
        }
        std::getline(std::cin, command);
        trim_left(command);
        if (command.empty())
        {
            continue;
        }
        if (m_socket != nullptr)
        {
            shell(command);
            continue;
        }
        auto tokens = tokenize(command);
        if (!tokens.empty())
        {
            if (tokens[0] == "telnet")
            {
                telnet(tokens);
            }
            else
            {
                ++errorCounter;
                m_output << "Wrong command!" << std::endl;
                if (errorCounter >= 3)
                {
                    throw std::runtime_error("Exiting...");
                }
            }
        }
    }
}

void ShellClient::telnet(const std::vector<std::string> &params)
{
    auto print_usage = [this]()
    {
        m_output << "Usage: telnet <ip_address>:<port>" << std::endl;
    };
    if (params.size() < 2)
    {
        m_output << "Not enough parameters" << std::endl;
        print_usage();
        return;
    }
    auto sv = std::string_view(params[1]);
    auto delim = sv.find_first_of(":", 0);
    if (delim + 1 >= sv.size())
    {
        print_usage();
        return;
    }

    unsigned long port = 0;
    try
    {
        port = std::stoul(params[1].substr(delim + 1, sv.size() - delim - 1));
    }
    catch (const std::exception &e)
    {
        print_usage();
        return;
    }

    boost::system::error_code ec;
    auto address = boost::asio::ip::make_address_v4(sv.substr(0, delim), ec);
    if (ec)
    {
        m_output << ec.message() << std::endl;
        print_usage();
    }

    m_socket = new tcp::socket(m_context);
    m_socket->connect(tcp::endpoint(address, port), ec);
    if (ec)
    {
        delete m_socket;
        m_socket = nullptr;
        m_output << ec.message();
    }
}

void ShellClient::shell(const std::string &command)
{
    if (command == "exit")
    {
        delete m_socket;
        m_socket = nullptr;
        return;
    }
    boost::system::error_code ec;
    do
    {
        boost::asio::write(*m_socket, boost::asio::buffer(command), ec);
        if (ec)
        {
            break;
        }

        static char buf[256];
        auto len = m_socket->read_some(boost::asio::buffer(buf, sizeof(buf) - 1));
        buf[len] = 0;
        m_output << buf << std::endl;
    } while (0);
    if (ec)
    {
        delete m_socket;
        m_socket = nullptr;
        m_output << ec.message();
    }
}
