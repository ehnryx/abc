/**
 * 3d Primitives
 * value bound (X is max coordinate)
 * delaunay value bound (X is max x,y-coordinate)
 *  3d convex hull only uses `plane_point_orientation` and `collinear`
 * TODO get error estimates on all of these
 * STATUS
 *  untested
 */
#pragma once

#include "geometry/point3d.h"

/// p lies on the plane and normal is normal to the plane
/// a and b lie on the line
/// value bound: X^2
/// int value bound: X^2
template <typename T>
auto plane_line_inter(
    point3d<T> const& p, point3d<T> const& normal, point3d<T> const& a, point3d<T> const& b) {
  using I = typename point3d<T>::intersection_t;
  if constexpr (std::is_same_v<T, I>) {
    return a + (b - a) * (dot(normal, p - a) / dot(normal, b - a));
  } else {
    return a + (b - a) * (I(dot(normal, p - a)) / I(dot(normal, b - a)));
  }
}

/// a and b lie on the line
/// v is the point
/// value bound: X^4
/// int value bound: X^2
template <typename T>
auto line_point_dist(point3d<T> const& a, point3d<T> const& b, point3d<T> const& v) {
  using I = typename point3d<T>::intersection_t;
  if constexpr (std::is_same_v<T, I>) {
    return std::sqrt(norm(cross(b - a, v - a)) / norm(b - a));
  } else {
    return std::sqrt(norm(point3d<I>(cross(b - a, v - a))) / norm(b - a));
  }
}

/// p lines on the plane and normal is normal to the plane (not necessarily unit)
/// v is the point
/// if normal is unit, then plane-point distance is just dot(unit_normal, v - point_on_plane)
/// value bound: X^2
template <std::floating_point T>
auto plane_point_dist(point3d<T> const& p, point3d<T> const& normal, point3d<T> const& v) {
  return dot(normal, v - p) / abs(normal);
}

/// a, b, c describes the plane (must not be collinear)
/// upwards is consistent with the right-hand-rule on triangle abc
/// returns positive if v is above the plane, negative if v is below
/// value bound: X^4
/// int value bound: X^2 before converting to floating
template <typename T>
auto plane_point_dist(
    point3d<T> const& a, point3d<T> const& b, point3d<T> const& c, point3d<T> const& v) {
  using I = typename point3d<T>::intersection_t;
  return plane_point_dist<I>(a, cross(b - a, c - a), v);
}

/// checks how far v is from the plane abc. coplanar if that is <= eps
/// a, b, c describes the plane (must not be collinear)
/// upwards is consistent with the right-hand-rule on triangle abc
/// returns 1 if v is above the plane, -1 if v is below, 0 otherwise
/// value bound: X^4
/// TODO is this stable ?
template <std::floating_point T>
auto plane_point_orientation(
    epsilon<T> eps, point3d<T> const& a, point3d<T> const& b, point3d<T> const& c,
    point3d<T> const& v) {
  auto const normal = cross(b - a, c - a);
  auto const normalized_eps = epsilon<T>{abs(normal) * eps};
  return geometry::sign(normalized_eps, dot(v - (a + b + c) / T(3), normal));
}
/// int value bound: X^3
/// delaunay value bound: X^4
template <geometry::non_floating T>
auto plane_point_orientation(
    point3d<T> const& a, point3d<T> const& b, point3d<T> const& c, point3d<T> const& v) {
  auto const normal = cross(b - a, c - a);  // point3d<__int128> for delaunay
  return geometry::sign(normal.dot(v - a));
}

/// a and b lie on line 1
/// c and d lie on line 2
/// The lines must be skew
/// value bound: X^4
/// int value bound: X^2 before converting to floating
template <typename T>
auto line_line_dist(
    point3d<T> const& a, point3d<T> const& b, point3d<T> const& c, point3d<T> const& d) {
  using I = typename point3d<T>::intersection_t;
  return std::abs(plane_point_dist<I>(a, cross(a - b, c - d), c));
}

/// checks how far v is to the line ab. collinear if that value is <= eps
/// value bound: X^4
/// TODO is this stable ?
template <std::floating_point T>
bool collinear(epsilon<T> eps, point3d<T> const& a, point3d<T> const& b, point3d<T> const& v) {
  return geometry::equal(eps, a, b) or
         norm(cross(b - a, v - (a + b) / T(2))) <= eps * eps * norm(b - a);
}
/// int bound: X^2
/// delaunay value bound: X^3
template <geometry::non_floating T>
bool collinear(point3d<T> const& a, point3d<T> const& b, point3d<T> const& c) {
  return a == b or cross(b - a, c - a).is_zero();  // point3d<__int128> for delaunay
}

/// p lies on the plane and normal is normal to the plane
/// v is the point to project onto this plane
template <std::floating_point T>
auto project_to_plane(point3d<T> const& p, point3d<T> const& normal, point3d<T> const& v) {
  return v - normal * (normal.dot(v - p) / norm(normal));
}

/// a, b, c describes the plane (must not be collinear)
/// v is the point to project onto this plane
template <typename T>
auto project_to_plane(
    point3d<T> const& a, point3d<T> const& b, point3d<T> const& c, point3d<T> const& v) {
  using I = typename point3d<T>::intersection_t;
  return project_to_plane<I>(a, cross(b - a, c - a), v);
}

// TODO plane-plane intersection, parallel checking projection
