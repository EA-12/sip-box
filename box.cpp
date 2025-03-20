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
cMultiMesh* needle;
cMultiMesh* mainObject;
GLFWwindow* window;
cBackground* background;
cViewPanel* viewPanel;
cFontPtr font;
cMaterial* material;
cCamera* needleCamera;



// mouse state
MouseStates mouseState = MOUSE_IDLE;

// last mouse position
double mouseX, mouseY;

// Variables para el zoom
double cameraDistance = 500.0; // Distancia inicial de la c�mara
const double zoomSpeed = 30.0; // Velocidad del zoom

//------------------------------------------------------------------------------
// DECLARED FUNCTIONS
//------------------------------------------------------------------------------
// callback to handle mouse click
void onMouseButtonCallback(GLFWwindow* a_window, int a_button, int a_action, int a_mods);

// callback to handle mouse motion
void onMouseMotionCallback(GLFWwindow* a_window, double a_posX, double a_posY);

// callback to handle mouse scroll
void onMouseScrollCallback(GLFWwindow* a_window, double a_offsetX, double a_offsetY);

// callback para teclado
void onKeyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods);

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



//------------------------------------------------------------------------------
// MAIN FUNCTION
//------------------------------------------------------------------------------

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

    glfwSetKeyCallback(window, onKeyCallback);


    // Create a new world
    world = new cWorld();

    // LIGHTNING
    world->m_backgroundColor.set(0.9f, 0.9f, 0.9f);

    // Create a camera and position it FARTHER
    camera = new cCamera(world);
    world->addChild(camera);
    // Configurar la camara en coordenadas esfericas
    camera->setSphericalReferences(cVector3d(0.0, 0.0, 0.0), // Origen (centro de la escena)
        cVector3d(0.0, 0.0, 1.0), // Direcci�n del cenit (eje Z)
        cVector3d(1.0, 0.0, 0.0)); // Direcci�n del acimut (eje X)

    // Configurar posicion inicial de la c�mara
    camera->setSphericalDeg(cameraDistance, // Radio (distancia)
        0.0,          // �ngulo polar (inclinaci�n)
        0.0);         // �ngulo acimutal (rotaci�n horizontal)
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


    // Aplicar color al mainObject
    material = new cMaterial();
    cColorf color(1.0f, 0.6f, 0.6f);  
    material->setColor(color);
    mainObject->setMaterial(*material);



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
        // "./stls/cover.stl", // descomentar si se quiere usar la tapa
    };



    for (const string& path : modelPaths)
    {
        cMultiMesh* child = loadModel(path);
        if (child)
        {
            mainObject->addChild(child); // Agregar cada hijo al objeto principal
        }
    }

    // Crear una lista de colores para los hijos
    vector<cColorf> coloresHijos = {
        cColorf(1.0f, 0.0f, 0.0f),  // Rojo
        cColorf(0.0f, 1.0f, 0.0f),  // Verde
        cColorf(0.0f, 0.0f, 1.0f),  // Azul
        cColorf(1.0f, 1.0f, 0.0f),  // Amarillo
        cColorf(1.0f, 0.0f, 1.0f),  // Magenta
        cColorf(0.0f, 1.0f, 1.0f),  // Cian
        cColorf(1.0f, 0.5f, 0.0f),  // Naranja
        cColorf(0.5f, 0.5f, 0.5f),  // Gris
    };

    // Cargar los modelos hijos y asignarles colores
    for (size_t i = 0; i < modelPaths.size(); ++i)
    {
        cMultiMesh* child = loadModel(modelPaths[i]);
        if (child)
        {
            cMaterial* materialHijo = new cMaterial();
            materialHijo->setColor(coloresHijos[i % coloresHijos.size()]); // Usar un color de la lista cíclicamente
            child->setMaterial(*materialHijo);
            mainObject->addChild(child);
        }
    }



    // LOAD NEEDLE
    needle = new cMultiMesh();

    // load a needle-like mesh and attach it to the tool
    bool fileload = needle->loadFromFile("./stls/needle.stl");
    if (!fileload) {
        std::cout << "Error: Could not load needle.stl" << std::endl;
        return -1;
    }

    // resize tool mesh model
    needle->scale(2.0);
    // demo

    // Rotar la aguja para que quede en posici�n horizontal
    cMatrix3d rotation;
    rotation.identity();
    rotation.rotateAboutGlobalAxisDeg(cVector3d(0.0, 0.0, 1.0), 90); // Rotar 90 grados alrededor del eje Z
    needle->setLocalRot(rotation);
    needle->setLocalPos(cVector3d(0.0, 0.0, 0.0));
    needle->setShowBoundaryBox(true);
    world->addChild(needle); // A�adir la aguja al mundo, no al objeto principal

    //NEEDLE CAMERA
    
    needleCamera = new cCamera(world);
    world->addChild(needleCamera);
    needleCamera->setSphericalDeg(cameraDistance, 0.0, 0.0);
    needleCamera->setClippingPlanes(0.01, 1000.0);

 



    // Compute boundary box for the main object
    mainObject->computeBoundaryBox(true);
    cVector3d min = mainObject->getBoundaryMin();
    cVector3d max = mainObject->getBoundaryMax();
    cVector3d center = (min + max) * 0.5;
    cVector3d boxSize = max - min; // Revisar (no se usa)

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





void updateGraphics() {
    // Limpiar el buffer de color y profundidad para la vista principal
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Renderizar la cámara principal usando todo el tamaño de la ventana
    camera->setSphericalRadius(cameraDistance);
    camera->renderView(800, 600);  // Renderiza la vista completa desde la cámara principal

   

    // Finalmente, actualizar la ventana
    glfwSwapBuffers(window);
}


void onMouseScrollCallback(GLFWwindow* a_window, double a_offsetX, double a_offsetY)
{
    // Ajustar la distancia de la c�mara seg�n el movimiento de la rueda
    cameraDistance -= a_offsetY * zoomSpeed;

    // Limitar el rango de zoom
    cameraDistance = cClamp(cameraDistance, 100.0, 1000.0);
    // Aplicar la nueva distancia a la c�mara
    camera->setSphericalRadius(cameraDistance);
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
// Callback para controlar el movimiento con las teclas
void onKeyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods)
{
    // Solo mover la aguja cuando la tecla es presionada
    if (a_action == GLFW_PRESS || a_action == GLFW_REPEAT)
    {
        double moveSpeed = 10.0;  // Velocidad de movimiento de la aguja
        cVector3d currentPos = needle->getLocalPos();

        if (a_key == GLFW_KEY_UP)
        {
            needle->setLocalPos(currentPos + cVector3d(0.0, moveSpeed, 0.0));  // Mover hacia arriba
        }
        else if (a_key == GLFW_KEY_DOWN)
        {
            needle->setLocalPos(currentPos - cVector3d(0.0, moveSpeed, 0.0));  // Mover hacia abajo
        }
        else if (a_key == GLFW_KEY_LEFT)
        {
            needle->setLocalPos(currentPos - cVector3d(moveSpeed, 0.0, 0.0));  // Mover hacia la izquierda
        }
        else if (a_key == GLFW_KEY_RIGHT)
        {
            needle->setLocalPos(currentPos + cVector3d(moveSpeed, 0.0, 0.0));  // Mover hacia la derecha
        }
        else if (a_key == GLFW_KEY_W)
        {
            needle->setLocalPos(currentPos + cVector3d(0.0, 0.0, moveSpeed));  // Mover hacia adelante (eje Z)
        }
        else if (a_key == GLFW_KEY_S)
        {
            needle->setLocalPos(currentPos - cVector3d(0.0, 0.0, moveSpeed));  // Mover hacia atrás (eje Z)
        }
    }
}

