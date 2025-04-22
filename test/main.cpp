#include "QuadTree.hpp"


int main()
{
    QuadTree qt(0.0, 0.0, 100.0, 100.0, 4, 4);
    qt.Insert(-10.0, -10.0);
    qt.Insert(10.0, 10.0);
    qt.Insert(-10.0, 10.0);
    qt.Insert(10.0, -10.0);

    std::vector<Point2D> points;
    Box range(10.0, 10.0, 1.0, 1.0);
    qt.Search(range, points);
    for (const auto &point : points)
    {
        std::cout << point << std::endl;
    }
    qt.Remove(10.0, 10.0);
    points.clear();
    qt.Search(range, points);
    for (const auto &point : points)
    {
        std::cout << point << std::endl;
    }
    qt.Clear();

    return 0;
}