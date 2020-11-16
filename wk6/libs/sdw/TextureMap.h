#pragma once

#include <iostream>
#include <fstream>
#include <stdexcept>
#include "Utils.h"
#include "Colour.h"

class TextureMap {
public:
	size_t width;
	size_t height;
	std::vector<uint32_t> pixels;
    Colour colour;

	TextureMap();
	TextureMap(const std::string &filename);
	friend std::ostream &operator<<(std::ostream &os, const TextureMap &point);
};
