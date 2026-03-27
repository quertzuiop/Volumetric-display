#pragma once

#include "types.h"

BinaryColor dither(Color color, Vec3<float> pos);
BinaryColor dither(Color color, float ditherRank);