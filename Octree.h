#pragma once
#include <vector>
#include <list>

#include "ColliderObject.h"
#include "Vec3.h"

#define DEPTH 5
#define MAX_OBJECTS 4
#define CHILDREN_COUNT 8

class Octree
{
public:
    Vec3 center;
    Vec3 halfSize;

    Octree* children[CHILDREN_COUNT];
    std::list<ColliderObject*> colliders;

    Octree(const Vec3& center, const Vec3& halfSize, int depth = 0);
    ~Octree();

    void Insert(ColliderObject* collider);
    void Query(const ColliderObject* collider, std::list<ColliderObject*>& possibleColliders);

private:
    bool IsInside(const Vec3& point) const;
    void Subdivide();
};
