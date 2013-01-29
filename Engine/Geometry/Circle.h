/* Copyright 2011 Jukka Jyl�nki

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

/** @file Circle.h
	@author Jukka Jyl�nki
	@brief The Circle geometry object. */
#pragma once

#include "../Math/MathFwd.h"
#include "../Math/float3.h"

MATH_BEGIN_NAMESPACE

/// A two-dimensional circle in 3D space.
/** This class represents both a hollow circle (only edge) and a solid circle (disc). */
class Circle
{
public:
	/// The center position of this circle.
	float3 pos;

	/// The normal direction of this circle. [similarOverload: pos]
	/** A circle is a two-dimensional object in 3D space. This normal vector (together with the pos member) 
		specifies the plane in which this circle lies in.
		This vector is always normalized. If you assign to this member directly, be sure to only assign normalized vectors. */
	float3 normal;

	/// The radius of the circle. [similarOverload: pos]
	/** This parameter must be strictly positive to specify a non-degenerate circle. If zero is specified, this circle
		is considered to be degenerate.
		@see Circle::Circle(). */
	float r;

	/// The default constructor does not initialize any members of this class.
	/** This means that the values of the members pos, normal and r are all undefined after creating a new circle using 
		this default constructor. Remember to assign to them before use.
		@see pos, normal, r. */
	Circle() {}

	/// Constructs a new circle by explicitly specifying the member variables.
	/** @param center The center point of the circle.
		@param normal The direction vector that specifies the orientation of this circle. This vector must be normalized,
			this constructor will not normalize the vector for you (for performance reasons).
		@param radius The radius of the circle.
		@see pos, normal, r. */
	Circle(const float3 &center, const float3 &normal, float radius);

	/// Returns a normalized direction vector to the 'U direction' of the circle.
	/** This vector lies on the plane of this circle.
		The U direction specifies the first basis vector of a local space of this circle. */
	float3 BasisU() const;

	/// Returns a normalized direction vector to the 'V direction' of the circle.
	/** This vector lies on the plane of this circle.
		The U direction specifies the second basis vector of a local space of this circle. */
	float3 BasisV() const;

	/// Returns a point at the edge of this circle.
	/** @param angleRadians The direction of the point to get. A full circle is generated by the range [0, 2*pi], 
			but it is ok to pass in values outside this range.
		@note This function is equivalent to calling GetPoint(float angleRadians, float d) with a value of d == 1.
		@return A point in world space at the edge of this circle. */
	float3 GetPoint(float angleRadians) const;

	/// Returns a point inside this circle.
	/** @param angleRadians The direction of the point to get. A full circle is generated by the range [0, 2*pi], 
			but it is ok to pass in values outside this range.
		@param d A value in the range [0,1] that specifies the normalzied distance of the point from the center of the circle.
			A value of 0 returns the center point of this circle. A value of 1 returns a point at the edge of this circle.
			The range of d is not restricted, so it is ok to pass in values larger than 1 to generate a point lying completely
			outside this circle. */
	float3 GetPoint(float angleRadians, float d) const;

	/// Returns the center point of this circle.
	/** This point is also the center of mass for this circle. The functions CenterPoint() and Centroid() are equivalent.
		@see pos. */
	float3 CenterPoint() const { return pos; }
	float3 Centroid() const { return pos; } ///< [similarOverload: CenterPoint]

	/// Computes the plane this circle is contained in.
	/** All the points of this circle lie inside this plane.
		@see class Plane. */
	Plane ContainingPlane() const;

	/// Tests if the given point is contained at the edge of this circle.
	/** @param maxDistance The epsilon threshold to test the distance against. This effectively turns the circle into a torus
			for this test.
		@see DistanceToEdge(), DistanceToDisc(), ClosestPointToEdge(), ClosestPointToDisc().
		@todo Implement DiscContains(float3/LineSegment/Triangle). */
	bool EdgeContains(const float3 &point, float maxDistance = 1e-6f) const;

	// Returns true if the given point lies inside this filled circle.
	// @param maxDistance The epsilon threshold to test the distance against.
//	bool DiscContains(const float3 &point, float maxDistance = 1e-6f) const;
//	bool DiscContains(const LineSegment &lineSegment, float maxDistance = 1e-6f) const;

	/// Computes the distance of the given object to the edge of this circle.
	/** @todo Implement DistanceToEdge(Ray/LineSegment/Line).
		@return The distance of the given point to the edge of this circle. If the point is contained on this circle,
			the value 0 is returned.
		@see DistanceToDisc(), ClosestPointToEdge(), ClosestPointToDisc(). */
	float DistanceToEdge(const float3 &point) const;
//	float DistanceToEdge(const Ray &ray, float *d, float3 *closestPoint) const;
//	float DistanceToEdge(const LineSegment &lineSegment, float *d, float3 *closestPoint) const;
//	float DistanceToEdge(const Line &line, float *d, float3 *closestPoint) const;

	/// Computes the distance of the given object to this disc (filled circle).
	/** If the point is contained inside this disc, the value 0 is returned.
		@see DistanceToEdge(), ClosestPointToEdge(), ClosestPointToDisc().
		@todo Implement DistanceToDisc(Ray/LineSegment/Line). */
	float DistanceToDisc(const float3 &point) const;
/*
	float DistanceToDisc(const Ray &ray, float *d, float3 *closestPoint) const;
	float DistanceToDisc(const LineSegment &lineSegment, float *d, float3 *closestPoint) const;
	float DistanceToDisc(const Line &line, float *d, float3 *closestPoint) const;
*/
	/// Computes the closest point on the edge of this circle to the given point.
	/** @todo Implement ClosestPointToEdge(Ray/LineSegment/Line).
		@see DistanceToEdge(), DistanceToDisc(), ClosestPointToDisc(). */
	float3 ClosestPointToEdge(const float3 &point) const;
//	float3 ClosestPointToEdge(const Ray &ray, float *d) const;
//	float3 ClosestPointToEdge(const LineSegment &lineSegment, float *d) const;
//	float3 ClosestPointToEdge(const Line &line, float *d) const;

	/// Computes the closest point on the disc of this circle to the given object.
	/** @todo Implement ClosestPointToDisc(Ray/LineSegment/Line).
		@see DistanceToEdge(), DistanceToDisc(), ClosestPointToEdge(). */
	float3 ClosestPointToDisc(const float3 &point) const;

	/// Tests this circle for an intersection against the given plane.
	/** @note For Circle-Plane intersection, there is no need to differentiate between a hollow or a filled circle (disc).
		@return The number of intersection points found for this circle and the given plane.
		@see IntersectsDisc(). */
	int Intersects(const Plane &plane, float3 *pt1, float3 *pt2) const;
	int Intersects(const Plane &plane) const;

	/// Tests this disc for an intersection against the given object.
	/** @see Intersects(). */
	bool IntersectsDisc(const Line &line) const;
	bool IntersectsDisc(const LineSegment &lineSegment) const;
	bool IntersectsDisc(const Ray &ray) const;

#ifdef MATH_ENABLE_STL_SUPPORT
	/// Returns a human-readable representation of this circle. Most useful for debugging purposes.
	/** The returned string specifies the center position, normal direction and the radius of this circle. */
	std::string ToString() const;
#endif
#ifdef MATH_QT_INTEROP
	operator QString() const { return toString(); }
	QString toString() const { return QString::fromStdString(ToString()); }
#endif
};

MATH_END_NAMESPACE

#ifdef MATH_QT_INTEROP
Q_DECLARE_METATYPE(Circle)
Q_DECLARE_METATYPE(Circle*)
#endif
