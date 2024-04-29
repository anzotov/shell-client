#include <boost/asio.hpp>
#include <string>
#include <string_view>
#include <map>
#include <functional>
#include <iostream>

using boost::asio::ip::tcp;

class ShellClient final
{
public:
    ShellClient(std::istream& m_input, std::ostream& m_output);
    ~ShellClient();
    void run();

private:
    void telnet(const std::vector<std::string> &params);
    void shell(const std::string &command);

    boost::asio::io_context m_context;
    tcp::socket *m_socket = nullptr;
    std::istream& m_input;
    std::ostream& m_output;
};