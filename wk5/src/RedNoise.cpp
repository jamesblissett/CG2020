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
#include <TextureMap.h>
#include <TexturePoint.h>
#include <Utils.h>

#include <unistd.h>
#include <math.h>

#define WIDTH 320
#define HEIGHT 240

glm::vec3 globalCameraPosition = glm::vec3(0, 0, 4);
glm::mat3 cameraOrientationMatrix = glm::mat3(1, 0, 0, 0, 1, 0, 0, 0, 1);
std::array<std::array<float, HEIGHT>, WIDTH> depthBuffer;

uint32_t packColour(Colour colour) {
    return (255 << 24) + (int(colour.red) << 16) + (int(colour.green) << 8) + int(colour.blue);
}

// we lose some accuracy in the numbers when we convert from the strings to the
// floats
std::vector<ModelTriangle> readOBJFile(std::string filename, float sf, std::map<std::string, TextureMap> palette) {
    std::ifstream file(filename);
    std::string line;

    std::vector<ModelTriangle> triangles;
    std::vector<glm::vec3> vertices;
    std::vector<TexturePoint> texturePoints;
    std::string currentColourName;

    if (file.is_open()) {
        while (getline(file, line)) {
            /* std::cout << line << std::endl; */
            std::vector<std::string> splitLine = split(line, ' ');

            if (splitLine[0] == "usemtl") {
                currentColourName = splitLine[1];
            } else if (splitLine[0] == "v") {
                vertices.push_back(glm::vec3(stof(splitLine[1], NULL) * sf, stof(splitLine[2], NULL) * sf, stof(splitLine[3], NULL) * sf));
            } else if (splitLine[0] == "vt") {
                texturePoints.push_back(TexturePoint(stof(splitLine[1], NULL) * palette[currentColourName].width, stof(splitLine[2], NULL) * palette[currentColourName].height));
            } else if (splitLine[0][0] == 'f') {
                std::vector<int> currentFaceVertices;
                std::vector<int> currentTexturePoints;
                for (int i = 1; i < splitLine.size(); i++) {
                    std::vector<std::string> splitSlashLine = split(splitLine[i], '/');
                    currentFaceVertices.push_back(stoi(splitSlashLine[0], NULL));

                    if (splitSlashLine[1] != "") currentTexturePoints.push_back(stoi(splitSlashLine[1], NULL));
                }
                ModelTriangle triangle = ModelTriangle(vertices[currentFaceVertices[0] - 1],
                                                       vertices[currentFaceVertices[1] - 1],
                                                       vertices[currentFaceVertices[2] - 1],
                                                       palette[currentColourName].colour);
                triangle.material = currentColourName;

                if (currentTexturePoints.size() >= 3) {
                    triangle.texturePoints[0] = texturePoints[currentTexturePoints[0] - 1];
                    triangle.texturePoints[1] = texturePoints[currentTexturePoints[1] - 1];
                    triangle.texturePoints[2] = texturePoints[currentTexturePoints[2] - 1];
                }
                triangles.push_back(triangle);
            }
        }
    }

    file.close();

    return triangles;
}

std::map<std::string, TextureMap> readOBJMaterialFile(std::string filename) {
    std::map<std::string, TextureMap> palette;

    std::ifstream file(filename);
    std::string line;

    std::string currentColourName;

    if (file.is_open()) {
        while (getline(file, line)) {
            std::vector<std::string> splitLine = split(line, ' ');
            if (splitLine[0] == "newmtl") {
                currentColourName = splitLine[1];
            } else if (splitLine[0] == "Kd") {
                TextureMap tm;
                tm.colour = Colour(round(stof(splitLine[1], NULL) * 255), round(stof(splitLine[2], NULL) * 255), round(stof(splitLine[3], NULL) * 255));
                palette[currentColourName] = tm;
            } else if (splitLine[0] == "map_Kd") {
                TextureMap tm = TextureMap(splitLine[1]);
                tm.colour = Colour("NOT A COLOUR", 0, 0, 0);
                palette[currentColourName] = tm;
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
            if (vs.at(i)[2] <= depthBuffer[std::round(vs.at(i)[0])][std::round(vs.at(i)[1])]) {
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

void drawTexturedLine(DrawingWindow &window, CanvasPoint startParam, CanvasPoint endParam, TextureMap tm) {
    CanvasPoint start = CanvasPoint((int) startParam.x, (int) startParam.y, startParam.depth);
    CanvasPoint end = CanvasPoint((int) endParam.x, (int) endParam.y, endParam.depth);
    start.texturePoint = TexturePoint(startParam.texturePoint.x, startParam.texturePoint.y);
    end.texturePoint = TexturePoint(endParam.texturePoint.x, endParam.texturePoint.y);
    int noSteps = std::max(std::abs(start.x - end.x), std::abs(start.y - end.y)) + 1;
    std::vector<glm::vec3> vs = interpolateThreeElementValues(
           glm::vec3(start.x, start.y, start.depth), glm::vec3(end.x, end.y, end.depth), noSteps);
    std::vector<glm::vec3> texvs = interpolateThreeElementValues(glm::vec3(start.texturePoint.x, start.texturePoint.y, 0), glm::vec3(end.texturePoint.x, end.texturePoint.y, 0), noSteps);
    for(int i=0; i<noSteps; i++){
        if (vs.at(i)[2] <= depthBuffer[std::round(vs.at(i)[0])][std::round(vs.at(i)[1])]) {
            depthBuffer[std::round(vs.at(i)[0])][std::round(vs.at(i)[1])] = vs.at(i)[2];
            window.setPixelColour(std::round(vs.at(i)[0]), std::round(vs.at(i)[1]), tm.pixels[tm.width * std::round(texvs.at(i)[1]) + std::round(texvs.at(i)[0])]);
        }
   }
}

void drawTexturedTriangle(DrawingWindow &window, CanvasTriangle triangle, TextureMap tm) {
    std::cout << triangle.v0().texturePoint << std::endl;
    std::cout << triangle.v1().texturePoint << std::endl;
    std::cout << triangle.v2().texturePoint << std::endl;
    if (triangle.v0().y > triangle.v1().y) {
       std::swap(triangle.v0(), triangle.v1());
    }
    if (triangle.v1().y > triangle.v2().y) {
       std::swap(triangle.v1(), triangle.v2());
    }
    if (triangle.v0().y > triangle.v1().y) {
       std::swap(triangle.v0(), triangle.v1());
    }
    CanvasPoint start = triangle.v0();
    CanvasPoint end = triangle.v2();
    std::vector<glm::vec3> Line1 = interpolateThreeElementValues(glm::vec3(start.x, start.y, start.depth), glm::vec3(end.x, end.y, end.depth), std::max(std::abs(start.x - end.x), std::abs(start.y - end.y)) + 1);
    end = triangle.v1();
    std::vector<glm::vec3> Line2 = interpolateThreeElementValues(glm::vec3(start.x, start.y,start.depth), glm::vec3(end.x, end.y,end.depth), std::max(std::abs(start.x - end.x), std::abs(start.y - end.y)) + 1);
    start = triangle.v1();
    end = triangle.v2();
    std::vector<glm::vec3> Line3 = interpolateThreeElementValues(glm::vec3(start.x, start.y,start.depth), glm::vec3(end.x, end.y,end.depth), std::max(std::abs(start.x - end.x), std::abs(start.y - end.y)) + 1);
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
         float percentageLine1 = (i - triangle.v0().y)/(triangle.v2().y - triangle.v0().y);
         float texLine1X = triangle.v0().texturePoint.x + (triangle.v2().texturePoint.x - triangle.v0().texturePoint.x) * percentageLine1;
         float texLine1Y = triangle.v0().texturePoint.y + (triangle.v2().texturePoint.y - triangle.v0().texturePoint.y) * percentageLine1;
         if (i < triangle.v1().y) {
             while (Line2.at(lineTwoIndex)[1] < i) {
                lineTwoIndex += 1;
             }
             if (lineTwoIndex == Line2.size()) {
                 lineTwoIndex--;
             }
             CanvasPoint s = CanvasPoint(Line1.at(lineOneIndex)[0], i, Line1.at(lineOneIndex)[2]);
             CanvasPoint e = CanvasPoint(Line2.at(lineTwoIndex)[0], i, Line2.at(lineTwoIndex)[2]);
             float percentageLine2 = (i - triangle.v0().y)/(triangle.v1().y - triangle.v0().y);
             float texLine2X = triangle.v0().texturePoint.x + (triangle.v1().texturePoint.x - triangle.v0().texturePoint.x) * percentageLine2;
             float texLine2Y = triangle.v0().texturePoint.y + (triangle.v1().texturePoint.y - triangle.v0().texturePoint.y) * percentageLine2;
             s.texturePoint = TexturePoint(texLine1X, texLine1Y);
             e.texturePoint = TexturePoint(texLine2X, texLine2Y);
             drawTexturedLine(window, s, e, tm);
         } else {
             while (Line3.at(lineThreeIndex)[1] < i ) {
                 lineThreeIndex += 1;
             }
             if(lineThreeIndex == Line3.size()) {
                 lineThreeIndex--;
             }
             CanvasPoint s = CanvasPoint(Line1.at(lineOneIndex)[0], i, Line1.at(lineOneIndex)[2]);
             CanvasPoint e = CanvasPoint(Line3.at(lineThreeIndex)[0], i, Line3.at(lineThreeIndex)[2]);
             float percentageLine3 = (i - triangle.v1().y)/(triangle.v2().y - triangle.v1().y);
             float texLine3X = triangle.v1().texturePoint.x + (triangle.v2().texturePoint.x - triangle.v1().texturePoint.x) * percentageLine3;
             float texLine3Y = triangle.v1().texturePoint.y + (triangle.v2().texturePoint.y - triangle.v1().texturePoint.y) * percentageLine3;
             s.texturePoint = TexturePoint(texLine1X, texLine1Y);
             e.texturePoint = TexturePoint(texLine3X, texLine3Y);
             drawTexturedLine(window, s, e, tm);
      }
   }
}

// does the formula in worksheet 4
// returns a pair of coords in a vector (u, v)
CanvasPoint objectCoordToImagePlaneCoord(glm::vec3 vertex, float focalLength, glm::vec3 cameraPosition, glm::vec3 screenShiftVector, float scaleFactor) {
    /* std::cout << "============================" << std::endl; */
    /* std::cout << "Camera Position: " << cameraPosition[0] << cameraPosition[1] << cameraPosition[2] << std::endl; */
    glm::vec3 translatedVertex = (vertex - cameraPosition) * cameraOrientationMatrix; // + screenShiftVector;
    /* std::cout << "Vertex :" << vertex[0] << vertex[1] << vertex[2] << std::endl; */
    /* std::cout << "TranslatedVertex :" << translatedVertex[0] << translatedVertex[1] << translatedVertex[2] << std::endl; */
    float u = ((-1 * scaleFactor * focalLength * (translatedVertex[0] / translatedVertex[2])) + (WIDTH / 2));
    float v = ((scaleFactor * focalLength * (translatedVertex[1] / translatedVertex[2])) + (HEIGHT / 2));
    /* printf("(%f, %f)\n", u, v); */
    /* std::cout << "============================" << std::endl; */

    return CanvasPoint(u, v, 1 / translatedVertex[2]);
}

void renderTriangles(DrawingWindow &window, std::vector<ModelTriangle> triangles, glm::vec3 cameraPosition, float focalLength, std::map<std::string, TextureMap> palette) {
    glm::vec3 screenShiftVector = glm::vec3(0, 0, 0);

    for (const ModelTriangle & triangle : triangles) {
        CanvasTriangle translatedTriangle;
        std::vector<CanvasPoint> translatedTrianglePoints;

        for (int i = 0; i < 3; i++) {
            CanvasPoint newPoint = objectCoordToImagePlaneCoord(triangle.vertices[i], focalLength, cameraPosition, screenShiftVector, WIDTH);
            newPoint.texturePoint = triangle.texturePoints[i];
            translatedTrianglePoints.push_back(newPoint);
        }
        translatedTriangle = CanvasTriangle(translatedTrianglePoints[0], translatedTrianglePoints[1], translatedTrianglePoints[2]);
        if (palette[triangle.material].colour.name == "NOT A COLOUR") {
            drawTexturedTriangle(window, translatedTriangle, palette[triangle.material]);
        } else {
            drawFilledTriangle(window, translatedTriangle, triangle.colour);
        }
    }
}

void draw(DrawingWindow &window, std::vector<ModelTriangle> triangles, std::map<std::string, TextureMap> palette) {
    window.clearPixels();

    for (int i = 0; i < WIDTH; i++) {
        for (int j = 0; j < HEIGHT; j++) {
            depthBuffer[i][j] = 0;
        }
    }

    renderTriangles(window, triangles, globalCameraPosition, 2.0, palette);
}

void update(DrawingWindow &window) {
    // Function for performing animation (shifting artifacts or moving the camera)
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
    if (event.type == SDL_KEYDOWN) {
        Colour c;
        CanvasTriangle triangle;
        glm::mat3 rotMat;
        glm::mat3 oriMat;
        float angle = 1 * (M_PI / 180);
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
                rotMat = glm::mat3(1, 0, 0, 0, cos(angle), sin(angle), 0, -1 * sin(angle), cos(angle));
                globalCameraPosition = globalCameraPosition * rotMat;
                break;
            case SDLK_p:
                rotMat = glm::mat3(cos(angle), 0, -1 * sin(angle), 0, 1, 0, sin(angle), 0, cos(angle));
                globalCameraPosition = globalCameraPosition * rotMat;
                break;
            case SDLK_q:
                oriMat = glm::mat3(cos(angle), 0, -1 * sin(angle), 0, 1, 0, sin(angle), 0, cos(angle));
                cameraOrientationMatrix = cameraOrientationMatrix * oriMat;
                break;
            case SDLK_x:
                oriMat = glm::mat3(1, 0, 0, 0, cos(angle), sin(angle), 0, -1 * sin(angle), cos(angle));;
                cameraOrientationMatrix = cameraOrientationMatrix * oriMat;
                break;
            case SDLK_t:
                oriMat = glm::mat3(cos(angle), sin(angle), 0, -1 * sin(angle), cos(angle), 0, 0, 0, 1);
                cameraOrientationMatrix = cameraOrientationMatrix * oriMat;
                break;
            case SDLK_l:
                angle *= -1;
                break;
            /* case SDLK_q: */
            /*     rotMat = glm::mat3(cos(angle), sin(angle), 0, -1 * sin(angle), cos(angle), 0, 0, 0, 1); */
            /*     globalCameraPosition = globalCameraPosition * rotMat; */
            /*     break; */
        }
    } else if (event.type == SDL_MOUSEBUTTONDOWN) window.savePPM("output.ppm");
}

int main(int argc, char *argv[]) {

    srand(time(NULL));

    std::map<std::string, TextureMap> palette = readOBJMaterialFile("textured-cornell-box.mtl");
    std::vector<ModelTriangle> triangles = readOBJFile("textured-cornell-box.obj", 0.17, palette);

    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
    SDL_Event event;
    while (true) {
        // We MUST poll for events - otherwise the window will freeze !
        if (window.pollForInputEvents(event)) handleEvent(event, window);
        update(window);
        draw(window, triangles, palette);

        // Need to render the frame at the end, or nothing actually gets shown on the screen !
        window.renderFrame();
    }
}
