#include "shell-client.h"

#include <iostream>

int main()
{
  ShellClient shellClient(std::cin, std::cout);
  try
  {
    shellClient.run();
  }
  catch (std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}