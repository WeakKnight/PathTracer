#include "lightcomponent.h"
#include "cyColor.h"
#include "utils.h"

cy::Color LightComponent::Le() const
{
	return intensity;
}

// 给MIS用的,已经确定击中的情况下,计算一下对应的Le
cy::Color LightComponent::ComputeLe(const Vec3f& lightPos, const Vec3f& lightNormal , const Vec3f& objPos, const Vec3f& wi) const
{
	auto obj = parent->GetNodeObj();
	float area = obj->Area();
	float cosNormalDotNeggativeWi = Max(-wi.Dot(lightNormal), 0.0001f);
	float pdf = (lightPos - objPos).LengthSquared() / (area * cosNormalDotNeggativeWi);
	return Le() / pdf;
}

float LightComponent::Pdf(const HitInfo& hitInfo, const Interaction& sampleInteraction, float distance)
{
	auto obj = parent->GetNodeObj();
	float distanceSquare = distance * distance;
	float area = obj->Area();

	Vec3f neggativeWi = (hitInfo.p - sampleInteraction.p).GetNormalized();

	const Vec3f& normal = sampleInteraction.n;
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
		Interaction lightInter;
		lightInter.n = lightHitInfo.N;
		lightInter.p = lightHitInfo.p;

		return Pdf(hitInfo, lightInter, (lightHitInfo.p - hitInfo.p).Length());
	}
}

cy::Color LightComponent::SampleLi(const HitInfo& hitInfo, float& pdf, Vec3f& wi)
{
	auto obj = parent->GetNodeObj();
	Interaction it = obj->Sample();
	auto& samplePoint = it.p;

	wi = (samplePoint - hitInfo.p).GetNormalized();

	float distance = (hitInfo.p - samplePoint).Length();

	pdf = Pdf(hitInfo, it, distance);

	HitInfo lightHitInfo;
	// test visibility
	if (LightVisTest(Ray(hitInfo.p + hitInfo.N * INTERSECTION_BIAS, wi), lightHitInfo,(hitInfo.p - samplePoint).Length(), parent))
	{
		return Color::Black();
	}

	return Le();
}