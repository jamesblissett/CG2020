#include <algorithm>
#include <fstream>
#include <map>
#include <vector>

#include <glm/vec3.hpp>

#include <CanvasPoint.h>
#include <CanvasTriangle.h>
#include <Colour.h>
#include <DrawingWindow.h>
#include <ModelTriangle.h>
#include <Utils.h>

#include <unistd.h>

#define WIDTH 320
#define HEIGHT 240

glm::vec3 globalCameraPosition = glm::vec3(0, 0, 4);

uint32_t packColour(Colour colour) {
    return (255 << 24) + (int(colour.red) << 16) + (int(colour.green) << 8) + int(colour.blue);
}

std::array<std::array<float, HEIGHT>, WIDTH> depthBuffer;

// we lose some accuracy in the numbers when we convert from the strings to the
// floats
std::vector<ModelTriangle> readOBJFile(std::string filename, float sf, std::map<std::string, Colour> palette) {
    std::ifstream file(filename);
    std::string line;

    std::vector<ModelTriangle> triangles;
    std::vector<glm::vec3> vertices;
    std::string currentColourName;

    if (file.is_open()) {
        while (getline(file, line)) {
            /* std::cout << line << std::endl; */
            std::vector<std::string> splitLine = split(line, ' ');

            if (splitLine[0] == "usemtl") {
                currentColourName = splitLine[1];
            } else if (splitLine[0] == "v") {
                float x = stof(splitLine[1], NULL) * sf;
                float y = stof(splitLine[2], NULL) * sf;
                float z = stof(splitLine[3], NULL) * sf;
                vertices.push_back(glm::vec3(stof(splitLine[1], NULL) * sf, stof(splitLine[2], NULL) * sf, stof(splitLine[3], NULL) * sf));
            } else if (splitLine[0][0] == 'f') {
                std::vector<int> currentFaceVertices;
                for (int i = 1; i < splitLine.size(); i++) {
                    std::vector<std::string> splitSlashLine = split(splitLine[i], '/');
                    currentFaceVertices.push_back(stoi(splitSlashLine[0], NULL));
                }

                triangles.push_back(ModelTriangle(vertices[currentFaceVertices[0] - 1],
                                                  vertices[currentFaceVertices[1] - 1],
                                                  vertices[currentFaceVertices[2] - 1],
                                                  palette[currentColourName]));
            }
        }
    }

    file.close();

    return triangles;
}


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

/* void drawLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, Colour colour) { */
/*     float xDiff = to.x - from.x; */
/*     float yDiff = to.y - from.y; */
/*     float nSteps = fmax(abs(xDiff), abs(yDiff)); */
/*     float xStep = xDiff / nSteps; */
/*     float yStep = yDiff / nSteps; */

/*     uint32_t colourValue = (255 << 24) + (int(colour.red) << 16) + (int(colour.green) << 8) + int(colour.blue); */

/*     for (float i = 0.0; i < nSteps; i++) { */
/*         float x = from.x + i * xStep; */
/*         float y = from.y + i * yStep; */
/*         /1* std::cout << x << ", " << y << std::endl; *1/ */
/*         window.setPixelColour(round(x), round(y), colourValue); */
/*     } */
/* } */

void drawLine(DrawingWindow &window, CanvasPoint start, CanvasPoint end, Colour colour) {
    // start = CanvasPoint(round(start.x), round(start.y));
    // end = CanvasPoint(round(end.x), round(end.y));
    int noSteps = std::max(std::abs(start.x - end.x), std::abs(start.y - end.y)) + 1;
    // std::vector<float> xs = interpolateSingleFloats(start.x, end.x, noSteps);
    // std::vector<float> ys = interpolateSingleFloats(start.y, end.y, noSteps);
    std::vector<glm::vec3> vs = interpolateThreeElementValues(glm::vec3(start.x, start.y, start.depth), glm::vec3(end.x, end.y, end.depth), noSteps);

    float red = colour.red;
    float green = colour.green;
    float blue = colour.blue;
    uint32_t col = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);

    for(int i=0; i<noSteps; i++){
        /* std::cout << vs.at(i)[2] << std::endl; */
        if (vs.at(i)[2] < depthBuffer[std::round(vs.at(i)[0])][std::round(vs.at(i)[1])]) {
            depthBuffer[std::round(vs.at(i)[0])][std::round(vs.at(i)[1])] = vs.at(i)[2];
            window.setPixelColour(std::round(vs.at(i)[0]), std::round(vs.at(i)[1]), col);
         }
        //  window.renderFrame();
         // usleep(5000);
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
    if (triangle.v0().y > triangle.v1().y) {
        std::swap(triangle.v0(), triangle.v1());
    }
    if (triangle.v1().y > triangle.v2().y) {
        std::swap(triangle.v1(), triangle.v2());
    }
    if (triangle.v0().y > triangle.v1().y) {
        std::swap(triangle.v0(), triangle.v1());
    }

    float red = colour.red;
    float green = colour.green;
    float blue = colour.blue;
    uint32_t col = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);

    CanvasPoint start = triangle.v0();
    CanvasPoint end = triangle.v2();
    std::vector<glm::vec3> Line1 = interpolateThreeElementValues(glm::vec3(start.x, start.y, triangle.v0().depth), glm::vec3(end.x, end.y,triangle.v2().depth), std::max(std::abs(start.x - end.x), std::abs(start.y - end.y)) + 1);
    end = triangle.v1();
    std::vector<glm::vec3> Line2 = interpolateThreeElementValues(glm::vec3(start.x, start.y,triangle.v0().depth), glm::vec3(end.x, end.y,triangle.v1().depth), std::max(std::abs(start.x - end.x), std::abs(start.y - end.y)) + 1);
    start = triangle.v1();
    end = triangle.v2();
    std::vector<glm::vec3> Line3 = interpolateThreeElementValues(glm::vec3(start.x, start.y,triangle.v1().depth), glm::vec3(end.x, end.y,triangle.v2().depth), std::max(std::abs(start.x - end.x), std::abs(start.y - end.y)) + 1);

    // std::vector<float> inter = interpolateSingleFloats(triangle.v0().x, triangle.v2().x, std::max(std::abs(triangle.v0().x - triangle.v2().x), std::abs(triangle.v0().y - triangle.v2().y)) + 1);

    // for (int i=0; i < Line1.size(); i++){
    //     for (int j=0; j < 3; j++) {
    //         // printf("%f", Line1.at(i)[j]);
    //         window.setPixelColour(std::round(Line1.at(i)[0]), std::round(Line1.at(i)[1]), col);
    //     }
    //     // printf(":%f:", inter.at(i));
    //     // printf("%f\n", inter.at(i) - Line1.at(i)[0]);
    // }

    /* printf("%f,%f:", triangle.v0().x, triangle.v0().y); */
    /* printf("%f,%f:", triangle.v1().x, triangle.v1().y); */
    /* printf("%f,%f\n", triangle.v2().x, triangle.v2().y); */

    int lineOneIndex = 0;
    int lineOneX = Line1.at(0)[0];
    int lineTwoIndex = 0;
    int lineTwoX = Line2.at(0)[0];
    int lineThreeIndex = 0;
    int lineThreeX = Line2.at(0)[0];
    for (int i = triangle.v0().y; i < triangle.v2().y; i++) {
        while (Line1.at(lineOneIndex)[1] < i) {
            lineOneIndex += 1;
            lineOneX = Line1.at(lineOneIndex)[0];
        }
        if (i < triangle.v1().y) {
            while (Line2.at(lineTwoIndex)[1] <= i) {
                lineTwoX = Line2.at(lineTwoIndex)[0];
                lineTwoIndex += 1;
            }
            // window.setPixelColour(std::round(lineOneX), i, col);
            // window.setPixelColour(std::round(lineTwoX), i, col);
            // printf("%d,%d\n", lineOneX, i);
            // printf("%f,%d\n", std::round(lineOneX), i);
            // printf("%d,%d\n", lineTwoX, i);
            // printf("%f,%d\n", std::round(lineTwoX), i);
            drawLine(window, CanvasPoint(lineOneX, i, Line1.at(lineOneIndex)[2]), CanvasPoint(lineTwoX, i, Line2.at(lineTwoIndex)[2]), colour);
        } else {
            while (Line3.at(lineThreeIndex)[1] <= i ) {
                lineThreeX = Line3.at(lineThreeIndex)[0];
                lineThreeIndex += 1;
            }
            // window.setPixelColour(std::round(lineOneX), i, col);
            // window.setPixelColour(std::round(lineThreeX), i, col);
            drawLine(window, CanvasPoint(lineOneX, i, Line1.at(lineOneIndex)[2]), CanvasPoint(lineThreeX, i, Line3.at(lineThreeIndex)[2]), colour);
        }
    }
    drawStrokedTriangle(window, triangle, colour);
}

std::map<std::string, Colour> readOBJMaterialFile(std::string filename) {
    std::map<std::string, Colour> palette;

    std::ifstream file(filename);
    std::string line;

    std::string currentColourName;

    if (file.is_open()) {
        while (getline(file, line)) {
            std::vector<std::string> splitLine = split(line, ' ');
            if (splitLine[0] == "newmtl") {
                currentColourName = splitLine[1];
            } else if (splitLine[0] == "Kd") {
                palette[currentColourName] = Colour(round(stof(splitLine[1], NULL) * 255), round(stof(splitLine[2], NULL) * 255), round(stof(splitLine[3], NULL) * 255));
            }
        }
    }

    return palette;
}

/* void drawFilledTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour) { */

/*     uint32_t colourValue = (255 << 24) + (int(colour.red) << 16) + (int(colour.green) << 8) + int(colour.blue); */
/*     // debug colours */
/*     uint32_t red = (255 << 24) + (int(255) << 16) + (int(0) << 8) + int(0); */
/*     uint32_t green = (255 << 24) + (int(0) << 16) + (int(255) << 8) + int(0); */
/*     uint32_t blue = (255 << 24) + (int(0) << 16) + (int(0) << 8) + int(255); */

/*     // sort triangle vertices by y coordinate */
/*     std::vector<CanvasPoint> points; */
/*     points.push_back(triangle.v0()); */
/*     points.push_back(triangle.v1()); */
/*     points.push_back(triangle.v2()); */

/*     std::sort(points.begin(), points.end(), compareCanvasPointsHeights); */

/*     /1* std::cout << points[0].y << ", " << points[1].y << ", " << points[2].y << std::endl; *1/ */

/*     float topBottomXDiff = points[2].x - points[0].x; */
/*     float topBottomYDiff = points[2].y - points[0].y; */
/*     float topBottomNSteps = fmax(abs(topBottomXDiff), abs(topBottomYDiff)); */

/*     float topMiddleXDiff = points[1].x - points[0].x; */
/*     float topMiddleYDiff = points[1].y - points[0].y; */
/*     float topMiddleNSteps = fmax(abs(topMiddleXDiff), abs(topMiddleYDiff)); */

/*     float middleBottomXDiff = points[2].x - points[1].x; */
/*     float middleBottomYDiff = points[2].y - points[1].y; */
/*     float middleBottomNSteps = fmax(abs(middleBottomXDiff), abs(middleBottomYDiff)); */

/*     std::vector<glm::vec3> topBottomPoints    = interpolateThreeElementValues(glm::vec3(points[0].x, points[0].y, 0), glm::vec3(points[2].x, points[2].y, 0), topBottomNSteps); */
/*     std::vector<glm::vec3> topMiddlePoints    = interpolateThreeElementValues(glm::vec3(points[0].x, points[0].y, 0), glm::vec3(points[1].x, points[1].y, 0), topMiddleNSteps); */
/*     std::vector<glm::vec3> middleBottomPoints = interpolateThreeElementValues(glm::vec3(points[1].x, points[1].y, 0), glm::vec3(points[2].x, points[2].y, 0), middleBottomNSteps); */

/*     float lastTopBottomY = 0; */
/*     float topBottomX = 0; */
/*     float topBottomY = 0; */
/*     for (int i = 0; i < topBottomPoints.size(); i++) { */
/*         topBottomX = topBottomPoints[i][0]; */

/*         lastTopBottomY = topBottomY; */
/*         topBottomY = topBottomPoints[i][1]; */

/*         /1* printf("topBottomY difference = %f\n", topBottomY - lastTopBottomY); *1/ */

/*         // reached the split point */
/*         if (round(topBottomY) >= round(points[1].y)) { */
/*             for (int j = middleBottomPoints.size() - 1; j >= 0; j--) { */
/*                 if (round(middleBottomPoints[j][1]) == round(topBottomY)) { */
/*                     /1* std::cout << "Line at " << round(topBottomY) << ", from " << round(topBottomX) << " to " << round(topBottomPoints[j][0]) << std::endl; *1/ */
/*                     drawLine(window, */
/*                              CanvasPoint(topBottomX, topBottomY), */
/*                              CanvasPoint(middleBottomPoints[j][0], middleBottomPoints[j][1]), */
/*                              colour); */
/*                     break; */
/*                 } */
/*             } */

/*         // before the split point */
/*         } else { */
/*             for (int j = topMiddlePoints.size() - 1; j >= 0; j--) { */
/*                 if (round(topMiddlePoints[j][1]) == round(topBottomY)) { */
/*                     /1* std::cout << "Line at " << round(topBottomY) << ", from " << round(topBottomX) << " to " << round(topMiddlePoints[j][0]) << std::endl; *1/ */
/*                     drawLine(window, */
/*                              CanvasPoint(topBottomX, topBottomY), */
/*                              CanvasPoint(topMiddlePoints[j][0], topMiddlePoints[j][1]), */
/*                              colour); */
/*                 } */
/*             } */
/*         } */
/*     } */
/* } */

// does the formula in worksheet 4
// returns a pair of coords in a vector (u, v)
CanvasPoint objectCoordToImagePlaneCoord(glm::vec3 vertex, float focalLength, glm::vec3 cameraPosition, glm::vec3 screenShiftVector, float scaleFactor) {
    /* std::cout << "============================" << std::endl; */
    /* std::cout << "Camera Position: " << cameraPosition[0] << cameraPosition[1] << cameraPosition[2] << std::endl; */
    glm::vec3 translatedVertex = (vertex - cameraPosition); // + screenShiftVector;
    /* std::cout << "Vertex :" << vertex[0] << vertex[1] << vertex[2] << std::endl; */
    /* std::cout << "TranslatedVertex :" << translatedVertex[0] << translatedVertex[1] << translatedVertex[2] << std::endl; */
    float u = ((-1 * scaleFactor * focalLength * (translatedVertex[0] / translatedVertex[2])) + (WIDTH / 2));
    float v = ((scaleFactor * focalLength * (translatedVertex[1] / translatedVertex[2])) + (HEIGHT / 2));
    /* printf("(%f, %f)\n", u, v); */
    /* std::cout << "============================" << std::endl; */

    return CanvasPoint(u, v, 1 / translatedVertex[2]);
}

void renderTriangles(DrawingWindow &window, std::vector<ModelTriangle> triangles, glm::vec3 cameraPosition, float focalLength) {
    glm::vec3 screenShiftVector = glm::vec3(0, 0, 0);

    for (const auto triangle : triangles) {
        CanvasTriangle translatedTriangle;
        std::vector<CanvasPoint> translatedTrianglePoints;

        for (const auto vertex : triangle.vertices) {
            CanvasPoint newPoint = objectCoordToImagePlaneCoord(vertex, focalLength, cameraPosition, screenShiftVector, WIDTH);
            translatedTrianglePoints.push_back(newPoint);
            /* std::cout << newPoint << std::endl; */
            /* window.setPixelColour(newPoint.x, newPoint.y, packColour(Colour(255, 255, 255))); */
        }
        translatedTriangle = CanvasTriangle(translatedTrianglePoints[0], translatedTrianglePoints[1], translatedTrianglePoints[2]);
        drawFilledTriangle(window, translatedTriangle, triangle.colour);
    }
}

void draw(DrawingWindow &window, std::vector<ModelTriangle> triangles) {
    window.clearPixels();

    for (int i = 0; i < WIDTH; i++) {
        for (int j = 0; j < HEIGHT; j++) {
            depthBuffer[i][j] = 0;
        }
    }

    /* for (size_t y = 0; y < window.height; y++) { */
    /*     for (size_t x = 0; x < window.width; x++) { */

    /*         float red = 0; */
    /*         float green = 0; */
    /*         float blue = 0; */

    /*         uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue); */

    /*         window.setPixelColour(x, y, colour); */
    /*     } */
    /* } */

    renderTriangles(window, triangles, globalCameraPosition, 2.0);
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
            case SDLK_UP:
                globalCameraPosition[1] -= 0.1;
                break;
            case SDLK_DOWN:
                globalCameraPosition[1] += 0.03;
                break;
            case SDLK_LEFT:
                globalCameraPosition[0] += 0.03;
                break;
            case SDLK_RIGHT:
                globalCameraPosition[0] -= 0.03;
                break;
            case SDLK_w:
                globalCameraPosition[2] -= 0.03;
                break;
            case SDLK_s:
                globalCameraPosition[2] += 0.03;
                break;
        }
    } else if (event.type == SDL_MOUSEBUTTONDOWN) window.savePPM("output.ppm");
}

int main(int argc, char *argv[]) {

    srand(time(NULL));

    /* std::map<std::string, Colour> palette = readOBJMaterialFile("/home/james/ForJames.obj.mtl"); */
    /* std::vector<ModelTriangle> triangles = readOBJFile("/home/james/ForJames.obj", 0.17, palette); */

    std::map<std::string, Colour> palette = readOBJMaterialFile("cornell-box.mtl");
    std::vector<ModelTriangle> triangles = readOBJFile("cornell-box.obj", 0.17, palette);

    /* for (const auto pair : palette) { */
    /*     std::cout << pair.first << ": " << pair.second << std::endl; */
    /* } */

    /* for (int i = 0; i < triangles.size(); i++) { */
    /*     std::cout << triangles[i] << std::endl; */
    /*     std::cout << triangles[i].colour << std::endl; */
    /* } */


    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
    SDL_Event event;
    while (true) {
        // We MUST poll for events - otherwise the window will freeze !
        if (window.pollForInputEvents(event)) handleEvent(event, window);
        update(window);
        draw(window, triangles);
        // Need to render the frame at the end, or nothing actually gets shown on the screen !
        window.renderFrame();
    }
}
