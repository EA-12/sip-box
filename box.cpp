#include "chai3d.h"
#include <GLFW/glfw3.h>
#include <vector>

using namespace chai3d;
using namespace std;

//------------------------------------------------------------------------------
// STATES
//------------------------------------------------------------------------------
enum MouseStates
{
    MOUSE_IDLE,
    MOUSE_MOVE_CAMERA
};

//------------------------------------------------------------------------------
// DECLARED VARIABLES
//------------------------------------------------------------------------------
cWorld* world;
cCamera* camera;
cDirectionalLight* light;
GLFWwindow* window;
// mouse state
MouseStates mouseState = MOUSE_IDLE;

// last mouse position
double mouseX, mouseY;

//------------------------------------------------------------------------------
// DECLARED FUNCTIONS
//------------------------------------------------------------------------------
// callback to handle mouse click
void onMouseButtonCallback(GLFWwindow* a_window, int a_button, int a_action, int a_mods);

// callback to handle mouse motion
void onMouseMotionCallback(GLFWwindow* a_window, double a_posX, double a_posY);

// callback to handle mouse scroll
void onMouseScrollCallback(GLFWwindow* a_window, double a_offsetX, double a_offsetY);

// Callback to render scene
void updateGraphics();

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

    // set GLFW mouse scroll callback
    glfwSetScrollCallback(window, onMouseScrollCallback);

    // set GLFW mouse position callback
    glfwSetCursorPosCallback(window, onMouseMotionCallback);

    // set GLFW mouse button callback
    glfwSetMouseButtonCallback(window, onMouseButtonCallback);

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

void updateGraphics()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera->renderView(800, 600);
    glfwSwapBuffers(window);
}

void onMouseScrollCallback(GLFWwindow* a_window, double a_offsetX, double a_offsetY)
{
    double r = camera->getSphericalRadius();
    double zoomSpeed = 0.1;
    r -= zoomSpeed * a_offsetY;  // Restar para acercar con scroll arriba, alejar con scroll abajo
    // Limitar el rango del radio para evitar valores extremos
    r = cClamp(r, 0.2, 10.0);
    // Establecer el nuevo radio en la cámara
    camera->setSphericalRadius(r);
}

void onMouseButtonCallback(GLFWwindow* a_window, int a_button, int a_action, int a_mods)
{
    if (a_button == GLFW_MOUSE_BUTTON_LEFT && a_action == GLFW_PRESS)
    {
        // store mouse position
        glfwGetCursorPos(a_window, &mouseX, &mouseY);

        // update mouse state
        mouseState = MOUSE_MOVE_CAMERA;
    }

    else
    {
        // update mouse state
        mouseState = MOUSE_IDLE;
    }
}

void onMouseMotionCallback(GLFWwindow* a_window, double a_posX, double a_posY)
{
    if (mouseState == MOUSE_MOVE_CAMERA)
    {
        // compute mouse motion
        int dx = a_posX - mouseX;
        int dy = a_posY - mouseY;
        mouseX = a_posX;
        mouseY = a_posY;

        // compute new camera angles
        double azimuthDeg = camera->getSphericalAzimuthDeg() - 0.5 * dx;
        double polarDeg = camera->getSphericalPolarDeg() - 0.5 * dy;

        // assign new angles
        camera->setSphericalAzimuthDeg(azimuthDeg);
        camera->setSphericalPolarDeg(polarDeg);
    }
}