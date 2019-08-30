#include "materials.h"

Color MtlBlinn::Shade(Ray const &ray, const HitInfo &hInfo, const LightList &lights) const
{
    return Color::White();
}

void MtlBlinn::SetViewportMaterial(int subMtlID) const
{
}
