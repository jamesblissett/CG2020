#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/vec3.hpp>

#define WIDTH 320
#define HEIGHT 240

std::vector<float> interpolateSingleFloats(float from, float to, int numberOfValues) {
    std::vector<float> out;
    float step = (to - from) / (numberOfValues - 1);

    for (int i = 0; i < numberOfValues; i++) {
        out.push_back(from + i * step);
    }
    return out;
}

std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues) {
    std::vector<glm::vec3> out;
    std::vector<float> steps;

    for (int i = 0; i < 3; i++) steps.push_back((to[i] - from[i]) / (numberOfValues - 1));

    for (int i = 0; i < numberOfValues; i++) {
        glm::vec3 current;
        for (int j = 0; j < 3; j++) current[j] = from[j] + i * steps[j];

        out.push_back(current);
    }

    return out;
}

void draw(DrawingWindow &window) {
    window.clearPixels();

    /* std::vector<float> values = interpolateSingleFloats(0, 255, window.width); */

    glm::vec3 topLeft     = glm::vec3(255.0, 0.0, 0.0);
    glm::vec3 topRight    = glm::vec3(0.0, 0.0, 255.0);
    glm::vec3 bottomRight = glm::vec3(0.0, 255.0, 0.0);
    glm::vec3 bottomLeft  = glm::vec3(255.0, 255.0, 0.0);

    std::vector<glm::vec3> leftColumn = interpolateThreeElementValues(topLeft, bottomLeft, window.height);
    std::vector<glm::vec3> rightColumn = interpolateThreeElementValues(topRight, bottomRight, window.height);

    for (size_t y = 0; y < window.height; y++) {
        std::vector<glm::vec3> row = interpolateThreeElementValues(leftColumn[y], rightColumn[y], window.width);

        for (size_t x = 0; x < window.width; x++) {
            /* float red = values[window.width - x - 1]; */
            /* float green = values[window.width - x - 1]; */
            /* float blue = values[window.width - x - 1]; */
            /* uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue); */
            /* window.setPixelColour(x, y, colour); */

            float red = row[x][0];
            float green = row[x][1];
            float blue = row[x][2];
            uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
            window.setPixelColour(x, y, colour);
        }
    }
}

void update(DrawingWindow &window) {
    // Function for performing animation (shifting artifacts or moving the camera)
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
        else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
        else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
        else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
    } else if (event.type == SDL_MOUSEBUTTONDOWN) window.savePPM("output.ppm");
}

int main(int argc, char *argv[]) {

    /* std::vector<float> result; */
    /* result = interpolateSingleFloats(2.2, 8.5, 7); */
    /* for (int i = 0; i < result.size(); i++) std::cout << result[i] << " "; */
    /* std::cout << std::endl; */

    std::vector<glm::vec3> result;
    result = interpolateThreeElementValues(glm::vec3(1.0, 4.0, 9.2), glm::vec3(4.0, 1.0, 9.8), 4);
    for (int i = 0; i < result.size(); i++) {
        for (int j = 0; j < 3; j++) {
            std::cout << result[i][j] << " ";
        }
        std::cout << std::endl;
    }

    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
    SDL_Event event;
    while (true) {
        // We MUST poll for events - otherwise the window will freeze !
        if (window.pollForInputEvents(event)) handleEvent(event, window);
        update(window);
        draw(window);
        // Need to render the frame at the end, or nothing actually gets shown on the screen !
        window.renderFrame();
    }
}
