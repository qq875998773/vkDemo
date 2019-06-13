#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[])
{
    try
    {
        std::cout << "hello vulkan!\n";
        //app.run();
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}