#include "main.hpp"

int main()
{
  try
  {
    vke::Program{}.run();
  }
  catch(std::runtime_error e)
  {
    std::cerr << clr::red << "[Exception] " << clr::white << e.what() << '.' << std::endl;
  }
  catch(...)
  {
    std::cerr << "Unhandled exception" << std::endl;
  }
}
