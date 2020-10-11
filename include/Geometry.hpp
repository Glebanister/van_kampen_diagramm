#pragma once

#include <cmath>

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
} // namespace van_kampen
