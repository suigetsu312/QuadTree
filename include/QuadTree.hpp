#ifndef QUADTREE_HPP
#define QUADTREE_HPP
#include <vector>
#include <memory>
#include <iostream>
#include <cassert>
#include <algorithm>
enum class Quadrant
{
    NE,
    NW,
    SE,
    SW
};

struct Point2D
{
    double x, y;
    Point2D(double x, double y) : x(x), y(y) {}

    bool operator==(const Point2D &other) const
    {
        const double EPSILON = 1e-9;
        return std::abs(x - other.x) < EPSILON && std::abs(y - other.y) < EPSILON;
    }

    bool operator!=(const Point2D &other) const
    {
        return !(*this == other);
    }

    bool operator<(const Point2D &other) const
    {
        if (x == other.x)
        {
            return y < other.y;
        }
        return x < other.x;
    }

    friend std::ostream &operator<<(std::ostream &os, const Point2D &p)
    {
        os << "Point(" << p.x << ", " << p.y << ")";
        return os;
    }
};

struct Box
{
    /* data */
    Point2D center;
    double width, height;

    Box(double x, double y, double width, double height)
        : center(x, y), width(width), height(height) {}

    // 取得所在象限
    Quadrant GetQuadrant(const Box &otherBox)
    {
        bool right = otherBox.center.x >= center.x;
        bool top = otherBox.center.y >= center.y;

        if (right && top)
            return Quadrant::NE;
        if (!right && top)
            return Quadrant::NW;
        if (right && !top)
            return Quadrant::SE;
        return Quadrant::SW;
    }

    Quadrant GetQuadrant(const Point2D &point)
    {
        if (point.x >= center.x && point.y >= center.y)
        {
            return Quadrant::NE; // 右上
        }
        else if (point.x < center.x && point.y >= center.y)
        {
            return Quadrant::NW; // 左上
        }
        else if (point.x >= center.x && point.y < center.y)
        {
            return Quadrant::SE; // 右下
        }
        else
        {
            return Quadrant::SW; // 左下
        }
    }

    inline bool Contains(const Point2D &point) const
    {
        return (point.x >= center.x - width / 2 && point.x <= center.x + width / 2 &&
                point.y >= center.y - height / 2 && point.y <= center.y + height / 2);
    }

    inline Point2D GetTopLeft() const
    {
        return Point2D(center.x - width / 2, center.y + height / 2);
    }
    inline Point2D GetBottomRight() const
    {
        return Point2D(center.x + width / 2, center.y - height / 2);
    }
    inline bool Intersection(const Box &box) const
    {
        auto tl = GetTopLeft();
        auto br = GetBottomRight();
        return !(box.GetTopLeft().x > br.x || box.GetBottomRight().x < tl.x ||
                 box.GetTopLeft().y < br.y || box.GetBottomRight().y > tl.y);
    }
};

struct QuadNode
{
    Box boundary;
    QuadNode *children[4] = {nullptr}; // NE, NW, SE, SW
    std::vector<Point2D> points;
    size_t depth = 0;

    QuadNode(Box &&box, int depth = 0)
        : boundary(std::move(box)), depth(depth) {}

    ~QuadNode()
    {
        // 手動釋放子節點
        for (int i = 0; i < 4; ++i)
        {
            delete children[i];
        }
    }

    bool IsLeaf() const
    {
        return children[0] == nullptr;
    }

    void split()
    {
        assert(IsLeaf() && "Only leaf nodes can be split");

        double halfWidth = boundary.width / 2.0;
        double halfHeight = boundary.height / 2.0;

        double cx = boundary.center.x;
        double cy = boundary.center.y;

        children[0] = new QuadNode(Box(cx + halfWidth / 2, cy + halfHeight / 2, halfWidth, halfHeight), depth + 1); // NE
        children[1] = new QuadNode(Box(cx - halfWidth / 2, cy + halfHeight / 2, halfWidth, halfHeight), depth + 1); // NW
        children[2] = new QuadNode(Box(cx + halfWidth / 2, cy - halfHeight / 2, halfWidth, halfHeight), depth + 1); // SE
        children[3] = new QuadNode(Box(cx - halfWidth / 2, cy - halfHeight / 2, halfWidth, halfHeight), depth + 1); // SW
    }

    bool TryMerge(int maxChildren)
    {
        if (IsLeaf())
            return false;

        int totalPoints = 0;
        for (int i = 0; i < 4; ++i)
        {
            if (!children[i]->IsLeaf())
                return false;
            totalPoints += children[i]->points.size();
        }

        if (totalPoints > maxChildren)
            return false;

        // 合併所有子節點的點
        for (int i = 0; i < 4; ++i)
        {
            points.insert(points.end(), children[i]->points.begin(), children[i]->points.end());
            delete children[i];
            children[i] = nullptr;
        }

        return true;
    }
};

class QuadTree
{
public:
    QuadTree(double x, double y, double width, double height, int maxDepth, int maxChildren)
        : maxDepth(maxDepth), maxChildren(maxChildren)
    {
        root = new QuadNode(Box(x, y, width, height));
    }
    QuadTree(double width, double height, int maxDepth, int maxChildren)
        : maxDepth(maxDepth), maxChildren(maxChildren)
    {
        root = new QuadNode(Box(0.0, 0.0, width, height));
    }
    ~QuadTree()
    {
        delete root;
    }
    void Insert(double x, double y)
    {
        Insert(root, Point2D(x, y));
    }
    void Insert(const Point2D &point)
    {
        Insert(root, point);
    }
    void Insert(const std::vector<Point2D> &points)
    {
        for (const auto &point : points)
        {
            Insert(point);
        }
    }
    void Search(Box &range, std::vector<Point2D> &values)
    {
        Search(root, range, values);
    }
    void Remove(const double &x, const double &y)
    {
        auto p = Point2D(x,y);
        Remove(root, p);
    }

    void Remove(const Point2D &point)
    {
        Remove(root, point);
    }
    void Remove(const std::vector<Point2D> &points)
    {
        for (const auto &point : points)
        {
            Remove(point);
        }
    }
    void Remove(Box &range)
    {
        std::vector<Point2D> points;
        Search(range, points);
        for (const auto &point : points)
        {
            Remove(point);
        }
    }
    void Clear()
    {
        double width = root->boundary.width;
        double height = root->boundary.height;
        delete root;
        root = new QuadNode(Box(0.0, 0.0, width, height));
    }

private:
    QuadNode *root = nullptr;
    size_t maxDepth = 8;
    size_t maxChildren = 16;
    void Subdivide(QuadNode &node, int depth);
    void Insert(QuadNode &node, double x, double y, int depth);

    void Insert(QuadNode *node, const Point2D &point)
    {
        if (!node->boundary.Contains(point))
        {
            return;
        }
        if (node->points.size() < maxChildren && node->depth <= maxDepth)
        {
            if (node->children[0] == nullptr)
            {
                node->split();
            }
            Quadrant quadrant = node->boundary.GetQuadrant(point);
            Insert(node->children[static_cast<int>(quadrant)], point); // SW
        }
        else
        {
            // 如果節點已經是葉節點，或達到最大深度，則將點插入到此節點
            node->points.push_back(point);
        }
    }

    void Search(QuadNode *node, Box &range, std::vector<Point2D> &values)
    {
        // 如果當前節點的範圍與查詢範圍不相交，則返回
        if (!node->boundary.Intersection(range))
        {
            return;
        }

        // 如果當前節點是葉節點，則檢查所有點
        if (node->IsLeaf())
        {
            for (const auto &point : node->points)
            {
                if (range.Contains(point))
                {
                    values.push_back(point);
                }
            }
        }
        else
        {
            // 遞迴查詢子節點
            for (int i = 0; i < 4; ++i)
            {
                Search(node->children[i], range, values);
            }
        }
    }

    bool Remove(QuadNode *node, const Point2D &point)
    {
        if (!node->boundary.Contains(point))
            return false;

        // 若是葉節點
        if (node->IsLeaf())
        {
            auto it = std::find(node->points.begin(), node->points.end(), point);
            if (it != node->points.end())
            {
                node->points.erase(it);
                return true;
            }
            return false;
        }

        // 若不是葉節點，遞迴到對的象限去刪
        Quadrant quadrant = node->boundary.GetQuadrant(point);
        bool removed = Remove(node->children[static_cast<int>(quadrant)], point);

        // 嘗試合併子節點
        if (removed)
        {
            node->TryMerge(maxChildren);
        }

        return removed;
    }
};

#endif