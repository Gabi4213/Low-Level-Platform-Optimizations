#include "Octree.h"

// It took me a really long time to get this octree class working. I ended up in the end creating multiple different versions of it.
// The reasons I kept retrying is because at first I couldn't get it to work properly. Then once I got it working, I had issues of the 
//class being overcomplicated and therefore causing lag when running the project. At some point I got it working with smaller numbers of
//objects but not with larger numbers. So I ended up re-doing it and trying to keep it very simple. This version not only is simple, but
//no longer includes functions like, remove, and doesnt seperate itself into 2 classes of OctreeNode and OctreeRoot. 

// I found this great article about octrees that helped me make this class: 
//https://www.gamedev.net/tutorials/programming/general-and-gameplay-programming/introduction-to-octrees-r3529/

// Here are other useful articles/websites and documents I found relating to octrees. Some are not c++ related:
//https://www.piko3d.net/tutorials/space-partitioning-tutorial-piko3ds-dynamic-octree/
//https://github.com/annell/octree-cpp
//https://gameprogrammingpatterns.com/spatial-partition.html

Octree::Octree(Vec3& octreeCenter, Vec3& octreeHalfSize, int depth)
{
    //initialize variables
    center = octreeCenter;
    halfSize = octreeHalfSize;

    octreeMaxObjects = 4;
    octreeDepth = 8;

    //set children to null
    for (int i = 0; i < CHILDREN_COUNT; i++)
    {
        children[i] = nullptr;
    }
}

Octree::~Octree()
{
    //clean up children 
    for (int i = 0; i < CHILDREN_COUNT; i++)
    {
        if (children[i])
        {
            delete children[i];
            children[i] = nullptr;
        }
    }
}

void Octree::insert(ColliderObject* collider)
{
    //if it doesnt contain collider position, then return out the function
    if (contains(collider->position) == false)
    {
        return;
    }

    //check that depth is not 0 or that the size isn't exceeded. add collider to current node
    if (octreeDepth <= 0 || colliders.size() < octreeMaxObjects)
    {
        colliders.push_back(collider);
    }
    else
    {
        //otherwise subdivide the node (into 8), redistribute colliders
        if (children[0] == nullptr)
        {
            subdivide();
        }
        for (int i = 0; i < CHILDREN_COUNT; i++)
        {
            children[i]->insert(collider);
        }
    }
}

void Octree::retrieve(ColliderObject* collider, std::list<ColliderObject*>& possibleColliders)
{
    //again if it doesnt contain collider position, then return out the function
    if (contains(collider->position) == false) 
    {
        return;
    }

    //push back all colliders to list
    for (auto* col : colliders)
    {
        possibleColliders.push_back(col);
    }

    //traverse the nodes and see if they contain the collider position
    //if so then push them back to possibleColliders
    if (children[0])
    {
        for (int i = 0; i < CHILDREN_COUNT; i++)
        {
            children[i]->retrieve(collider, possibleColliders);
        }
    }
}

bool Octree::contains(Vec3& point)
{
    //set starting bools to false
    bool x = false;
    bool y = false;
    bool z = false;

    //check current octree node bound - x axis
    if (point.x >= center.x - halfSize.x && point.x <= center.x + halfSize.x)
    {
        x = true;
    }
    //check current octree node bound - y axis
    if (point.y >= center.y - halfSize.y && point.y <= center.y + halfSize.y)
    {
        y = true;
    }
    //check current octree node bound - z axis
    if (point.z >= center.z - halfSize.z && point.z <= center.z + halfSize.z)
    {
        z = true;
    }

    //return the result of that
    return x && y && z;
}

void Octree::subdivide()
{
    //store a reference to the half size of the child size. 
    Vec3 halfChildSize = halfSize * 0.5f;

    // loop through all children ( i is basically the child index)
    for (int i = 0; i < CHILDREN_COUNT; i++)
    {
        //current child center is set here to the parents center
        Vec3 childCenter = center;

        //here i am using bitwise operations. the lets me known which like area the child is in
        // so like its done for each axis x,y and z
        //https://www.programiz.com/cpp-programming/bitwise-operators
        //https://beginnersbook.com/2022/09/bitwise-operators-in-c-with-examples/
        if (i & 1)
        {
            //move possitively - x
            childCenter.x += halfChildSize.x;
        }
        else
        {
            //move negatively - x
            childCenter.x -= halfChildSize.x;
        }

        if (i & 2)
        {
            //move possitively - y
            childCenter.y += halfChildSize.y;
        }
        else 
        {
            //move negatively - y
            childCenter.y -= halfChildSize.y;
        }

        if (i & 4)
        {
            //move possitively - z
            childCenter.z += halfChildSize.z;
        }
        else
        {
            //move negatively - z
            childCenter.z -= halfChildSize.z;
        }

        //create new octree per child
        children[i] = new Octree(childCenter, halfChildSize, octreeDepth - 1);
    }
}

void Octree::setOctreeVariables(int depth, int maxObjects)
{
    //returns depth and max objects
    octreeDepth = depth;
    octreeMaxObjects = maxObjects;
}