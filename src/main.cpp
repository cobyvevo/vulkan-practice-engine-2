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
#include "meshtools.hpp"

int main() {
    MainEngine engine(300,300);
    //MeshTools::MeshData test;
   // test.load_from_file("assets/cube.obj");

    std::cout << "ended" << std::endl;

   // [[maybe_unused]] int x;
   // std::cin >> x;

    return EXIT_SUCCESS;

}