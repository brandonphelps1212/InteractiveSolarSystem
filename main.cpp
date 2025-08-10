// main.cpp - interactions + picking + HUD + pure OBJ model (AcrimSAT) orbiting Earth
// FIXES:
//  - Removed accidental Skybox copy (double free).
//  - Ensured a valid texture bound before drawing OBJ (silences sampler warning).

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <sstream>
#include <vector>
#include <limits>
#include <algorithm>

#include "Shader.h"
#include "Camera/Camera.h"
#include "Texture.h"
#include "SolarSystem.h"
#include "Skybox.h"
#include "AsteroidBelt.h"
#include "Model.h"

// ====== stb_easy_font (public domain) ======
#define STB_EASY_FONT_IMPLEMENTATION
#include "stb_easy_font.h"
// ==========================================

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

Camera camera(glm::vec3(0.0f, 0.0f, 25.0f));

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool cursorCaptured = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

static SolarSystem *gSolar = nullptr;

// HUD globals
static Shader *gHudShader = nullptr;
static unsigned int gHudVAO = 0, gHudVBO = 0, gHudEBO = 0;
static unsigned int gPanelVAO = 0, gPanelVBO = 0, gPanelEBO = 0;
static float gHudAlpha = 0.0f;
static double gHudShowTimer = 0.0;

// Model globals
static ObjModel gAcrimSAT;
static bool gShowSat = true;
static GLuint gWhiteTex = 0;

// Earth orbiting
static int gEarthIdx = -1;
static float gSatAngle = 0.0f;                      // radians around Earth
static float gSatAngularSpeed = 0.8f;               // rad/sec (sim units)
static float gSatOrbitRadius = 2.2f;                // distance from Earth's center (scene units)
static float gSatInclination = glm::radians(28.0f); // tilt

// Protos
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window, SolarSystem &solar);
void generateSphere(unsigned int &VAO, unsigned int &VBO, int &vertexCount, int sectorCount = 36, int stackCount = 18);

// HUD helpers
struct HudVertex
{
    float x, y, z;
    unsigned char r, g, b, a;
};
static void hudEnsureBuffers();
static void hudDrawString(GLFWwindow *window, Shader &hudShader, const std::string &text, float x, float y, float alpha);
static void hudDrawPanel(GLFWwindow *window, Shader &hudShader, float x, float y, float w, float h, float alpha);
static void hudRender(GLFWwindow *window, Shader &hudShader, const SolarSystem &solar, float dt);

static void centerCameraOnSolarSystem()
{
    camera.Position = glm::vec3(0.0f, 8.0f, 23.0f);
    camera.Yaw = -90.0f;
    camera.Pitch = -10.0f;
}

static void updateWindowTitle(GLFWwindow *window, const SolarSystem &solar, const std::string &extra = "")
{
    std::ostringstream ss;
    ss.setf(std::ios::fixed);
    ss.precision(2);
    ss << "Interactive Solar System  |  Selected: " << solar.selectedName()
       << "  |  Speed: x" << solar.timeScale()
       << "  |  Paused: " << (solar.isPaused() ? "Yes" : "No");
    if (!extra.empty())
        ss << "  |  " << extra;
    glfwSetWindowTitle(window, ss.str().c_str());
}

static std::pair<glm::vec3, glm::vec3> screenPosToWorldRay(double mouseX, double mouseY, GLFWwindow *window)
{
    int ww, wh;
    glfwGetWindowSize(window, &ww, &wh);
    if (ww <= 0 || wh <= 0)
    {
        ww = SCR_WIDTH;
        wh = SCR_HEIGHT;
    }

    camera.setAspectRatio((float)ww / (float)wh);

    float x = (2.0f * (float)mouseX) / (float)ww - 1.0f;
    float y = 1.0f - (2.0f * (float)mouseY) / (float)wh;
    glm::vec4 ray_clip(x, y, -1.0f, 1.0f);

    glm::mat4 proj = camera.getProjectionMatrix();
    glm::mat4 view = camera.GetViewMatrix();

    glm::vec4 ray_eye = glm::inverse(proj) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

    glm::vec3 ray_wor = glm::normalize(glm::vec3(glm::inverse(view) * ray_eye));
    glm::vec3 origin = camera.Position;

    return {origin, ray_wor};
}

// ---------- HUD ----------
static void hudEnsureBuffers()
{
    if (gHudVAO != 0)
        return;

    glGenVertexArrays(1, &gHudVAO);
    glGenBuffers(1, &gHudVBO);
    glGenBuffers(1, &gHudEBO);

    glBindVertexArray(gHudVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gHudVBO);
    glBufferData(GL_ARRAY_BUFFER, 1, nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gHudEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 1, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(HudVertex), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(HudVertex), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    glGenVertexArrays(1, &gPanelVAO);
    glGenBuffers(1, &gPanelVBO);
    glGenBuffers(1, &gPanelEBO);

    glBindVertexArray(gPanelVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gPanelVBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(HudVertex), nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gPanelEBO);
    unsigned int idx[6] = {0, 1, 2, 2, 3, 0};
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(HudVertex), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(HudVertex), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

static void hudDrawString(GLFWwindow *window, Shader &hudShader, const std::string &text, float x, float y, float alpha)
{
    if (text.empty())
        return;
    hudEnsureBuffers();

    const int MAX = 2048;
    static std::vector<unsigned char> raw;
    raw.resize(MAX * 4 * sizeof(HudVertex));
    unsigned char white[4] = {255, 255, 255, (unsigned char)std::clamp<int>(int(alpha * 255.0f), 0, 255)};

    int num_quads = stb_easy_font_print(x, y, (char *)text.c_str(), white, raw.data(), (int)raw.size());
    if (num_quads <= 0)
        return;

    static std::vector<unsigned int> indices;
    indices.resize(num_quads * 6);
    for (int q = 0; q < num_quads; ++q)
    {
        unsigned base = q * 4;
        indices[q * 6 + 0] = base + 0;
        indices[q * 6 + 1] = base + 1;
        indices[q * 6 + 2] = base + 2;
        indices[q * 6 + 3] = base + 0;
        indices[q * 6 + 4] = base + 2;
        indices[q * 6 + 5] = base + 3;
    }

    glBindVertexArray(gHudVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gHudVBO);
    glBufferData(GL_ARRAY_BUFFER, num_quads * 4 * sizeof(HudVertex), raw.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gHudEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), GL_DYNAMIC_DRAW);

    int ww, wh;
    glfwGetWindowSize(window, &ww, &wh);
    if (ww <= 0 || wh <= 0)
    {
        ww = SCR_WIDTH;
        wh = SCR_HEIGHT;
    }
    glm::mat4 ortho = glm::ortho(0.0f, (float)ww, (float)wh, 0.0f, -1.0f, 1.0f);

    hudShader.use();
    hudShader.setMat4("uProjection", ortho);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
    glDisable(GL_BLEND);
    glBindVertexArray(0);
}

static void hudDrawPanel(GLFWwindow *window, Shader &hudShader, float x, float y, float w, float h, float alpha)
{
    hudEnsureBuffers();
    HudVertex v[4];
    auto setv = [&](int i, float px, float py)
    {
        v[i].x = px;
        v[i].y = py;
        v[i].z = 0.0f;
        v[i].r = 0;
        v[i].g = 0;
        v[i].b = 0;
        v[i].a = (unsigned char)std::clamp<int>(int(alpha * 180.0f), 0, 180);
    };
    setv(0, x, y);
    setv(1, x + w, y);
    setv(2, x + w, y + h);
    setv(3, x, y + h);

    glBindVertexArray(gPanelVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gPanelVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(HudVertex), v);

    int ww, wh;
    glfwGetWindowSize(window, &ww, &wh);
    if (ww <= 0 || wh <= 0)
    {
        ww = SCR_WIDTH;
        wh = SCR_HEIGHT;
    }
    glm::mat4 ortho = glm::ortho(0.0f, (float)ww, (float)wh, 0.0f, -1.0f, 1.0f);

    hudShader.use();
    hudShader.setMat4("uProjection", ortho);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glDisable(GL_BLEND);
    glBindVertexArray(0);
}

static void hudRender(GLFWwindow *window, Shader &hudShader, const SolarSystem &solar, float dt)
{
    gHudShowTimer += dt;
    float target = (gHudShowTimer < 4.0) ? 1.0f : 0.0f;
    float speed = (target > gHudAlpha) ? 4.0f : 1.5f;
    gHudAlpha += (target - gHudAlpha) * std::clamp(dt * speed, 0.0f, 1.0f);
    gHudAlpha = std::clamp(gHudAlpha, 0.0f, 1.0f);
    if (gHudAlpha <= 0.01f)
        return;
    if (solar.selectedIndex() < 0)
        return;

    const std::string name = solar.selectedName();
    const std::string fact = solar.selectedFact();

    std::string wrapped = name + "\n";
    const int wrapLimit = 54;
    int col = 0;
    for (char c : fact)
    {
        wrapped.push_back(c);
        if (c == '\n')
        {
            col = 0;
            continue;
        }
        if (++col >= wrapLimit && c == ' ')
        {
            wrapped.push_back('\n');
            col = 0;
        }
    }

    int ww, wh;
    glfwGetWindowSize(window, &ww, &wh);
    if (ww <= 0 || wh <= 0)
    {
        ww = SCR_WIDTH;
        wh = SCR_HEIGHT;
    }
    int text_w = stb_easy_font_width((char *)wrapped.c_str());
    int lines = 1;
    for (char c : wrapped)
        if (c == '\n')
            ++lines;
    int text_h = lines * 8;

    float padX = 10.0f, padY = 10.0f;
    float panelX = 14.0f, panelY = 14.0f;
    float panelW = (float)text_w + padX * 2.0f;
    float panelH = (float)text_h + padY * 2.0f;

    hudDrawPanel(window, hudShader, panelX, panelY, panelW, panelH, gHudAlpha);
    float textX = panelX + padX;
    float textY = panelY + padY + 8.0f;
    hudDrawString(window, hudShader, wrapped, textX, textY, gHudAlpha);
}

static GLuint createWhiteTexture1x1()
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    unsigned char px[4] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Interactive Solar System", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // 3D Shaders
    Shader shader("vertex.glsl", "fragment.glsl");
    Shader shadowShader("shadow_vertex.glsl", "shadow_fragment.glsl");
    Shader particleShader("particle_vertex.glsl", "particle_fragment.glsl");
    Shader skyboxShader("skybox_vertex.glsl", "skybox_fragment.glsl");

    // HUD shader
    Shader hudShader("hud_vertex.glsl", "hud_fragment.glsl");
    gHudShader = &hudShader;

    gWhiteTex = createWhiteTexture1x1();

    // Sphere geometry
    unsigned int sphereVAO, sphereVBO;
    int vertexCount = 0;
    generateSphere(sphereVAO, sphereVBO, vertexCount);

    // Solar system
    SolarSystem solarSystem;
    solarSystem.initialize();
    gSolar = &solarSystem;

    // Find Earth index for orbiting the satellite
    int gEarthIdxLocal = solarSystem.findPlanetIndex("Earth");
    gEarthIdx = gEarthIdxLocal;

    // Camera
    centerCameraOnSolarSystem();

    // Skybox & Asteroids
    Skybox skybox("assets/textures/stars.jpg");
    AsteroidBelt asteroidBelt(500, 12.0f, 15.0f);

    // Depth map FBO
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1, 1, 1, 1};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // -------- Load your AcrimSAT model (OBJ) --------
    const char *satPath = "assets/models/acrimsat.obj";
    if (gAcrimSAT.loadFromOBJ(satPath))
    {
        gAcrimSAT.setScale(0.3f); // adjust if huge/tiny
        gAcrimSAT.setRotationEuler(glm::vec3(0.0f, 0.0f, 0.0f));
        std::cout << "Loaded OBJ model: " << satPath << "\n";
    }
    else
    {
        std::cout << "Could not load: " << satPath << " — continuing without it.\n";
        gShowSat = false;
    }

    std::cout << "Controls:\n"
              << "  WASD / Space / Ctrl : Move camera\n"
              << "  Mouse               : Look, Left-click: select planet\n"
              << "  Shift / Alt         : Camera faster / slower\n"
              << "  [ / ]               : Slow / Fast simulation\n"
              << "  P                   : Pause / Resume\n"
              << "  Q / E               : Prev / Next planet\n"
              << "  F                   : Focus camera on selected\n"
              << "  T                   : Toggle camera tracking\n"
              << "  M                   : Toggle satellite visibility\n"
              << "  Tab                 : Toggle mouse capture\n"
              << "  R                   : Reset camera\n";

    double titleTimer = 0.0;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, solarSystem);
        camera.UpdateTracking(deltaTime);

        // 1) Shadow pass (spheres only for simplicity)
        glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 1.0f, 50.0f);
        glm::mat4 lightView = glm::lookAt(glm::vec3(10.0f, 20.0f, 10.0f), glm::vec3(0.0f), glm::vec3(0, 1, 0));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        shadowShader.use();
        shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        solarSystem.render(shadowShader, sphereVAO, vertexCount, camera.Position);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2) Scene pass
        int fbw, fbh;
        glfwGetFramebufferSize(window, &fbw, &fbh);
        camera.setAspectRatio((float)fbw / (float)fbh);
        glViewport(0, 0, fbw, fbh);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        glm::mat4 projection = camera.getProjectionMatrix();
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        shader.setInt("shadowMap", 1);

        glm::vec3 lightPos = glm::vec3(10.0f, 20.0f, 10.0f);
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("viewPos", camera.Position);
        shader.setVec3("atmosphereColor", glm::vec3(0.4f, 0.6f, 1.0f));
        shader.setFloat("atmosphereIntensity", 0.5f);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);

        glDepthFunc(GL_LEQUAL);
        skybox.render(skyboxShader, sphereVAO, vertexCount, camera.Position); // <-- no copy
        glDepthFunc(GL_LESS);

        solarSystem.update(deltaTime);
        solarSystem.render(shader, sphereVAO, vertexCount, camera.Position);

        // ===== Update satellite orbit around Earth =====
        if (gShowSat && gAcrimSAT.isReady() && gEarthIdx >= 0)
        {
            gSatAngle += gSatAngularSpeed * deltaTime;

            glm::vec3 earthPos = solarSystem.planetPosition(gEarthIdx);
            glm::vec3 orbitX = glm::vec3(1, 0, 0);
            glm::vec3 orbitY = glm::normalize(glm::vec3(0, cos(gSatInclination), sin(gSatInclination)));

            glm::vec3 offset = gSatOrbitRadius * (cos(gSatAngle) * orbitX + sin(gSatAngle) * orbitY);
            glm::vec3 satPos = earthPos + offset;

            gAcrimSAT.setPosition(satPos);
            gAcrimSAT.setRotationEuler(glm::vec3(0.0f, gSatAngle, 0.0f));

            // Ensure a valid texture is bound for the material sampler
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gWhiteTex);
            shader.setInt("texture1", 0);

            glm::mat4 model = gAcrimSAT.modelMatrix();
            shader.setMat4("model", model);
            gAcrimSAT.draw();
        }

        // Asteroids
        particleShader.use();
        particleShader.setMat4("projection", projection);
        particleShader.setMat4("view", view);
        particleShader.setMat4("model", glm::mat4(1.0f));
        asteroidBelt.render(particleShader);

        // HUD overlay
        glDisable(GL_DEPTH_TEST);
        hudRender(window, hudShader, solarSystem, deltaTime);
        glEnable(GL_DEPTH_TEST);

        titleTimer += deltaTime;
        if (titleTimer > 0.2)
        {
            updateWindowTitle(window, solarSystem);
            titleTimer = 0.0;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    if (gWhiteTex)
        glDeleteTextures(1, &gWhiteTex);

    if (gHudVAO)
        glDeleteVertexArrays(1, &gHudVAO);
    if (gHudVBO)
        glDeleteBuffers(1, &gHudVBO);
    if (gHudEBO)
        glDeleteBuffers(1, &gHudEBO);
    if (gPanelVAO)
        glDeleteVertexArrays(1, &gPanelVAO);
    if (gPanelVBO)
        glDeleteBuffers(1, &gPanelVBO);
    if (gPanelEBO)
        glDeleteBuffers(1, &gPanelEBO);

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) { glViewport(0, 0, width, height); }

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (!cursorCaptured)
        return;
    if (firstMouse)
    {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }
    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;
    lastX = (float)xpos;
    lastY = (float)ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int)
{
    if (!gSolar)
        return;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        auto [origin, dir] = screenPosToWorldRay(mx, my, window);

        float tHit = std::numeric_limits<float>::infinity();
        int idx = gSolar->pickPlanet(origin, dir, tHit);
        if (idx >= 0)
        {
            gSolar->setSelected(idx);
            gHudShowTimer = 0.0;
            gHudAlpha = 0.0f;
            std::string fact = gSolar->selectedFact();
            std::cout << "[Selected] " << gSolar->selectedName() << " — " << fact << std::endl;
            updateWindowTitle(window, *gSolar, "Fact: " + fact);
        }
    }
}

void scroll_callback(GLFWwindow *window, double, double yoffset) { camera.ProcessMouseScroll((float)yoffset); }

void processInput(GLFWwindow *window, SolarSystem &solar)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    static bool tabPressed = false;
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
    {
        if (!tabPressed)
        {
            cursorCaptured = !cursorCaptured;
            glfwSetInputMode(window, GLFW_CURSOR, cursorCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
            firstMouse = true;
            tabPressed = true;
        }
    }
    else
        tabPressed = false;

    float cameraSpeed = camera.MovementSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        cameraSpeed *= 2.0f;
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
        cameraSpeed *= 0.3f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime * cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime * cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime * cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime * cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime * cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime * cameraSpeed);

    static bool rPressed = false;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        if (!rPressed)
        {
            camera.ResetPosition(glm::vec3(0.0f, 5.0f, 20.0f));
            rPressed = true;
        }
    }
    else
        rPressed = false;

    static bool tPressed = false;
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    {
        if (!tPressed)
        {
            camera.SetTrackingMode(!camera.trackingMode, camera.targetPosition);
            tPressed = true;
        }
    }
    else
        tPressed = false;

    static bool lbPressed = false, rbPressed = false;
    if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
    {
        if (!lbPressed)
        {
            solar.decreaseTimeScale();
            lbPressed = true;
        }
    }
    else
        lbPressed = false;
    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
    {
        if (!rbPressed)
        {
            solar.increaseTimeScale();
            rbPressed = true;
        }
    }
    else
        rbPressed = false;

    static bool pPressed = false;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        if (!pPressed)
        {
            solar.setPaused(!solar.isPaused());
            pPressed = true;
        }
    }
    else
        pPressed = false;

    static bool qPressed = false, ePressed = false;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        if (!qPressed)
        {
            solar.cycleSelection(-1);
            gHudShowTimer = 0.0;
            gHudAlpha = 0.0f;
            qPressed = true;
        }
    }
    else
        qPressed = false;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        if (!ePressed)
        {
            solar.cycleSelection(+1);
            gHudShowTimer = 0.0;
            gHudAlpha = 0.0f;
            ePressed = true;
        }
    }
    else
        ePressed = false;

    static bool fPressed = false;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
        if (!fPressed)
        {
            glm::vec3 target = solar.selectedPosition();
            float r = solar.selectedRadius();
            if (target != glm::vec3(0.0f) || solar.selectedIndex() >= 0)
            {
                camera.trackingDistance = std::max(4.0f, r * 6.0f);
                camera.SetTrackingMode(true, target);
            }
            gHudShowTimer = 0.0;
            gHudAlpha = 0.0f;
            fPressed = true;
        }
    }
    else
        fPressed = false;

    // Toggle satellite visibility
    static bool mPressed = false;
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
    {
        if (!mPressed)
        {
            gShowSat = !gShowSat;
            mPressed = true;
        }
    }
    else
        mPressed = false;
}

void generateSphere(unsigned int &VAO, unsigned int &VBO, int &vertexCount, int sectorCount, int stackCount)
{
    std::vector<float> vertices;
    for (int i = 0; i <= stackCount; ++i)
    {
        float stackAngle = glm::pi<float>() / 2 - i * glm::pi<float>() / stackCount;
        float xy = cos(stackAngle);
        float z = sin(stackAngle);

        for (int j = 0; j <= sectorCount; ++j)
        {
            float sectorAngle = j * 2 * glm::pi<float>() / sectorCount;
            float x = xy * cos(sectorAngle);
            float y = xy * sin(sectorAngle);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back((float)j / sectorCount);
            vertices.push_back((float)i / stackCount);
        }
    }

    std::vector<unsigned int> indices;
    for (int i = 0; i < stackCount; ++i)
    {
        int k1 = i * (sectorCount + 1);
        int k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            indices.push_back(k1);
            indices.push_back(k2);
            indices.push_back(k1 + 1);
            indices.push_back(k1 + 1);
            indices.push_back(k2);
            indices.push_back(k2 + 1);
        }
    }

    std::vector<float> expanded;
    expanded.reserve(indices.size() * 8);
    for (unsigned int idx : indices)
        for (int i = 0; i < 8; ++i)
            expanded.push_back(vertices[idx * 8 + i]);

    vertexCount = (int)expanded.size() / 8;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, expanded.size() * sizeof(float), expanded.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
}
