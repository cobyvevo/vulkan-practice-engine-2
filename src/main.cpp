#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <string.h>
#include <cstring>
#include <optional>
#include <set>
#include <limits>
#include <algorithm>
#include <fstream>

#include "engine.hpp"

int main() {
    
    MainEngine engine(300,300);

    std::cout << "ended" << std::endl;

    return EXIT_SUCCESS;

}