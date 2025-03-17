#include "chai3d.h"
#include <GLFW/glfw3.h>
#include <vector>

using namespace chai3d;
using namespace std;

// Global Variables
cWorld* world;
cCamera* camera;
cDirectionalLight* light;

// GLFW Window
GLFWwindow* window;

//------------------------------------------------------------------------------
// STATES
//------------------------------------------------------------------------------
enum MouseStates
{
    MOUSE_IDLE,
    MOUSE_MOVE_CAMERA
};

// Callback to render scene
void updateGraphics()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera->renderView(800, 600);
    glfwSwapBuffers(window);
}

// Función para cargar un modelo y crear un contenedor de mallas múltiples
cMultiMesh* loadModel(const std::string& filepath)
{
    cMultiMesh* model = new cMultiMesh();
    if (!model->loadFromFile(filepath))
    {
        cout << "Error: Could not load STL file " << filepath << endl;
        return nullptr;
    }
    return model;
}

// Main function
int main(int argc, char* argv[])
{
    // Initialize GLFW
    if (!glfwInit())
    {
        cout << "Failed to initialize GLFW" << endl;
        return -1;
    }

    // Create Window
    window = glfwCreateWindow(800, 600, "Chai3D STL Viewer", NULL, NULL);
    if (!window)
    {
        cout << "Failed to create window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Create a new world
    world = new cWorld();

    // Create a camera and position it FARTHER
    camera = new cCamera(world);
    world->addChild(camera);
    camera->set(cVector3d(0.0, 0.0, 500.0),   // Camera position (mucho más lejos)
        cVector3d(0.0, 0.0, 0.0),       // Look at center
        cVector3d(0.0, 1.0, 0.0));      // Up vector
    camera->setClippingPlanes(0.01, 1000.0);


    // Create a light source
    light = new cDirectionalLight(world);
    world->addChild(light);
    light->setEnabled(true);
    light->setDir(-1.0, -1.0, -1.0);

    // Load STL model
    cMultiMesh* mainObject = loadModel("./stls/box_wo_cover.stl");
    if (!mainObject) return -1;
    world->addChild(mainObject);

    // Cargar modelos hijos
    vector<string> modelPaths = {
        "./stls/ball1.stl",
        "./stls/ball2.stl",
        "./stls/ball3.stl",
        "./stls/stick1.stl",
        "./stls/stick2.stl",
        "./stls/stick3.stl",
        "./stls/cube.stl",
        "./stls/stick4.stl",
        "./stls/cover.stl",
    };

    for (const string& path : modelPaths)
    {
        cMultiMesh* child = loadModel(path);
        if (child)
        {
            mainObject->addChild(child); // Agregar cada hijo al objeto principal
        }
    }

    // Compute bounding box for the main object
    mainObject->computeBoundaryBox(true);
    cVector3d min = mainObject->getBoundaryMin();
    cVector3d max = mainObject->getBoundaryMax();
    cVector3d center = (min + max) * 0.5;

    // Translate the object to the center
    mainObject->translate(-center);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Render scene
        updateGraphics();

        // Poll events
        glfwPollEvents();
    }

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}