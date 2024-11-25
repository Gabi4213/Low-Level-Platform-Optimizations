#include "imgui/imgui.h"
#include "imgui/imgui_impl_freeglut.h"
#include "imgui/imgui_impl_opengl2.h"

#include <stdlib.h>
#include <GL/glut.h>
#include <list>
#include <iostream>

#include <cstdlib>
#include <ctime>
#include <chrono>
#include <string>
#include <windows.h>
#include <psapi.h>
#include <vector>
#include <deque>

#include <thread>
#include <mutex>

#include "globals.h"
#include "Vec3.h"
#include "ColliderObject.h"
#include "DiagnosticsTracker.h"
#include "MemoryPool.h"
#include "Octree.h"

using namespace std::chrono;

//bozxes and spheres
int numberOfBoxes = 50;
int numberOfSpheres = 50;
std::list<ColliderObject*> colliders;

//octree
int octreeDepth = 5;
int octreeMaxObjects = 4;
Octree* octree;

//memory
int memoryAllocatedSize = 10;
size_t boxTotalBytesAllocated = 1024; //defualt of 1024. works for most but once collider count is high this needs to be increased
DiagnosticsTracker* diagnosticsTracker;

//threads
int maxThreads = 4;
std::mutex globalMutex;
std::mutex collisionMutex;
std::list<std::thread> threads;

//camera
#define LOOKAT_X 10
#define LOOKAT_Y 10
#define LOOKAT_Z 50

#define LOOKDIR_X 10
#define LOOKDIR_Y 0
#define LOOKDIR_Z 0

//used to destroy all colliders in the scene
void destroyColliders()
{
    //something interesting I need to actually use a cast here otherwise for some reason sometimes VS would get confused
    //And triued to delete a box using the sphere delete operator
    //not sure why that was the case as sometimes i did work. So for saftey i cast it
    for (ColliderObject* obj : colliders)
    {
        if (Box* box = dynamic_cast<Box*>(obj))
        {
            Box::operator delete(obj, sizeof(Box));
        }
        else if (Sphere* sphere = dynamic_cast<Sphere*>(obj))
        {
            Sphere::operator delete(obj, sizeof(Sphere));
        }
    }

    colliders.clear();
}

//allocate how bytes for spheres and boxes individually
//i use this because like mentioned above 1024 works most the time
//but when there is thousands of objects it needs to be increased
void allocateColliderBytes(size_t boxBytes, size_t sphereBytes)
{
    destroyColliders();

    Box::initalizeMemoryPool(boxBytes);
    Sphere::initalizeMemoryPool(sphereBytes);
}

//initlaize the scene
void initScene(int boxCount, int sphereCount)
{
    diagnosticsTracker = new DiagnosticsTracker();

    for (int i = 0; i < boxCount; ++i)
    {            
        Box* box = new Box;

        // Assign random x, y, and z positions within specified ranges
        box->position.x = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 20.0f));
        box->position.y = 10.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 1.0f));
        box->position.z = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 20.0f));

        box->size = { 1.0f, 1.0f, 1.0f };

        // Assign random x-velocity between -1.0f and 1.0f
        float randomXVelocity = -1.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 2.0f));
        box->velocity = { randomXVelocity, 0.0f, 0.0f };

        // Assign a random color to the box
        box->colour.x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        box->colour.y = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        box->colour.z = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

        colliders.push_back(box);
    }

    for (int i = 0; i < sphereCount; ++i) 
    {
        Sphere* sphere = new Sphere;

        // Assign random x, y, and z positions within specified ranges
        sphere->position.x = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 20.0f));
        sphere->position.y = 10.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 1.0f));
        sphere->position.z = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 20.0f));

        sphere->size = { 1.0f, 1.0f, 1.0f };

        // Assign random x-velocity between -1.0f and 1.0f
        float randomXVelocity = -1.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 2.0f));
        sphere->velocity = { randomXVelocity, 0.0f, 0.0f };

        // Assign a random color to the box
        sphere->colour.x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        sphere->colour.y = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        sphere->colour.z = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

        colliders.push_back(sphere);
    }
}

// a ray which is used to tap (by default, remove) a box - see the 'mouse' function for how this is used
bool rayBoxIntersection(const Vec3& rayOrigin, const Vec3& rayDirection, const ColliderObject* box) 
{
    float tMin = (box->position.x - box->size.x / 2.0f - rayOrigin.x) / rayDirection.x;
    float tMax = (box->position.x + box->size.x / 2.0f - rayOrigin.x) / rayDirection.x;

    if (tMin > tMax) std::swap(tMin, tMax);

    float tyMin = (box->position.y - box->size.y / 2.0f - rayOrigin.y) / rayDirection.y;
    float tyMax = (box->position.y + box->size.y / 2.0f - rayOrigin.y) / rayDirection.y;

    if (tyMin > tyMax) std::swap(tyMin, tyMax);

    if ((tMin > tyMax) || (tyMin > tMax))
        return false;

    if (tyMin > tMin)
        tMin = tyMin;

    if (tyMax < tMax)
        tMax = tyMax;

    float tzMin = (box->position.z - box->size.z / 2.0f - rayOrigin.z) / rayDirection.z;
    float tzMax = (box->position.z + box->size.z / 2.0f - rayOrigin.z) / rayDirection.z;

    if (tzMin > tzMax) std::swap(tzMin, tzMax);

    if ((tMin > tzMax) || (tzMin > tMax))
        return false;

    return true;
}

// used in the 'mouse' tap function to convert a screen point to a point in the world
Vec3 screenToWorld(int x, int y) 
{
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
    GLdouble posX, posY, posZ;

    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);

    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
    glReadPixels(x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

    gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

    return Vec3((float)posX, (float)posY, (float)posZ);
}

//this function checks for collisions and resolves them
// it uses the octree and is in a differnt function now so that threads can call it
void updateColliders(std::list<ColliderObject*> colliderList)
{
    for (ColliderObject* collider : colliderList)
    {
        std::list<ColliderObject*> possibleColliders;
        octree->retrieve(collider, possibleColliders);

        for (ColliderObject* other : possibleColliders)
        {
            if (collider != other)
            {
                if (collider->checkCollision(collider, other))
                {
                    std::lock_guard<std::mutex> lock(collisionMutex);
                    collider->resolveCollision(collider, other);
                }
            }
        }
    }
}

// this function adds colliders to octree, updates their velocity etc, and uses threads#
// it does NOT do any collision checking though
void updatePhysics(const float deltaTime)
{
    //start timer
    std::string functionName = "updatePhysics";
    diagnosticsTracker->startTimer(functionName);

    //create octree
    Vec3 octreeCenter = Vec3(0.0f, 0.0f, 0.0f);
    Vec3 octreeHalfSize = Vec3(100.0f, 100.0f, 100.0f);

    octree = new Octree(octreeCenter, octreeHalfSize);
    octree->setOctreeVariables(octreeDepth, octreeMaxObjects);

    //insert colliders to octre
    for (ColliderObject* collider : colliders)
    {
        octree->insert(collider);
    }

    //calculate how many colliders each thread deals with
    int colldidersPerThreads = colliders.size() / maxThreads;

    //itterate through colliders and add them to new lists called chunks
    auto itterator = colliders.begin();

    for (int i = 0; i < maxThreads; i++)
    {
        std::list<ColliderObject*> chunks;

        for (int j = 0; j < colldidersPerThreads; j++)
        {
            if (itterator != colliders.end())
            {
                chunks.push_back(*itterator);
                itterator++;
            }
        }
        //since my threads are in a list push them back and have them call the update colliders function with the chunks list
        threads.push_back(std::thread(updateColliders, chunks));
    }

    //updates colliders - this is just gravity, velocity and wall collisions
    //no longer has actual collision checks though
    for (ColliderObject* collider : colliders)
    {
        collider->update(deltaTime);
    }

    //join threads
    for (auto& thread : threads)
    {
        thread.join();
    }

    //clean up
    threads.clear();
    delete octree;
    octree = nullptr;

    //stop timer
    diagnosticsTracker->stopTimer(functionName);
}

// draw the sides of the containing area
void drawQuad(const Vec3& v1, const Vec3& v2, const Vec3& v3, const Vec3& v4) 
{

    glBegin(GL_QUADS);
    glVertex3f(v1.x, v1.y, v1.z);
    glVertex3f(v2.x, v2.y, v2.z);
    glVertex3f(v3.x, v3.y, v3.z);
    glVertex3f(v4.x, v4.y, v4.z);
    glEnd();
}

// draw the entire scene
void drawScene() 
{
    //start timer
    std::string functionName = "drawScene";
    diagnosticsTracker->startTimer(functionName);

    // Draw the side wall
    GLfloat diffuseMaterial[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseMaterial);

    // Draw the left side wall
    glColor3f(0.5f, 0.5f, 0.5f); // Set the wall color
    Vec3 leftSideWallV1(minX, 0.0f, maxZ);
    Vec3 leftSideWallV2(minX, 50.0f, maxZ);
    Vec3 leftSideWallV3(minX, 50.0f, minZ);
    Vec3 leftSideWallV4(minX, 0.0f, minZ);
    drawQuad(leftSideWallV1, leftSideWallV2, leftSideWallV3, leftSideWallV4);

    // Draw the right side wall
    glColor3f(0.5f, 0.5f, 0.5f); // Set the wall color
    Vec3 rightSideWallV1(maxX, 0.0f, maxZ);
    Vec3 rightSideWallV2(maxX, 50.0f, maxZ);
    Vec3 rightSideWallV3(maxX, 50.0f, minZ);
    Vec3 rightSideWallV4(maxX, 0.0f, minZ);
    drawQuad(rightSideWallV1, rightSideWallV2, rightSideWallV3, rightSideWallV4);


    // Draw the back wall
    glColor3f(0.5f, 0.5f, 0.5f); // Set the wall color
    Vec3 backWallV1(minX, 0.0f, minZ);
    Vec3 backWallV2(minX, 50.0f, minZ);
    Vec3 backWallV3(maxX, 50.0f, minZ);
    Vec3 backWallV4(maxX, 0.0f, minZ);
    drawQuad(backWallV1, backWallV2, backWallV3, backWallV4);

    for (ColliderObject* box : colliders)
    {
        box->draw();
    }

    //stop timer
    diagnosticsTracker->stopTimer(functionName);
}

//draw imgui UI
//i addded #ifdef _DEBUG so its in debug only now
void drawImGui()
{
#ifdef _DEBUG
    ImGui::Begin("Statistics & Controls");

    if (ImGui::CollapsingHeader("Diagnostic Data"))
    {
        //Memory
        ImGui::Text("RAM Memory Usage: %s", diagnosticsTracker->getMemoryUsage().c_str());

        //CPU
        std::string cpuUsage = diagnosticsTracker->getCPUUsage() + "%";
        ImGui::Text("CPU Usage: %s", cpuUsage.c_str());

        //Frame Time
        std::string frameTime = "Frame Time: " + diagnosticsTracker->getFrameTime();
        ImGui::Text(frameTime.c_str());

        //FPS
        std::string fps = "FPS: " + diagnosticsTracker->getFPS() + "ms";
        ImGui::Text(fps.c_str());

        //Thread Count
        std::string threadCount = "Thread Count: " + std::to_string(maxThreads);;
        ImGui::Text(threadCount.c_str());

        //Total Memory Allocation
        std::string totalMemoryAllocation = "Total Memory Allocation: " + diagnosticsTracker->getTotalMemoryAllocated();
        ImGui::Text(totalMemoryAllocation.c_str());

        //function run times

        //physics function
        std::string updatePhysicsfunctionName = "updatePhysics";
        std::string updatePhysicsRunTime = "updatePhysics() Execution Time: " + diagnosticsTracker->getFunctionRunTime(updatePhysicsfunctionName);
        ImGui::Text(updatePhysicsRunTime.c_str());

        //draw function
        std::string drawScenefunctionName = "drawScene";
        std::string drawSceneRunTime = "drawScene() Execution Time: " + diagnosticsTracker->getFunctionRunTime(drawScenefunctionName);
        ImGui::Text(drawSceneRunTime.c_str());

        if (ImGui::Button("Print Function Execution Times"))
        {
            std::cout << "drawScene() Execution Time: " + diagnosticsTracker->getFunctionRunTime(drawScenefunctionName) << std::endl;
            std::cout << "updatePhysics() Execution Time: " + diagnosticsTracker->getFunctionRunTime(updatePhysicsfunctionName) << std::endl;
        }
    }
    if (ImGui::CollapsingHeader("Memory Management"))
    {
        if (ImGui::Button("Walk The Heap"))
        {
            diagnosticsTracker->walkTheHeap();
        }

        if (ImGui::Button("Output Memory Allocation"))
        {
            diagnosticsTracker->outputMemoryAllocation();
        }

        if (ImGui::Button("Output Box Memory Data"))
        {
            diagnosticsTracker->outputBoxMemoryAllocation();
        }

        if (ImGui::Button("Output Sphere Memory Data"))
        {
            diagnosticsTracker->outputSphereMemoryAllocation();
        }

        if (ImGui::Button("Trigger Buffer Overflow"))
        {
            diagnosticsTracker->triggerBufferOverflow();
        }
        if (ImGui::Button("Trigger Corruption"))
        {
            diagnosticsTracker->triggerMemoryCorruption();
        }

        ImGui::Text("Demonstration of ‘global new and delete’");
        ImGui::SliderInt("Memory Amount", &memoryAllocatedSize, 0, 50);

        if (ImGui::Button("Allocate Memory"))
        {
            diagnosticsTracker->allocateMemory(memoryAllocatedSize);
        }
        if (ImGui::Button("Deallocate Memory"))
        {
            diagnosticsTracker->deallocateMemory();
        }
    }
    if (ImGui::CollapsingHeader("Objects"))
    {
        ImGui::SliderInt("Number of Cubes", &numberOfBoxes, 100, 1000);
        ImGui::SliderInt("Number of Spheres", &numberOfSpheres, 100, 1000);

        //here I use allocateColliderBytes so that i can change the memory allocated to the colliders
        // like mentioned above when there is a lot of boxes this needs to increase
        if (ImGui::Button("Initialize Scene"))
        {
            allocateColliderBytes(1024, 1024);

            initScene(numberOfBoxes, numberOfSpheres);
        }

        if (ImGui::Button("Set 100"))
        {
            numberOfBoxes = 50;
            numberOfSpheres = 50;

            allocateColliderBytes(1024, 1024);

            initScene(numberOfBoxes, numberOfSpheres);
        }
        ImGui::SameLine();

        if (ImGui::Button("Set 1000"))
        {
            numberOfBoxes = 500;
            numberOfSpheres = 500;

            allocateColliderBytes(1024, 1024);

            initScene(numberOfBoxes, numberOfSpheres);
        }
        ImGui::SameLine();

        if (ImGui::Button("Set 10000"))
        {
            numberOfBoxes = 2500;
            numberOfSpheres = 2500;

            allocateColliderBytes(5000, 5000);

            initScene(numberOfBoxes, numberOfSpheres);
        }
        if (ImGui::Button("Add Impulse")) 
        {
            for (ColliderObject* collider : colliders)
            {
                collider->impulse(20.0f);
            }
        }
        if (ImGui::Button("Delete Object 1 by 1"))
        {
            if (colliders.size() > 0)
            {
                if (Box* box = dynamic_cast<Box*>(colliders.front()))
                {
                    Box::operator delete(colliders.front(), sizeof(Box));
                    colliders.remove(colliders.front());
                }
                else if (Sphere* sphere = dynamic_cast<Sphere*>(colliders.front()))
                {
                    Sphere::operator delete(colliders.front(), sizeof(Sphere));
                    colliders.remove(colliders.front());
                }
            }
        }
        if (ImGui::Button("Delete All Objects"))
        {
            destroyColliders();
        }
        if (ImGui::Button("Add Box"))
        {
            Box* box = new Box();

            // Assign random x, y, and z positions within specified ranges
            box->position.x = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 20.0f));
            box->position.y = 10.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 1.0f));
            box->position.z = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 20.0f));

            box->size = { 1.0f, 1.0f, 1.0f };

            // Assign random x-velocity between -1.0f and 1.0f
            float randomXVelocity = -1.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 2.0f));
            box->velocity = { randomXVelocity, 0.0f, 0.0f };

            // Assign a random color to the box
            box->colour.x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            box->colour.y = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            box->colour.z = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

            colliders.push_back(box);
        }

        ImGui::SameLine();

        if (ImGui::Button("Add Sphere"))
        {
            Sphere* sphere = new Sphere;

            // Assign random x, y, and z positions within specified ranges
            sphere->position.x = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 20.0f));
            sphere->position.y = 10.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 1.0f));
            sphere->position.z = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 20.0f));

            sphere->size = { 1.0f, 1.0f, 1.0f };

            // Assign random x-velocity between -1.0f and 1.0f
            float randomXVelocity = -1.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 2.0f));
            sphere->velocity = { randomXVelocity, 0.0f, 0.0f };

            // Assign a random color to the box
            sphere->colour.x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            sphere->colour.y = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            sphere->colour.z = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

            colliders.push_back(sphere);
        }
    }
    if (ImGui::CollapsingHeader("Threads"))
    {
        ImGui::SliderInt("Number of Threads", &maxThreads, 1, std::thread::hardware_concurrency());
    }
    if (ImGui::CollapsingHeader("Octree"))
    {
        std::string depthText = "Octree Depth: " + std::to_string(octreeDepth);
        ImGui::Text(depthText.c_str());

        std::string maxObjectsText = "Octree Max Objects: " + std::to_string(octreeMaxObjects);
        ImGui::Text(maxObjectsText.c_str());

        ImGui::SliderInt("Depth", &octreeDepth, 1, 30);
        ImGui::SliderInt("Max Objects", &octreeMaxObjects, 1, 30);
    }

    ImGui::End();
#endif
}

// called by GLUT - displays the scene
void display()
{
#ifdef _DEBUG
    //imgui new frame
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplFreeGLUT_NewFrame();

    drawImGui();

    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO();

    glViewport(0, 0, (GLsizei)io.DisplaySize.x, (GLsizei)io.DisplaySize.y);
#endif

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

#ifdef _DEBUG
    gluPerspective(40.0, io.DisplaySize.x / io.DisplaySize.y, 1.0, 150.0);
    glLoadIdentity();
#endif

    gluLookAt(LOOKAT_X, LOOKAT_Y, LOOKAT_Z, LOOKDIR_X, LOOKDIR_Y, LOOKDIR_Z, 0, 1, 0);
    drawScene();

#ifdef _DEBUG
    glDisable(GL_LIGHTING);
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
#endif
    glEnable(GL_LIGHTING);
    glutSwapBuffers();
}

// called by GLUT when the cpu is idle - has a timer function you can use for FPS, and updates the physics
// see https://www.opengl.org/resources/libraries/glut/spec3/node63.html#:~:text=glutIdleFunc
// NOTE this may be capped at 60 fps as we are using glutPostRedisplay(). If you want it to go higher than this, maybe a thread will help here. 
void idle()
{
    static auto last = steady_clock::now();
    auto old = last;
    last = steady_clock::now();
    const duration<float> frameTime = last - old;
    float deltaTime = frameTime.count();

    updatePhysics(deltaTime);

    // tell glut to draw - note this will cap this function at 60 fps
    glutPostRedisplay();
}

// called the mouse button is tapped
void mouse(int button, int state, int x, int y) 
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) 
    {
        // Get the camera position and direction
        Vec3 cameraPosition(LOOKAT_X, LOOKAT_Y, LOOKAT_Z); // Replace with your actual camera position
        Vec3 cameraDirection(LOOKDIR_X, LOOKDIR_Y, LOOKDIR_Z); // Replace with your actual camera direction

        // Get the world coordinates of the clicked point
        Vec3 clickedWorldPos = screenToWorld(x, y);

        // Calculate the ray direction from the camera position to the clicked point
        Vec3 rayDirection = clickedWorldPos - cameraPosition;
        rayDirection.normalise();

        // Perform a ray-box intersection test and remove the clicked box
        bool clickedBoxOK = false;
        float minIntersectionDistance = std::numeric_limits<float>::max();

        for (ColliderObject* box : colliders) 
        {
            if (rayBoxIntersection(cameraPosition, rayDirection, box))
            {
                // Calculate the distance between the camera and the intersected box
                Vec3 diff = box->position - cameraPosition;
                float distance = diff.length();

                // Update the clicked box index if this box is closer to the camera
                if (distance < minIntersectionDistance) 
                {
                    clickedBoxOK = true;
                    minIntersectionDistance = distance;
                }
            }
        }

        // Remove the clicked box if any
        if (clickedBoxOK != false) 
        {
            // TODO
            //colliders.erase(colliders.begin() + clickedBoxIndex);
        }
    }
}

// called when the keyboard is used
void keyboard(unsigned char key, int x, int y) 
{
    //output memory data in general, sphere memeory ,box memory
    if (key == 'm') 
    {
        diagnosticsTracker->outputMemoryAllocation();
        diagnosticsTracker->outputBoxMemoryAllocation();
        diagnosticsTracker->outputSphereMemoryAllocation();
    }

    //allocate 10 bytes
    else if (key == 't')
    {
        diagnosticsTracker->allocateMemory(10);
    }
    //deallocate memory
    else if (key == 'u')
    {
        diagnosticsTracker->deallocateMemory();
    }

    //add boxes 1 by 1
    else if (key == 'r')
    {
        Box* box = new Box();

        // Assign random x, y, and z positions within specified ranges
        box->position.x = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 20.0f));
        box->position.y = 10.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 1.0f));
        box->position.z = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 20.0f));

        box->size = { 1.0f, 1.0f, 1.0f };

        // Assign random x-velocity between -1.0f and 1.0f
        float randomXVelocity = -1.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 2.0f));
        box->velocity = { randomXVelocity, 0.0f, 0.0f };

        // Assign a random color to the box
        box->colour.x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        box->colour.y = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        box->colour.z = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

        colliders.push_back(box);
    }
    //remove boxes 1 by 1
    else if (key == 'a')
    {
        auto itterator = colliders.begin();
        while (itterator != colliders.end())
        {
            if (Box* box = dynamic_cast<Box*>(*itterator))
            {
                Box::operator delete(*itterator, sizeof(Box));
                colliders.erase(itterator);
                break; 
            }
            else
            {
                itterator++;
            }
        }
    }

    //add spheres 1 by 1
    else if (key == 'R')
    {
        Sphere* sphere = new Sphere;

        // Assign random x, y, and z positions within specified ranges
        sphere->position.x = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 20.0f));
        sphere->position.y = 10.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 1.0f));
        sphere->position.z = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 20.0f));

        sphere->size = { 1.0f, 1.0f, 1.0f };

        // Assign random x-velocity between -1.0f and 1.0f
        float randomXVelocity = -1.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 2.0f));
        sphere->velocity = { randomXVelocity, 0.0f, 0.0f };

        // Assign a random color to the box
        sphere->colour.x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        sphere->colour.y = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        sphere->colour.z = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

        colliders.push_back(sphere);
    }
    //remove spheres 1 by 1
    else if (key == 'A')
    {
        auto itterator = colliders.begin();
        while (itterator != colliders.end())
        {
            if (Sphere* box = dynamic_cast<Sphere*>(*itterator))
            {
                Sphere::operator delete(*itterator, sizeof(Sphere));
                colliders.erase(itterator);
                break;
            }
            else
            {
                itterator++;
            }
        }
    }

    //walk the heap
    else if (key == 'w')
    {
        diagnosticsTracker->walkTheHeap();
    }

    //enforce memory corruption
    else if (key == 'f')
    {
        diagnosticsTracker->triggerMemoryCorruption();
    }

    //buffer overflow
    else if (key == 'h')
    {
        diagnosticsTracker->triggerBufferOverflow();
    }

    else if (key == ' ') 
    {
        for (ColliderObject* collider : colliders) 
        {
            collider->impulse(20.0f);
        }
    }
}

// the main function. 
int main(int argc, char** argv)
{
    srand(static_cast<unsigned>(time(0))); // Seed random number generator
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1200, 600);
    glutCreateWindow("Physics Simulation - Gabriela Maczynska");
    glutDisplayFunc(display);

    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);

#ifdef _DEBUG
    // Setup ImGui binding
    ImGui::CreateContext();
    ImGui_ImplFreeGLUT_Init();
    ImGui_ImplFreeGLUT_InstallFuncs();
    ImGui_ImplOpenGL2_Init();
#endif

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 800.0 / 600.0, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);

#ifdef _DEBUG
    // Setup style
    ImGui::StyleColorsDark();
#endif

    allocateColliderBytes(1024,1024);

    initScene(numberOfBoxes, numberOfSpheres);

    glutIdleFunc(idle);

    // it will stick here until the program ends. 
    glutMainLoop();

#ifdef _DEBUG
    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplFreeGLUT_Shutdown();
    ImGui::DestroyContext();

#endif

    //Diagnositcs Tracker Cleanup
    delete diagnosticsTracker;
    diagnosticsTracker = nullptr;

    return 0;
}