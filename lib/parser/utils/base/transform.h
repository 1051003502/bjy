
#ifndef __PARSER_UTILS_TRANSFORM_H__
#define __PARSER_UTILS_TRANSFORM_H__
#include "point.h"

class Transform
{
public:
    Transform() : x(0), y(0), xscale(1), yscale(1), rotation(0) {}
    Transform(const Transform &t) : x(t.x), y(t.y), xscale(t.xscale), yscale(t.yscale), rotation(t.rotation) {}
    ~Transform() {}
    bool setTransform(const Point& point);

    Point referencePoint(const Point& point);

    Transform &operator=(const Transform &_o)
    {
        if (this != &_o)
        {
            x = _o.x;
            y = _o.y;
            xscale = _o.xscale;
            yscale = _o.yscale;
            rotation = _o.rotation;
        }
        return *this;
    }

    Point trans(const Point &point) const;

    Point transR(const Point &point) const;

    double trans(double r) const;

public:
    double x;
    double y;
    double xscale;
    double yscale;
    double rotation;
};

class Transforms
{
public:
    Transforms() {}
    Transforms(const Transforms &_o)
    {
        transes = _o.transes;
    }
    ~Transforms() {}

    Point trans(const Point &_point) const
    {
        auto p = _point;
        for (auto t : transes)
        {
            p = t.trans(p);
        }
        return p;
    }

    Point transR(const Point &_point) const
    {
        auto p = _point;
        for (auto t : transes)
        {
            p = t.transR(p);
        }
        return p;
    }

    double trans(double _r) const
    {
        auto p = _r;
        for (auto t : transes)
        {
            p = t.trans(p);
        }
        return p;
    }

    Transforms &operator=(const Transforms &_o)
    {
        if (this != &_o)
        {
            transes = _o.transes;
        }
        return *this;
    }

    void appendTransform(const Transform &_tran)
    {
        transes.insert(transes.begin(), _tran);
    };

    std::vector<Transform> transes;
};

#endif