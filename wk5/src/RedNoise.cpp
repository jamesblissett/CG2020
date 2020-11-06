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
#include <math.h>

#define WIDTH 320
#define HEIGHT 240

glm::vec3 globalCameraPosition = glm::vec3(0, 0, 4);
glm::vec3 angles = glm::vec3(0, 0, 0);
std::array<std::array<float, HEIGHT>, WIDTH> depthBuffer;

uint32_t packColour(Colour colour) {
    return (255 << 24) + (int(colour.red) << 16) + (int(colour.green) << 8) + int(colour.blue);
}

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

void drawLine(DrawingWindow &window, CanvasPoint start, CanvasPoint end, Colour colour) {
    start = CanvasPoint((int) start.x, (int) start.y, start.depth);
    end = CanvasPoint((int) end.x, (int) end.y, end.depth);
    int noSteps = std::max(std::abs(start.x - end.x), std::abs(start.y - end.y)) + 1;
    std::vector<glm::vec3> vs = interpolateThreeElementValues(glm::vec3(start.x, start.y, start.depth), glm::vec3(end.x, end.y, end.depth), noSteps);
    for(int i=0; i<noSteps; i++){
        if (std::round(vs.at(i)[0]) < WIDTH && std::round(vs.at(i)[0]) > 0 && std::round(vs.at(i)[1]) < HEIGHT && std::round(vs.at(i)[1]) > 0) {
            if (vs.at(i)[2] < depthBuffer[std::round(vs.at(i)[0])][std::round(vs.at(i)[1])]) {
                depthBuffer[std::round(vs.at(i)[0])][std::round(vs.at(i)[1])] = vs.at(i)[2];
                window.setPixelColour(std::round(vs.at(i)[0]), std::round(vs.at(i)[1]), packColour(colour));
            }
        }
    }
}

void drawStrokedTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour) {
    drawLine(window, triangle.v0(), triangle.v1(), colour);
    drawLine(window, triangle.v1(), triangle.v2(), colour);
    drawLine(window, triangle.v2(), triangle.v0(), colour);
}

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

   drawStrokedTriangle(window, triangle, colour);

   CanvasPoint start = triangle.v0();
   CanvasPoint end = triangle.v2();
   std::vector<glm::vec3> Line1 = interpolateThreeElementValues(glm::vec3(start.x, start.y, start.depth),
                                                                glm::vec3(end.x, end.y, end.depth),
                                                                std::max(std::abs(start.x - end.x), std::abs(start.y - end.y)) + 1);
   end = triangle.v1();
   std::vector<glm::vec3> Line2 = interpolateThreeElementValues(glm::vec3(start.x, start.y,start.depth),
                                                                glm::vec3(end.x, end.y,end.depth),
                                                                std::max(std::abs(start.x - end.x), std::abs(start.y - end.y)) + 1);
   start = triangle.v1();
   end = triangle.v2();
   std::vector<glm::vec3> Line3 = interpolateThreeElementValues(glm::vec3(start.x, start.y,start.depth),
                                                                glm::vec3(end.x, end.y,end.depth),
                                                                std::max(std::abs(start.x - end.x), std::abs(start.y - end.y)) + 1);
    int lineOneIndex = 1;
    int lineTwoIndex = 1;
    int lineThreeIndex = 1;
    for (int i = triangle.v0().y; i < triangle.v2().y; i++) {
      while (Line1.at(lineOneIndex)[1] < i) {
            lineOneIndex += 1;
        }
         if (lineOneIndex == Line1.size()) {
             lineOneIndex--;
         }
      if (i < triangle.v1().y) {
         while (Line2.at(lineTwoIndex)[1] < i) {
            lineTwoIndex += 1;
         }
         if (lineTwoIndex == Line2.size()) {
             lineTwoIndex--;
         }
         drawLine(window, CanvasPoint(Line1.at(lineOneIndex)[0], i, Line1.at(lineOneIndex)[2]), CanvasPoint(Line2.at(lineTwoIndex)[0], i, Line2.at(lineTwoIndex)[2]), colour);
      } else {
         while (Line3.at(lineThreeIndex)[1] < i ) {
            lineThreeIndex += 1;
         }
            if(lineThreeIndex == Line3.size()) {
                lineThreeIndex--;
            }
         drawLine(window, CanvasPoint(Line1.at(lineOneIndex)[0], i, Line1.at(lineOneIndex)[2]), CanvasPoint(Line3.at(lineThreeIndex)[0], i, Line3.at(lineThreeIndex)[2]), colour);
      }
   }
}

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

    for (const ModelTriangle & triangle : triangles) {
        CanvasTriangle translatedTriangle;
        std::vector<CanvasPoint> translatedTrianglePoints;

        for (const auto vertex : triangle.vertices) {
            CanvasPoint newPoint = objectCoordToImagePlaneCoord(vertex, focalLength, cameraPosition, screenShiftVector, WIDTH);
            translatedTrianglePoints.push_back(newPoint);
            /* std::cout << newPoint << std::endl; */
            /* window.setPixelColour(newPoint.x, newPoint.y, packColour(Colour(255, 255, 255))); */
        }
        translatedTriangle = CanvasTriangle(translatedTrianglePoints[0], translatedTrianglePoints[1], translatedTrianglePoints[2]);
        drawStrokedTriangle(window, translatedTriangle, triangle.colour);
    }
}

void draw(DrawingWindow &window, std::vector<ModelTriangle> triangles) {
    window.clearPixels();

    for (int i = 0; i < WIDTH; i++) {
        for (int j = 0; j < HEIGHT; j++) {
            depthBuffer[i][j] = 0;
        }
    }

    renderTriangles(window, triangles, globalCameraPosition, 2.0);
}

void update(DrawingWindow &window) {
    // Function for performing animation (shifting artifacts or moving the camera)
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
    if (event.type == SDL_KEYDOWN) {
        Colour c;
        CanvasTriangle triangle;
        glm::mat3 rotMat;
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
            case SDLK_r:
                angles[0] += 1 * (M_PI / 180);
                rotMat = glm::mat3(1, 0, 0, 0, cos(angles[0]), sin(angles[0]), 0, -1 * sin(angles[0]), cos(angles[0]));
                globalCameraPosition = globalCameraPosition * rotMat;
                break;
            case SDLK_p:
                angles[1] += 1 * (M_PI / 180);
                rotMat = glm::mat3(cos(angles[1]), 0, -1 * sin(angles[1]), 0, 1, 0, sin(angles[1]), 0, cos(angles[1]));
                globalCameraPosition = globalCameraPosition * rotMat;
                break;
            case SDLK_q:
                angles[2] += 1 * (M_PI / 180);
                rotMat = glm::mat3(cos(angles[2]), sin(angles[2]), 0, -1 * sin(angles[2]), cos(angles[2]), 0, 0, 0, 1);
                globalCameraPosition = globalCameraPosition * rotMat;
                break;
        }
    } else if (event.type == SDL_MOUSEBUTTONDOWN) window.savePPM("output.ppm");
}

int main(int argc, char *argv[]) {

    srand(time(NULL));

    std::map<std::string, Colour> palette = readOBJMaterialFile("cornell-box.mtl");
    std::vector<ModelTriangle> triangles = readOBJFile("cornell-box.obj", 0.17, palette);

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
