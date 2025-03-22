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
// CONSTANTS
//------------------------------------------------------------------------------

string objects[8] = { "Blue ball", "Red ball", "Green ball", "Orange cube", "Blue stick", "Yellow stick", "Pink stick", "Gray stick"};

//------------------------------------------------------------------------------
// DECLARED VARIABLES
//------------------------------------------------------------------------------
cWorld* world;
cCamera* camera;

// a viewport to display the scene viewed by the camera
cViewport* viewport = nullptr;

// fullscreen mode
bool fullscreen = false;

// mirrored display
bool mirroredDisplay = false;

cDirectionalLight* light;
cSpotLight* mobileLight;
cMultiMesh* needle;
cMultiMesh* mainObject;
cBackground* background;
cFontPtr font;
cMaterial* material;
cCamera* needleCamera;
cViewPanel* viewPanel;
cFrameBufferPtr needleFrameBuffer;

// a label to display the rate [Hz] at which the simulation is running
cLabel* labelRates;

// a label to explain what is happening
cLabel* labelMessage;

// a widget panel
cPanel* panel;

// some labels
cLabel* instrucciones;
cLabel* object1;
cLabel* object2;

GLFWwindow* window = nullptr;

cMultiMesh* model;

// a flag to indicate if the haptic simulation currently running
bool simulationRunning = false;

// a flag to indicate if the haptic simulation has terminated
bool simulationFinished = true;

// a frequency counter to measure the simulation graphic rate
cFrequencyCounter freqCounterGraphics;

// a frequency counter to measure the simulation haptic rate
cFrequencyCounter freqCounterHaptics;

// mouse position
double mouseX, mouseY;

// current size of GLFW window
int windowW = 0;
int windowH = 0;

// current size of GLFW framebuffer
int framebufferW = 0;
int framebufferH = 0;

// mouse state
MouseStates mouseState = MOUSE_IDLE;

// Variables para el zoom
double cameraDistance = 500.0; // Distancia inicial de la c�mara
const double zoomSpeed = 30.0; // Velocidad del zoom

// POSITION THE NEEDLE INSIDE THE HOLES
std::vector<cVector3d> holePositions = {
    cVector3d(-60.0, -162.0, 60.0), // Posición del agujero 1 (-60, -162, 60)
    cVector3d(-27.0, -162.0, 60.0),   // Posición del agujero 2  (-27, -162, 60)
    cVector3d(6.0, -162.0, 60.0),   // Posición del agujero 3 (6, -162, 60)
    cVector3d(-60.0, -162.0, 27.0),   // Posición del agujero 4  (-60, -162, 27)
    cVector3d(-27.0, -162.0, 27.0),    // Posición del agujero 5  (-27, -162, 27)
    cVector3d(6.0, -162.0, 27.0),    // Posición del agujero 6 (6, -162, 27)
    cVector3d(-60.0, -165.0, -6.0),   // Posición del agujero 7 (-60, -165, -6)
    cVector3d(-27.0, -165.0, -6.0),    // Posición del agujero 8 (-27, -165, -6)
    cVector3d(6.0, -165.0, -6.0)     // Posición del agujero 9  (6, -165, -6)
};

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

// callback when the window is resized
void onWindowSizeCallback(GLFWwindow* a_window, int a_width, int a_height);

// callback when the window framebuffer is resized
void onFrameBufferSizeCallback(GLFWwindow* a_window, int a_width, int a_height);

// callback when window content scaling is modified
void onWindowContentScaleCallback(GLFWwindow* a_window, float a_xscale, float a_yscale);

// callback when an error GLFW occurs
void onErrorCallback(int a_error, const char* a_description);

// Callback to render scene
void renderGraphics();

// this function contains the main haptics simulation loop
void renderHaptics(void);

// this function closes the application
void close(void);

void updateCameraPosition();


// Función para cargar un modelo y crear un contenedor de mallas múltiples
cMultiMesh* loadModel(const std::string& filepath)
{
    model = new cMultiMesh();
    if (!model->loadFromFile(filepath))
    {
        cout << "Error: Could not load STL file " << filepath << endl;
        return nullptr;
    }
    return model;
}

//Declare the function check collisions
void checkCollisions();



//------------------------------------------------------------------------------
// MAIN FUNCTION
//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    //--------------------------------------------------------------------------
    // INITIALIZATION
    //--------------------------------------------------------------------------

    cout << endl;
    cout << "-----------------------------------" << endl;
    cout << "CHAI3D" << endl;
    cout << "OPERATION: THE GAME" << endl;
    cout << "Grupo 1" << endl;
    cout << "-----------------------------------" << endl << endl << endl;
    cout << "Keyboard Options:" << endl << endl;
    cout << "[← →] - Move the needle sideways" << endl;
    cout << "[↑ ↓] - Move the needle inside/outside the box" << endl;
    cout << "[w] - Move the needle up" << endl;
    cout << "[w] - Move the needle down" << endl;
    cout << "";
    cout << endl << endl;

    // Initialize GLFW
    if (!glfwInit())
    {
        cout << "Failed to initialize GLFW" << endl;
        cSleepMs(1000);
        return -1;
    }

    // set GLFW error callback
    glfwSetErrorCallback(onErrorCallback);

    // compute desired size of window
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    windowW = 0.8 * mode->height;
    windowH = 0.5 * mode->height;
    int x = 0.5 * (mode->width - windowW);
    int y = 0.5 * (mode->height - windowH);

    // set OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // enable double buffering
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

    // set the desired number of samples to use for multisampling
    glfwWindowHint(GLFW_SAMPLES, 4);

    // specify that window should be resized based on monitor content scale
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    // Create Window
    window = glfwCreateWindow(windowW, windowH, "Chai3D STL Viewer", NULL, NULL);
    if (!window)
    {
        cout << "Failed to create window" << endl;
        cSleepMs(1000);
        glfwTerminate();
        return 1;
    }

    // set GLFW key callback
    glfwSetKeyCallback(window, onKeyCallback);

    // set GLFW window size callback
    glfwSetWindowSizeCallback(window, onWindowSizeCallback);

    // set GLFW framebuffer size callback
    glfwSetFramebufferSizeCallback(window, onFrameBufferSizeCallback);

    // set GLFW window content scaling callback
    glfwSetWindowContentScaleCallback(window, onWindowContentScaleCallback);

    // set GLFW mouse position callback
    glfwSetCursorPosCallback(window, onMouseMotionCallback);

    // set GLFW mouse button callback
    glfwSetMouseButtonCallback(window, onMouseButtonCallback);

    // get width and height of window
    glfwGetFramebufferSize(window, &framebufferW, &framebufferH);

    // set position of window
    glfwSetWindowPos(window, x, y);

    // set window size
    glfwSetWindowSize(window, windowW, windowH);

    // set GLFW current display context
    glfwMakeContextCurrent(window);

#ifdef GLEW_VERSION
    // initialize GLEW library
    if (glewInit() != GLEW_OK)
    {
        cout << "failed to initialize GLEW library" << endl;
        glfwTerminate();
        return 1;
    }
#endif

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
    camera->setStereoEyeSeparation(0.03);
    camera->setStereoFocalLength(1.8);

    // set vertical mirrored display mode
    camera->setMirrorVertical(mirroredDisplay);

    // Create a light source
    light = new cDirectionalLight(world);
    world->addChild(light);
    // camera->addChild(light);
    light->setEnabled(true);
    light->setLocalPos(0.0, 0.5, 0.0);
    light->setDir(1.0, 1.0, -1.0);

    // Load STL model
    mainObject = loadModel("./stls/box_wo_cover.stl");
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

    // Rotar la aguja para que quede en posici�n horizontal
    cMatrix3d rotation;
    rotation.identity();
    rotation.rotateAboutGlobalAxisDeg(cVector3d(0.0, 0.0, 1.0), 90); // Rotar 90 grados alrededor del eje Z
    needle->setLocalRot(rotation);
    // needle->setLocalPos(cVector3d(0.0, 0.0, 0.0)); NOT NEEDED
    // Posición inicial de la aguja (agujero central)
    needle->setLocalPos(holePositions[4]); // El índice 4 corresponde al agujero 5
    needle->setShowBoundaryBox(false);
    world->addChild(needle); // A�adir la aguja al mundo, no al objeto principal

    // Obtener la posición local de la aguja
    cVector3d needlePos = needle->getLocalPos();
    // Imprimir la posición de la aguja
    std::cout << "Posición de la aguja: "
        << "(" << needlePos.x() << ", "
        << needlePos.y() << ", "
        << needlePos.z() << ")" << std::endl;

    cVector3d needleTip = needlePos + cVector3d(-10.0, 50, 0.0);

    // Imprimir la posición de la punta de la aguja
    std::cout << "Posición de la punta de la aguja: "
        << "(" << needleTip.x() << ", "
        << needleTip.y() << ", "
        << needleTip.z() << ")" << std::endl;



    //NEEDLE CAMERA

    needleCamera = new cCamera(world);
    world->addChild(needleCamera);
    needleCamera->setLocalPos(needleTip);

    needleCamera->setSphericalReferences(
        needleTip,  // Punto de referencia (centro de la aguja)
        cVector3d(0.0, 1.0, 0.0),
        cVector3d(0.0, 0.0, -1.0)
    );

    cVector3d cameraPos = needleCamera->getLocalPos();
    needleCamera->setSphericalDeg(0.0, 0.0, 0.0);
    needleCamera->setClippingPlanes(0.01, 1000.0);

    // Needle light
    mobileLight = new cSpotLight (world);
    world->addChild(mobileLight);
    mobileLight->setEnabled(true);
    mobileLight->setParent(needleCamera);
    mobileLight->setLocalPos(needleTip);
    mobileLight->setDir(1.0, 1.0, -1.0);

    // Crear un framebuffer para la cámara de la aguja
    needleFrameBuffer = cFrameBuffer::create();
    needleFrameBuffer->setup(needleCamera, 200, 200, true, true);
    needleCamera->renderView(200, 200);


    // Crear el panel de vista con el framebuffer
    viewPanel = new cViewPanel(needleFrameBuffer);
    camera->m_frontLayer->addChild(viewPanel);
    viewPanel->setFrameBuffer(needleFrameBuffer);

    viewPanel->setSize(150, 150);
    viewPanel->setLocalPos(0, 0, 0);
    viewPanel->setShowEnabled(true);

    world->addChild(viewPanel);


    // Compute boundary box for the main object
    mainObject->computeBoundaryBox(true);
    cVector3d min = mainObject->getBoundaryMin();
    cVector3d max = mainObject->getBoundaryMax();
    cVector3d center = (min + max) * 0.5;
    cVector3d boxSize = max - min; // Revisar (no se usa)

    // Translate the object to the center
    mainObject->translate(-center);

//--------------------------------------------------------------------------
// WIDGETS
//--------------------------------------------------------------------------

// create a font
    font = NEW_CFONT_CALIBRI_20();

    // create a label to display the haptic and graphic rate of the simulation
    labelRates = new cLabel(font);
    camera->m_frontLayer->addChild(labelRates);

    // set font color
    labelRates->m_fontColor.setBlack();

    // create a background
    background = new cBackground();
    camera->m_backLayer->addChild(background);

    // set background properties
    background->setCornerColors(cColorf(1.0, 1.0, 1.0),
        cColorf(1.0, 1.0, 1.0),
        cColorf(0.8, 0.8, 0.8),
        cColorf(0.8, 0.8, 0.8));

    // a widget panel
    panel = new cPanel();
    camera->m_frontLayer->addChild(panel);
    panel->setSize(500, 70);
    panel->setColor(cColorf(230.0 / 255.0, 209.0 / 255.0, 34.0 / 255.0));
    panel->setTransparencyLevel(0.8);

    // instructions
    instrucciones = new cLabel(font);
    panel->addChild(instrucciones);
    instrucciones->setText("To make him better, touch the following objects with the needle:");
    instrucciones->setLocalPos(10, 50, 0.1);
    instrucciones->m_fontColor.setBlack();

    int num1 = rand() % 7 + 0;
    object1 = new cLabel(font);
    panel->addChild(object1);
    object1->setText(objects[num1]);
    object1->setLocalPos(10, 30, 0.1);
    object1->m_fontColor.setBlack();

    int num2 = rand() % 7 + 0;
    object2 = new cLabel(font);
    panel->addChild(object2);
    object2->setText(objects[num2]);
    object2->setLocalPos(10, 10, 0.1);
    object2->m_fontColor.setBlack();

    // create a label with a small message
    labelMessage = new cLabel(font);
    camera->m_frontLayer->addChild(labelMessage);

    // set font color
    labelMessage->m_fontColor.setBlack();

    // set text message
    labelMessage->setText("OPERATION. Make him better or get the buzzer!");

    //--------------------------------------------------------------------------
    // VIEWPORT DISPLAY
    //--------------------------------------------------------------------------

    // get content scale factor
    float contentScaleW, contentScaleH;
    glfwGetWindowContentScale(window, &contentScaleW, &contentScaleH);

    // create a viewport to display the scene.
    viewport = new cViewport(camera, contentScaleW, contentScaleH);


    //compute boundary box for the needle 
    needle->computeBoundaryBox(true);
    cVector3d minNeedle = needle->getBoundaryMin();
    cVector3d maxNeedle = needle->getBoundaryMax();



    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        
        // Vérifier les collisions
        checkCollisions();

        // Render scene
        renderGraphics();

        // Poll events
        glfwPollEvents();

    }

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}





/* 
void renderGraphics() {
    // Limpiar el buffer de color y profundidad para la vista principal
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Renderizar la cámara principal usando todo el tamaño de la ventana
    camera->setSphericalRadius(cameraDistance);
    camera->renderView(800, 600);  // Renderiza la vista completa desde la cámara principal

   

    // Finalmente, actualizar la ventana
    glfwSwapBuffers(window);
}
*/


void renderGraphics(void)
{
    // sanity check
    if (viewport == nullptr) { return; }

    /////////////////////////////////////////////////////////////////////
    // UPDATE WIDGETS
    /////////////////////////////////////////////////////////////////////

    // get width and height of CHAI3D internal rendering buffer
    int displayW = viewport->getDisplayWidth();
    int displayH = viewport->getDisplayHeight();

    // update haptic and graphic rate data
    labelRates->setText(cStr(freqCounterGraphics.getFrequency(), 0) + " Hz / " +
        cStr(freqCounterHaptics.getFrequency(), 0) + " Hz");

    // update position of label
    labelRates->setLocalPos((int)(0.5 * (displayW - labelRates->getWidth())), 15);

    // update panel position
    panel->setLocalPos(10, (displayH - panel->getHeight()) - 10);

    // update position of label
    labelMessage->setLocalPos((int)(0.5 * (displayW - labelMessage->getWidth())), 40);

    /////////////////////////////////////////////////////////////////////
    // UPDATE NEEDLE CAMERA POSITION
    /////////////////////////////////////////////////////////////////////

     // Actualizar la posición de la cámara de la aguja
    updateCameraPosition();

    // Renderizar el framebuffer del panel de vista
    needleFrameBuffer->renderView();

    /////////////////////////////////////////////////////////////////////
    // RENDER SCENE
    /////////////////////////////////////////////////////////////////////

    // update shadow maps (if any)
    world->updateShadowMaps(false, mirroredDisplay);

    // render world
    viewport->renderView(framebufferW, framebufferH);

    // wait until all GL commands are completed
    glFinish();

    // check for any OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) cout << "Error:  %s\n" << gluErrorString(error);

    // swap buffers
    glfwSwapBuffers(window);

    // signal frequency counter
    freqCounterGraphics.signal(1);
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

/*
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
*/


void onMouseButtonCallback(GLFWwindow* a_window, int a_button, int a_action, int a_mods)
{
    if (a_button == GLFW_MOUSE_BUTTON_LEFT && a_action == GLFW_PRESS)
    {
        // store mouse position
        glfwGetCursorPos(window, &mouseX, &mouseY);

        // update mouse state
        mouseState = MOUSE_MOVE_CAMERA;

        // variable for storing collision information
        cCollisionRecorder recorder;
        cCollisionSettings settings;

        // detect for any collision between mouse and front layer widgets
        bool hit = viewport->selectFrontLayer(mouseX, (windowH - mouseY), windowW, windowH, recorder, settings);
        if (hit)
        {
            // reset all label font colors to white
            instrucciones->m_fontColor.setWhite();
        }
        else
        {
            // detect for any collision between mouse and world
            hit = camera->selectWorld(mouseX, (windowH - mouseY), windowW, windowH, recorder, settings);
        }
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

void close(void)
{
    // stop the simulation
    simulationRunning = false;

    // wait for graphics and haptics loops to terminate
    while (!simulationFinished) { cSleepMs(100); }
}

// Callback para controlar el movimiento con las teclas
void onKeyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods)
{
    // Solo mover la aguja cuando la tecla es presionada
    if (a_action == GLFW_PRESS || a_action == GLFW_REPEAT)
    {
        double moveSpeed = 3.0;  // Velocidad de movimiento de la aguja
        cVector3d currentPos = needle->getLocalPos();
        if (a_mods & GLFW_MOD_SHIFT)
        {
            if (a_key == GLFW_KEY_UP && a_action == GLFW_PRESS)
            {
                needle->setLocalPos(currentPos + cVector3d(0.0, moveSpeed, 0.0));  // Mover hacia dentro
            }
            if (a_key == GLFW_KEY_DOWN && a_action == GLFW_PRESS)
            {
                needle->setLocalPos(currentPos - cVector3d(0.0, moveSpeed, 0.0));  // Mover hacia fuera
            }
        }
        else if (a_key == GLFW_KEY_UP)
        {
            needle->setLocalPos(currentPos + cVector3d(0.0, 0.0, moveSpeed));  // Mover hacia arriba
        }
        else if (a_key == GLFW_KEY_DOWN)
        {
            needle->setLocalPos(currentPos - cVector3d(0.0, 0.0, moveSpeed));  // Mover hacia abajo
        }
        else if (a_key == GLFW_KEY_LEFT)
        {
            needle->setLocalPos(currentPos - cVector3d(moveSpeed, 0.0, 0.0));  // Mover hacia la izquierda
        }
        else if (a_key == GLFW_KEY_RIGHT)
        {
            needle->setLocalPos(currentPos + cVector3d(moveSpeed, 0.0, 0.0));  // Mover hacia la derecha
        }

        /// GET NEEDLE POSITION
        //cVector3d needlePosition = needle->getLocalPos(); // Obtener la posición local de la aguja
        //cout << "Posición de la aguja: ("
        //    << needlePosition.x() << ", "
        //    << needlePosition.y() << ", "
        //    << needlePosition.z() << ")"
        //    << endl;
        ///
        // Mover la aguja según la tecla presionada
        if (a_key >= GLFW_KEY_1 && a_key <= GLFW_KEY_9)
        {
            int holeIndex = a_key - GLFW_KEY_1; // Convertir la tecla a un índice (0-8)
            if (holeIndex < holePositions.size())
            {
                // Mover la aguja a la posición del agujero correspondiente
                needle->setLocalPos(holePositions[holeIndex]);
            }
        }
    }
}

/*
void onKeyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods)
{
    // filter calls that only include a key press
    if ((a_action != GLFW_PRESS) && (a_action != GLFW_REPEAT))
    {
        return;
    }

    // option - exit
    else if ((a_key == GLFW_KEY_ESCAPE) || (a_key == GLFW_KEY_Q))
    {
        glfwSetWindowShouldClose(a_window, GLFW_TRUE);
    }

    // option - toggle fullscreen
    else if (a_key == GLFW_KEY_F)
    {
        // toggle state variable
        fullscreen = !fullscreen;

        // get handle to monitor
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();

        // get information about monitor
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        // set fullscreen or window mode
        if (fullscreen)
        {
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        }
        else
        {
            int w = 0.8 * mode->height;
            int h = 0.5 * mode->height;
            int x = 0.5 * (mode->width - w);
            int y = 0.5 * (mode->height - h);
            glfwSetWindowMonitor(window, NULL, x, y, w, h, mode->refreshRate);
        }

        // set the desired swap interval and number of samples to use for multisampling
        // glfwSwapInterval(swapInterval);
        glfwWindowHint(GLFW_SAMPLES, 4);
    }

    // option - toggle vertical mirroring
    else if (a_key == GLFW_KEY_M)
    {
        mirroredDisplay = !mirroredDisplay;
        camera->setMirrorVertical(mirroredDisplay);
    }
}
*/


void onWindowSizeCallback(GLFWwindow* a_window, int a_width, int a_height)
{
    // update window size
    windowW = a_width;
    windowH = a_height;

    // render scene
    renderGraphics();
}

//------------------------------------------------------------------------------

void onFrameBufferSizeCallback(GLFWwindow* a_window, int a_width, int a_height)
{
    // update frame buffer size
    framebufferW = a_width;
    framebufferH = a_height;
}

void onWindowContentScaleCallback(GLFWwindow* a_window, float a_xscale, float a_yscale)
{
    // update window content scale factor
    viewport->setContentScale(a_xscale, a_yscale);
}

void onErrorCallback(int a_error, const char* a_description)
{
    cout << "Error: " << a_description << endl;
}

void renderHaptics(void)
{
    // simulation in now running
    simulationRunning = true;
    simulationFinished = false;

    // main haptic simulation loop
    while (simulationRunning)
    {
        /////////////////////////////////////////////////////////////////////////
        // HAPTIC RENDERING
        /////////////////////////////////////////////////////////////////////////

        // signal frequency counter
        freqCounterHaptics.signal(1);

        // compute global reference frames for each object
        world->computeGlobalPositions(true);
    }

    // exit haptics thread
    simulationFinished = true;
}
void updateCameraPosition() {
    
    cVector3d needlePos = needle->getLocalPos();
    cVector3d needleTip = needlePos + cVector3d(28, 50, 0);  
    cVector3d cameraPosition = needleTip; // Ajusta los valores para más distancia y altura
    needleCamera->setLocalPos(cameraPosition);

    needleCamera->setSphericalReferences(
        needleTip,               // La cámara sigue la punta de la aguja
        cVector3d(0.0, -1.0, 0.0),  // Mirando hacia adelante, ligeramente inclinada en Z
        cVector3d(0.0, 0.0, -1.0) // Dirección del acimut (eje X)
    );

    // Actualizar la vista renderizada de la cámara
    needleCamera->renderView(200, 200);  // Cambia el tamaño del renderizado si es necesario
}


//-------------------------------------------------------------------------------------
//COLLISIONS
//-------------------------------------------------------------------------------------
void checkCollisions()
{
    // Verify that `needle` and `mainObject` are charged when the function is called
    if (!needle || !mainObject) {
        cout << "ERROR: Needle or main objet not itialised !" << endl;
        return;
    }

    //  Upload the boundary box after transformation
    // Refresh the global position of the objects
    world->computeGlobalPositions(true);

    // Charge the global positions and the boundary box limits
    cVector3d needleGlobalPos = needle->getGlobalPos();
    needle->computeBoundaryBox(true);
    cVector3d minNeedleGlobal = needle->getBoundaryMin() + needleGlobalPos;
    cVector3d maxNeedleGlobal = needle->getBoundaryMax() + needleGlobalPos;

    cout << "Needle Global Position: (" << needleGlobalPos.x() << ", " << needleGlobalPos.y() << ", " << needleGlobalPos.z() << ")" << endl;
    cout << "Needle Min: (" << minNeedleGlobal.x() << ", " << minNeedleGlobal.y() << ", " << minNeedleGlobal.z() << ") - Max: (" << maxNeedleGlobal.x() << ", " << maxNeedleGlobal.y() << ", " << maxNeedleGlobal.z() << ")" << endl;

    for (int i = 0; i < mainObject->getNumChildren(); i++) {
        cMultiMesh* objectMesh = dynamic_cast<cMultiMesh*>(mainObject->getChild(i));

        if (objectMesh) {
            for (int j = 0; j < objectMesh->getNumMeshes(); j++) {
                cMesh* mesh = objectMesh->getMesh(j);

                if (mesh) {
                    mesh->computeBoundaryBox(true);
                    cVector3d minObj = mesh->getBoundaryMin();
                    cVector3d maxObj = mesh->getBoundaryMax();

                    cout << "Object " << i << " Mesh " << j << " Min: (" << minObj.x() << ", " << minObj.y() << ", " << minObj.z() << ") - Max: (" << maxObj.x() << ", " << maxObj.y() << ", " << maxObj.z() << ")" << endl;

                    // Check if a collision occur with the boundary boxes 
                    if (minNeedleGlobal.x() < maxObj.x() && maxNeedleGlobal.x() > minObj.x() &&
                        minNeedleGlobal.y() < maxObj.y() && maxNeedleGlobal.y() > minObj.y() &&
                        minNeedleGlobal.z() < maxObj.z() && maxNeedleGlobal.z() > minObj.z()) {
                        cout << "Collision detected with object " << i << "!" << endl;
                    }
                }
            }
        }
    }

}