#pragma once

#include <cmath>
#include <stdexcept>
#include <vector>

namespace van_kampen
{
    // Point in 2-dimentional space
    struct Point
    {
        double x = 0.0, y = 0.0; // Coordinates
    };

    // Circle in 2-dimentional space
    struct Circle
    {
        Point center;        // Center of circle
        double radius = 1.0; // Radius of circle
    };

    // Arc - part of circle in 2-dimentional space
    struct Arc
    {
        Circle circle;           // Corresponding circle
        double beginAngle = 0.0; // Arc begin angle (radians)
        double range = M_PI;     // Arc length (radians)
    };

    // Point
    struct Segment
    {
        Point first, second; // End points of segment
    };

    inline Point operator+(const Point &first, const Point &second)
    {
        return {first.x + second.x, first.y + second.y};
    }

    inline Point operator-(const Point &first, const Point &second)
    {
        return {first.x - second.x, first.y - second.y};
    }

    inline Point operator*(const Point &p, double coef)
    {
        return {p.x * coef, p.y * coef};
    }

    inline Point operator/(const Point &p, double coef)
    {
        if (coef == 0)
        {
            throw std::invalid_argument("Can not divide point by zero");
        }
        return {p.x / coef, p.y / coef};
    }

    // Find half sum of two points
    // Returns result coordinates
    inline Point middleOf(const Point &first, const Point &second)
    {
        return (first + second) / 2.0;
    }

    // Generate polygon of vertCount verticies with radius shape.radius and center in shape.center
    // First vertex angle is startAngle
    // Returns vector of generated polygon
    inline std::vector<Point> polygonCoordinates(const Circle &shape, std::size_t vertCount, double startAngle = 0.0)
    {
        if (vertCount < 2)
        {
            throw std::invalid_argument("Can not create polygon coordinates with less than 2 verticies");
        }
        std::vector<Point> polygon(vertCount);
        double segment = 2.0 * M_PI / vertCount;
        std::size_t count = 0;
        for (double angle = startAngle; count < vertCount; ++count, angle += segment)
        {
            polygon[count] = Point{std::cos(angle) * shape.radius, std::sin(angle) * shape.radius} + shape.center;
        }
        return polygon;
    }

    // Returns distance between two points
    double distance(const Point &a, const Point &b)
    {
        return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
    }

    // Returns distance from seg to pt
    // Original code source: https://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
    double distance(const Segment &seg, const Point pt)
    {
        double x = pt.x, y = pt.y;
        double x1 = seg.first.x;
        double y1 = seg.first.y;
        double x2 = seg.second.x;
        double y2 = seg.second.y;
        double A = x - x1;
        double B = y - y1;
        double C = x2 - x1;
        double D = y2 - y1;
        double dot = A * C + B * D;
        double len_sq = C * C + D * D;
        double param = -1;
        if (len_sq != 0)
        {
            param = dot / len_sq;
        }

        double xx, yy;
        if (param < 0)
        {
            xx = x1;
            yy = y1;
        }
        else if (param > 1)
        {
            xx = x2;
            yy = y2;
        }
        else
        {
            xx = x1 + param * C;
            yy = y1 + param * D;
        }

        double dx = x - xx;
        double dy = y - yy;
        return std::sqrt(dx * dx + dy * dy);
    }
} // namespace van_kampen
