#include "objbase.h"
#include "node.h"
#include "hitinfo.h"

Interaction Object::Sample() const 
{
	Interaction it = Interaction();
	return it;
};

void Object::TransformInteractionToWorld(Interaction& it) const
{
	it.p = parent->TransformPointToWorld(it.p);
	it.n = (parent->TransformPointToWorld(it.n) - parent->TransformPointToWorld(Vec3f(0.0f, 0.0f, 0.0f))).GetNormalized();
}
