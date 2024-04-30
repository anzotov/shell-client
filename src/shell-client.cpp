#include "shell-client.h"

#include <locale>
#include <vector>
#include <stdexcept>

using namespace std::literals;

static void trim_left(std::string string);
static bool contains(const std::string_view input, const char ch);
static std::vector<std::string> tokenize(const std::string_view input, std::string_view delim = " \t"sv);

ShellClient::ShellClient(std::istream &input, std::ostream &output)
    : m_input(input), m_output(output)
{
}

ShellClient::~ShellClient()
{
    delete m_socket;
}

void ShellClient::run()
{
    m_errorCounter = 0;
    std::string command;
    for (;;)
    {
        if (m_socket != nullptr)
        {
            m_output << "#";
        }
        std::getline(std::cin, command);
        trim_left(command);
        if (m_socket == nullptr)
        {
            dispatch_command(command);
            continue;
        }
        if (command == "exit")
        {
            close_socket();
            continue;
        }
        shell(command);
    }
}

void ShellClient::telnet(const std::vector<std::string> &params)
{
    try
    {
        auto endpoint = parse_endpoint(params);

        m_socket = new tcp::socket(m_context);

        boost::system::error_code ec;
        m_socket->connect(endpoint, ec);

        if (ec)
        {
            close_socket();
            m_output << ec.message();
        }
    }
    catch (const std::logic_error &e)
    {
    }
}

void ShellClient::shell(std::string &command)
{
    m_context.restart();
    boost::system::error_code ec;
    do
    {
        if (!command.empty())
        {
            command += '\0';
            boost::asio::write(*m_socket, boost::asio::buffer(command), ec);
            if (ec)
                break;
        }
        auto read_handler = [this](const boost::system::error_code &ec, std::size_t bytes_transferred)
        {
            (void)bytes_transferred;
            if (ec)
            {
                close_socket();
                m_output << ec.message();
                return;
            }
            std::istream is(&m_buf);
            std::string line;
            std::getline(is, line, '\0');
            m_output << line << std::endl;
        };
        boost::asio::async_read_until(*m_socket, m_buf, '\0', read_handler);

        m_context.run_for(10s);
    } while (0);

    if (ec)
    {
        close_socket();
        m_output << ec.message();
    }
}

void ShellClient::close_socket()
{
    delete m_socket;
    m_socket = nullptr;
    std::istream is(&m_buf);
    is.ignore(std::numeric_limits<std::streamsize>::max());
}

void ShellClient::dispatch_command(const std::string &command)
{
    auto tokens = tokenize(command);
    if (tokens.empty())
        return;

    if (tokens[0] == "telnet")
    {
        telnet(tokens);
        return;
    }

    ++m_errorCounter;
    m_output << "Wrong command!" << std::endl;
    if (m_errorCounter >= 3)
    {
        throw std::runtime_error("Exiting...");
    }
}

tcp::endpoint ShellClient::parse_endpoint(const std::vector<std::string> &params)
{
    auto print_usage = [this]()
    {
        m_output << "Usage: telnet <ip_address>:<port>" << std::endl;
    };

    if (params.size() < 2)
    {
        m_output << "Not enough parameters" << std::endl;
        print_usage();
        throw std::logic_error("");
    }

    auto sv = std::string_view(params[1]);
    auto delim = sv.find_first_of(":", 0);
    if (delim == std::string::npos || delim + 1 >= sv.size())
    {
        m_output << "<port> not provided" << std::endl;
        print_usage();
        throw std::logic_error("");
    }

    unsigned long port = 0;
    try
    {
        port = std::stoul(params[1].substr(delim + 1, sv.size() - delim - 1));
    }
    catch (const std::exception &e)
    {
        m_output << "Invalid <port>" << std::endl;
        print_usage();
        throw std::logic_error("");
    }

    boost::system::error_code ec;
    auto address = boost::asio::ip::make_address_v4(sv.substr(0, delim), ec);
    if (ec)
    {
        m_output << "Invalid <address>" << std::endl;
        print_usage();
        throw std::logic_error("");
    }
    return tcp::endpoint(address, port);
}

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

static std::vector<std::string> tokenize(const std::string_view input, std::string_view delim)
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
