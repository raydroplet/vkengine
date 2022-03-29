#include "program.hpp"

int main()
{
  try
  {
    vke::Program{}.run();
  }
  catch(std::exception e)
  {
    std::cerr << e.what() << '.' << std::endl;
  }
  catch(...)
  {
    std::cerr << "Unhandled exception" << std::endl;
  }
}
