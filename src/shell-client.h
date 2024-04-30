#include <boost/asio.hpp>
#include <string>
#include <string_view>
#include <map>
#include <functional>
#include <iostream>
#include <optional>

using boost::asio::ip::tcp;

class ShellClient final
{
public:
    ShellClient(std::istream &m_input, std::ostream &m_output);
    void run();

private:
    void telnet(const std::vector<std::string> &params);
    void shell(std::string &command);
    void close_socket();
    void dispatch_command(const std::string &command);
    tcp::endpoint parse_endpoint(const std::vector<std::string> &params);

    boost::asio::io_context m_context;
    std::optional<tcp::socket> m_socket;
    std::istream &m_input;
    std::ostream &m_output;
    int m_errorCounter = 0;
    boost::asio::streambuf m_buf;
};