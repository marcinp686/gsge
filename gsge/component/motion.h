#pragma once

#include <DirectXMath.h>

namespace component
{

// Motion description of an object
// Velocity/speed of rotation unit is units/s and degrees/s

struct motion
{
    DirectX::XMFLOAT4A velocity = {0.0f, 0.0f, 0.0f, 0.0f}; // w is ignored
    DirectX::XMFLOAT4A rotation = {0.0f, 0.0f, 0.0f, 0.0f}; // w is ignored
};
} // namespace component
