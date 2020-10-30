#include <algorithm>
#include <fstream>
#include <vector>

#include <glm/vec3.hpp>

#include <CanvasPoint.h>
#include <CanvasTriangle.h>
#include <Colour.h>
#include <DrawingWindow.h>
#include <Utils.h>

#include <unistd.h>

#define WIDTH 320
#define HEIGHT 240

std::vector<float> interpolateSingleFloats(float from, float to, int numberOfValues) {
    std::vector<float> out;

    if (from == to) {
        for (int i = 0; i < numberOfValues; i++) {
            out.push_back(from);
        }
        return out;
    }

    float step = (to - from) / (numberOfValues - 1);

    for (int i = 0; i < numberOfValues; i++) {
        out.push_back(from + i * step);
    }
    return out;
}

std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues) {
    std::vector<glm::vec3> out;

    if (from == to) {
        for (int i = 0; i < numberOfValues; i++) {
            out.push_back(from);
        }
        return out;
    }

    std::vector<float> steps;

    std::vector<float> xs = interpolateSingleFloats(from[0], to[0], numberOfValues);
    std::vector<float> ys = interpolateSingleFloats(from[1], to[1], numberOfValues);
    std::vector<float> zs = interpolateSingleFloats(from[2], to[2], numberOfValues);

    for (int i = 0; i < numberOfValues; i++) {
        out.push_back(glm::vec3(xs.at(i), ys.at(i), zs.at(i)));
    }

    return out;
}

void drawLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, Colour colour) {
    float xDiff = to.x - from.x;
    float yDiff = to.y - from.y;
    float nSteps = fmax(abs(xDiff), abs(yDiff));
    float xStep = xDiff / nSteps;
    float yStep = yDiff / nSteps;

    uint32_t colourValue = (255 << 24) + (int(colour.red) << 16) + (int(colour.green) << 8) + int(colour.blue);

    for (float i = 0.0; i < nSteps; i++) {
        float x = from.x + i * xStep;
        float y = from.y + i * yStep;
        /* std::cout << x << ", " << y << std::endl; */
        window.setPixelColour(round(x), round(y), colourValue);
    }
}

/* void drawLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, Colour colour) { */
/*     float xDiff = to.x - from.x; */
/*     float yDiff = to.y - from.y; */
/*     float nSteps = fmax(abs(xDiff), abs(yDiff)) + 1; */

/*     std::vector<glm::vec3> points = interpolateThreeElementValues(glm::vec3(from.x, from.y, 0), glm::vec3(to.x, to.y, 0), round(nSteps)); */

/*     uint32_t colourValue = (255 << 24) + (int(colour.red) << 16) + (int(colour.green) << 8) + int(colour.blue); */

/*     for (float i = 0.0; i < points.size(); i++) { */
/*         window.setPixelColour(round(points.at(i)[0]), round(points.at(i)[1]), colourValue); */
/*     } */
/* } */

void drawStrokedTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour) {
    drawLine(window, triangle.v0(), triangle.v1(), colour);
    drawLine(window, triangle.v1(), triangle.v2(), colour);
    drawLine(window, triangle.v2(), triangle.v0(), colour);
}

bool compareCanvasPointsHeights(CanvasPoint i, CanvasPoint j) { return i.y < j.y; }

void drawFilledTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour) {

    uint32_t colourValue = (255 << 24) + (int(colour.red) << 16) + (int(colour.green) << 8) + int(colour.blue);
    // debug colours
    uint32_t red = (255 << 24) + (int(255) << 16) + (int(0) << 8) + int(0);
    uint32_t green = (255 << 24) + (int(0) << 16) + (int(255) << 8) + int(0);
    uint32_t blue = (255 << 24) + (int(0) << 16) + (int(0) << 8) + int(255);

    // sort triangle vertices by y coordinate
    std::vector<CanvasPoint> points;
    points.push_back(triangle.v0());
    points.push_back(triangle.v1());
    points.push_back(triangle.v2());

    std::sort(points.begin(), points.end(), compareCanvasPointsHeights);

    /* std::cout << points[0].y << ", " << points[1].y << ", " << points[2].y << std::endl; */

    float topBottomXDiff = points[2].x - points[0].x;
    float topBottomYDiff = points[2].y - points[0].y;
    float topBottomNSteps = fmax(abs(topBottomXDiff), abs(topBottomYDiff));

    float topMiddleXDiff = points[1].x - points[0].x;
    float topMiddleYDiff = points[1].y - points[0].y;
    float topMiddleNSteps = fmax(abs(topMiddleXDiff), abs(topMiddleYDiff));

    float middleBottomXDiff = points[2].x - points[1].x;
    float middleBottomYDiff = points[2].y - points[1].y;
    float middleBottomNSteps = fmax(abs(middleBottomXDiff), abs(middleBottomYDiff));

    std::vector<glm::vec3> topBottomPoints    = interpolateThreeElementValues(glm::vec3(points[0].x, points[0].y, 0), glm::vec3(points[2].x, points[2].y, 0), topBottomNSteps);
    std::vector<glm::vec3> topMiddlePoints    = interpolateThreeElementValues(glm::vec3(points[0].x, points[0].y, 0), glm::vec3(points[1].x, points[1].y, 0), topMiddleNSteps);
    std::vector<glm::vec3> middleBottomPoints = interpolateThreeElementValues(glm::vec3(points[1].x, points[1].y, 0), glm::vec3(points[2].x, points[2].y, 0), middleBottomNSteps);

    float lastTopBottomY = 0;
    float topBottomX = 0;
    float topBottomY = 0;
    for (int i = 0; i < topBottomPoints.size(); i++) {
        topBottomX = topBottomPoints[i][0];

        lastTopBottomY = topBottomY;
        topBottomY = topBottomPoints[i][1];

        printf("topBottomY difference = %f\n", topBottomY - lastTopBottomY);

        // reached the split point
        if (round(topBottomY) >= round(points[1].y)) {
            for (int j = middleBottomPoints.size() - 1; j >= 0; j--) {
                if (round(middleBottomPoints[j][1]) == round(topBottomY)) {
                    /* std::cout << "Line at " << round(topBottomY) << ", from " << round(topBottomX) << " to " << round(topBottomPoints[j][0]) << std::endl; */
                    drawLine(window,
                             CanvasPoint(topBottomX, topBottomY),
                             CanvasPoint(middleBottomPoints[j][0], middleBottomPoints[j][1]),
                             colour);
                    break;
                }
            }

        // before the split point
        } else {
            for (int j = topMiddlePoints.size() - 1; j >= 0; j--) {
                if (round(topMiddlePoints[j][1]) == round(topBottomY)) {
                    /* std::cout << "Line at " << round(topBottomY) << ", from " << round(topBottomX) << " to " << round(topMiddlePoints[j][0]) << std::endl; */
                    drawLine(window,
                             CanvasPoint(topBottomX, topBottomY),
                             CanvasPoint(topMiddlePoints[j][0], topMiddlePoints[j][1]),
                             colour);
                }
            }
        }
    }
}

void draw(DrawingWindow &window) {
    /* window.clearPixels(); */

    for (size_t y = 0; y < window.height; y++) {
        for (size_t x = 0; x < window.width; x++) {

            float red = 0;
            float green = 0;
            float blue = 0;

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
        Colour c;
        CanvasTriangle triangle;
        switch (event.key.keysym.sym) {
            case SDLK_u:

                c = Colour(rand() % 255, rand() % 255, rand() % 255);
                triangle = CanvasTriangle(CanvasPoint(rand() % WIDTH, rand() % HEIGHT),
                                                         CanvasPoint(rand() % WIDTH, rand() % HEIGHT),
                                                         CanvasPoint(rand() % WIDTH, rand() % HEIGHT));

                drawStrokedTriangle(window, triangle, c);
                break;
            case SDLK_f:
                c = Colour(rand() % 255, rand() % 255, rand() % 255);
                triangle = CanvasTriangle(CanvasPoint(rand() % WIDTH, rand() % HEIGHT),
                                                         CanvasPoint(rand() % WIDTH, rand() % HEIGHT),
                                                         CanvasPoint(rand() % WIDTH, rand() % HEIGHT));

                /* triangle = CanvasTriangle(CanvasPoint(45, 87), CanvasPoint(120, 59), CanvasPoint(236, 89)); */
                /* triangle = CanvasTriangle(CanvasPoint(99, 182), CanvasPoint(53, 220), CanvasPoint(90, 219)); */
                /* triangle = CanvasTriangle(CanvasPoint(0, 0), CanvasPoint(290, 125), CanvasPoint(200, 7)); */
                std::cout << triangle << std::endl;
                drawFilledTriangle(window, triangle, c);
                drawStrokedTriangle(window, triangle, Colour(255, 255, 255));
            break;
        }
    } else if (event.type == SDL_MOUSEBUTTONDOWN) window.savePPM("output.ppm");
}

int main(int argc, char *argv[]) {

    srand(time(NULL));

    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
    SDL_Event event;
    while (true) {
        // We MUST poll for events - otherwise the window will freeze !
        if (window.pollForInputEvents(event)) handleEvent(event, window);
        update(window);
        /* draw(window); */
        // Need to render the frame at the end, or nothing actually gets shown on the screen !
        window.renderFrame();
    }
}
