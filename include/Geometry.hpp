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

    inline Point operator+(const Point &first, const Point &second)
    {
        return {first.x + second.x, first.y + second.y};
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

    inline Point middleOf(const Point &first, const Point &second)
    {
        return (first + second) / 2.0;
    }

    inline std::vector<Point> polygonCoordinates(const Circle &shape, std::size_t vertCount)
    {
        if (vertCount < 2)
        {
            throw std::invalid_argument("Can not create polygon coordinates with less than 2 verticies");
        }
        std::vector<Point> polygon(vertCount);
        double segment = 2.0 * M_PI / vertCount;
        for (std::size_t i = 0; i < vertCount; ++i)
        {
            polygon[i] = {std::cos(segment * i), std::sin(segment + i)};
        }
        return polygon;
    }
} // namespace van_kampen
