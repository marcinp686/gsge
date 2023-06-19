#pragma once

#include <DirectXMath.h>

namespace component
{
struct alignas(32) transform
{
    DirectX::XMFLOAT4A position{0.f, 0.f, 0.f, 0.0f};   // in units, w is ignored
    DirectX::XMFLOAT4A rotation{0.f, 0.f, 0.f, 0.0f};   // in radians, w is ignored
    DirectX::XMFLOAT4A scale{1.0f, 1.0f, 1.0f, 0.0f};   // in units, w is ignored   
};
} // namespace component
