#include "Octree.h"

Octree::Octree(const Vec3& center, const Vec3& halfSize, int depth) : center(center), halfSize(halfSize)
{
    for (int i = 0; i < CHILDREN_COUNT; ++i)
    {
        children[i] = nullptr;
    }
}

Octree::~Octree()
{
    for (int i = 0; i < CHILDREN_COUNT; ++i)
    {
        if (children[i])
        {
            delete children[i];
        }
    }
}

void Octree::Insert(ColliderObject* collider)
{
    if (!IsInside(collider->position)) return;

    std::lock_guard<std::mutex> lock(mutex);

    if (colliders.size() < MAX_OBJECTS || DEPTH <= 0)
    {
        colliders.push_back(collider);
    }
    else
    {
        if (!children[0])
        {
            Subdivide();
        }

        std::vector<std::future<void>> futures;
        for (int i = 0; i < CHILDREN_COUNT; ++i)
        {
            if (futures.size() < MAX_THREADS) 
            {
                futures.emplace_back(std::async(std::launch::async, [&] {children[i]->Insert(collider);}));
            }
            else 
            {
                for (auto& future : futures)
                {
                    future.get();
                }
                futures.clear();
                futures.emplace_back(std::async(std::launch::async, [&] {children[i]->Insert(collider); }));
            }
        }

        for (auto& future : futures) 
        {
            future.get();
        }
    }
}

void Octree::Query(const ColliderObject* collider, std::list<ColliderObject*>& possibleColliders)
{
    if (!IsInside(collider->position)) return;

    {
        std::lock_guard<std::mutex> lock(mutex); // Lock the mutex
        for (auto* obj : colliders)
        {
            possibleColliders.push_back(obj);
        }
    }

    std::vector<std::future<void>> futures;
    if (children[0])
    {
        for (int i = 0; i < CHILDREN_COUNT; ++i)
        {

            if (futures.size() < MAX_THREADS) 
            {
                futures.emplace_back(std::async(std::launch::async, [&] {children[i]->Query(collider, possibleColliders);}));
            }
            else 
            {
                for (auto& future : futures)
                {
                    future.get();
                }
                futures.clear();
                futures.emplace_back(std::async(std::launch::async, [&] {children[i]->Query(collider, possibleColliders);}));
            }
        }

        for (auto& future : futures) 
        {
            future.get();
        }
    }
}

bool Octree::IsInside(const Vec3& point) const
{
    return (point.x >= center.x - halfSize.x && point.x <= center.x + halfSize.x &&
        point.y >= center.y - halfSize.y && point.y <= center.y + halfSize.y &&
        point.z >= center.z - halfSize.z && point.z <= center.z + halfSize.z);
}

void Octree::Subdivide()
{
    Vec3 halfChildSize = halfSize * 0.5f;

    for (int childIndex = 0; childIndex < CHILDREN_COUNT; ++childIndex)
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

        children[childIndex] = new Octree(childCenter, halfChildSize, DEPTH - 1);
    }
}