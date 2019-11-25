#include "lightcomponent.h"
#include "cyColor.h"
#include "utils.h"

cy::Color LightComponent::Le()
{
	cy::Color result = cy::Color::White() * intensity;
	return result;
}

float LightComponent::Pdf(const HitInfo& hitInfo, const Vec3f& samplePoint, float distance)
{
	auto obj = parent->GetNodeObj();
	float distanceSquare = distance * distance;
	float area = obj->Area();

	Vec3f neggativeWi = (hitInfo.p - samplePoint).GetNormalized();

	Vec3f normal = obj->Normal(samplePoint);
	float cosNormalDotNeggativeWi = Max(neggativeWi.Dot(normal), 0.0001f);

	float pdf = distanceSquare / (area * cosNormalDotNeggativeWi);
	return pdf;
}

float LightComponent::Pdf(const HitInfo& hitInfo, const Vec3f& wi)
{
	HitInfo lightHitInfo;
	if (LightVisTest(Ray(hitInfo.p + hitInfo.N * INTERSECTION_BIAS, wi), lightHitInfo,
		BIGFLOAT, parent))
	{
		return 0.0f;
	}
	else
	{
		return Pdf(hitInfo, lightHitInfo.p, (lightHitInfo.p - hitInfo.p).Length());
	}
}

cy::Color LightComponent::SampleLi(const HitInfo& hitInfo, float& pdf, Vec3f& wi)
{
	auto obj = parent->GetNodeObj();
	auto samplePoint = obj->Sample();

	wi = (samplePoint - hitInfo.p).GetNormalized();

	float distance = (hitInfo.p - samplePoint).Length();

	pdf = Pdf(hitInfo, samplePoint, distance);

	HitInfo lightHitInfo;
	// test visibility
	if (LightVisTest(Ray(hitInfo.p + hitInfo.N * INTERSECTION_BIAS, wi), lightHitInfo,(hitInfo.p - samplePoint).Length(), parent))
	{
		return Color::Black();
	}

	return Le();
}