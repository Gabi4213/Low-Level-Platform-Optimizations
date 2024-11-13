#include "Octree.h"

// It took me a really long time to get this octree class working. I ended up in the end creating multiple different versions of it.
// The reasons I kept retrying is because at first I couldn't get it to work properly. Then once I got it working, I had issues of the 
//class being overcomplicated and therefore causing lag when running the project. At some point I got it working with smaller numbers of
//objects but now with larger numbers. So I ended up re-doing it and trying to keep it very simple. This version not only is simple, but
//no longer includes functions like, remove, and doesnt seperate itself into 2 classes of OctreeNode and OctreeRoot. 

// I found this great article about octrees that helped me make this class: 
//https://www.gamedev.net/tutorials/programming/general-and-gameplay-programming/introduction-to-octrees-r3529/

// Here are other useful articles/websites and documents I found relating to octrees. Some are not c++ related:
//https://www.piko3d.net/tutorials/space-partitioning-tutorial-piko3ds-dynamic-octree/
//https://github.com/annell/octree-cpp
//https://gameprogrammingpatterns.com/spatial-partition.html

Octree::Octree(Vec3& octreeCenter, Vec3& octreeHalfSize, int depth)
{
    center = octreeCenter;
    halfSize = octreeHalfSize;

    octreeMaxObjects = 4;
    octreeDepth = 8;

    for (int i = 0; i < CHILDREN_COUNT; i++)
    {
        children[i] = nullptr;
    }
}

Octree::~Octree()
{
    for (int i = 0; i < CHILDREN_COUNT; i++)
    {
        if (children[i])
        {
            delete children[i];
            children[i] = nullptr;
        }
    }
}

void Octree::Insert(ColliderObject* collider)
{
    if (Contains(collider->position) == false)
    {
        return;
    }

    if (octreeDepth <= 0 || colliders.size() < octreeMaxObjects)
    {
        colliders.push_back(collider);
    }
    else
    {
        if (children[0] == nullptr)
        {
            Subdivide();
        }
        for (int i = 0; i < CHILDREN_COUNT; i++)
        {
            children[i]->Insert(collider);
        }
    }
}

void Octree::Retrieve(ColliderObject* collider, std::list<ColliderObject*>& possibleColliders)
{
    if (Contains(collider->position) == false) 
    {
        return;
    }

    for (auto* obj : colliders)
    {
        possibleColliders.push_back(obj);
    }

    if (children[0])
    {
        for (int i = 0; i < CHILDREN_COUNT; i++)
        {
            children[i]->Retrieve(collider, possibleColliders);
        }
    }
}

bool Octree::Contains(Vec3& point)
{
    bool isInsideX = false;
    bool isInsideY = false;
    bool isInsideZ = false;

    if (point.x >= center.x - halfSize.x && point.x <= center.x + halfSize.x)
    {
        isInsideX = true;
    }
    if (point.y >= center.y - halfSize.y && point.y <= center.y + halfSize.y)
    {
        isInsideY = true;
    }
    if (point.z >= center.z - halfSize.z && point.z <= center.z + halfSize.z)
    {
        isInsideZ = true;
    }

    return isInsideX && isInsideY && isInsideZ;
}

void Octree::Subdivide()
{
    Vec3 halfChildSize = halfSize * 0.5f;

    for (int childIndex = 0; childIndex < CHILDREN_COUNT; childIndex++)
    {
        Vec3 childCenter = center;

        if (childIndex & 1)
        {
            childCenter.x += halfChildSize.x;
        }
        else
        {
            childCenter.x -= halfChildSize.x;
        }

        if (childIndex & 2)
        {
            childCenter.y += halfChildSize.y;
        }
        else {
            childCenter.y -= halfChildSize.y;
        }

        if (childIndex & 4)
        {
            childCenter.z += halfChildSize.z;
        }
        else
        {
            childCenter.z -= halfChildSize.z;
        }

        children[childIndex] = new Octree(childCenter, halfChildSize, octreeDepth - 1);
    }
}

void Octree::SetOctreeVariables(int depth, int maxObjects)
{
    octreeDepth = depth;
    octreeMaxObjects = maxObjects;
}