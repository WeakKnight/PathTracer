#pragma once

#include "cyMatrix.h"
#include "cyVector.h"

using namespace cy;

class Transformation
{
private:
	Matrix3f tm;            // Transformation matrix to the local space
	Vec3f    pos;           // Translation part of the transformation matrix
	mutable Matrix3f itm;   // Inverse of the transformation matrix (cached)

public:

	Matrix3f GetParentToLocalMatrix()
	{
		return tm;
	}

	Transformation() : pos(0, 0, 0) { tm.SetIdentity(); itm.SetIdentity(); }
	Matrix3f const& GetTransform() const { return tm; }
	Vec3f    const& GetPosition() const { return pos; }
	Matrix3f const& GetInverseTransform() const { return itm; }

	Vec3f TransformTo(Vec3f const& p) const { return itm * (p - pos); } // Transform to the local coordinate system
	Vec3f TransformFrom(Vec3f const& p) const { return tm * p + pos; }  // Transform from the local coordinate system

	// Transforms a vector to the local coordinate system (same as multiplication with the inverse transpose of the transformation)
	Vec3f VectorTransformTo(Vec3f const& dir) const { return TransposeMult(tm, dir); }

	// Transforms a vector from the local coordinate system (same as multiplication with the inverse transpose of the transformation)
	Vec3f VectorTransformFrom(Vec3f const& dir) const { return TransposeMult(itm, dir); }

	void Translate(Vec3f const& p) { pos += p; }
	void Rotate(Vec3f const& axis, float degrees) { Matrix3f m; m.SetRotation(axis, degrees * (float)M_PI / 180.0f); Transform(m); }
	void Scale(float sx, float sy, float sz) { Matrix3f m; m.Zero(); m[0] = sx; m[4] = sy; m[8] = sz; Transform(m); }
	void Transform(Matrix3f const& m) { tm = m * tm; pos = m * pos; tm.GetInverse(itm); }

	void InitTransform() {
		pos.Zero(); tm.SetIdentity(); itm.SetIdentity();
	}

private:
	// Multiplies the given vector with the transpose of the given matrix
	static Vec3f TransposeMult(Matrix3f const& m, Vec3f const& dir)
	{
		Vec3f d;
		d.x = m.GetColumn(0) % dir;
		d.y = m.GetColumn(1) % dir;
		d.z = m.GetColumn(2) % dir;
		return d;
	}
};
