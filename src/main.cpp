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
    MainEngine engine(9999,9999); //fullscreen
    Scene* scene = &engine.testscene;

    engine.SCENE_new_material(scene,"assets/tex.png", "SmileTexture");
    engine.SCENE_new_material(scene,"assets/tabletexture.png", "Table_Texture");

    Object* cool_object = engine.SCENE_new_object(scene,"assets/cube.obj", "Cube", "SmileTexture");
    engine.SCENE_new_object(scene,"assets/table.obj", "TestAsset", "Table_Texture");
    engine.SCENE_new_object(scene,"assets/ball.obj", "TestAsset2", "none");

    while (engine.StepEngine()) {

        glm::vec3 objcenter = {sin(engine.tick*4) * 10.0f,0.0f,0.0f};
       // glm::vec3 sc = {cos(engine.tick*2) * 5.0f,1.0f,1.0f};

        cool_object->transform = glm::translate(glm::mat4(1.f), objcenter);
        //cool_object->transform = glm::scale(cool_object->transform,sc);

    }

    std::cout << "ended" << std::endl;

    engine.cleanup();

    
    return 0;

}