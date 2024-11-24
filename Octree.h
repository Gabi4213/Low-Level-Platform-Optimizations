#pragma once
#include <vector>
#include <list>

#include "ColliderObject.h"
#include "Vec3.h"

#define CHILDREN_COUNT 8

class Octree
{
public:
    Vec3 center;
    Vec3 halfSize;

    Octree* children[CHILDREN_COUNT];
    std::list<ColliderObject*> colliders;

    Octree(Vec3& octreeCenter, Vec3& octreeHalfSize, int depth = 0);
    ~Octree();

    void insert(ColliderObject* collider);
    void retrieve(ColliderObject* collider, std::list<ColliderObject*>& possibleColliders);
    void setOctreeVariables(int depth, int maxObjects);

private:
    int octreeDepth;
    int octreeMaxObjects;

    bool contains(Vec3& point);
    void subdivide();
};