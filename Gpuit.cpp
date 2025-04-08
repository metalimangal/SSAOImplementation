#include <windows.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <GL/glext.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <random>
#include <vector>
#include <string>

#include "InitShader.h"    //Functions for loading shaders from text files
#include "LoadMesh.h"      //Functions for creating OpenGL buffers from mesh files
#include "LoadTexture.h"   //Functions for creating OpenGL textures from image files




// I will be need 4 passes - geometry, ssao, lighting and blurring passes. Hence I will define them here

static const std::string vertex_shader1("geometry_vs.glsl");
static const std::string fragment_shader1("geometry_fs.glsl");
static const std::string vertex_shader2("lighting_vs.glsl");
static const std::string fragment_shader2("ssao_pass_fs.glsl");
static const std::string vertex_shader3("lighting_vs.glsl");
static const std::string fragment_shader3("lighting_fs.glsl");
static const std::string vertex_shader4("lighting_vs.glsl");
static const std::string fragment_shader4("blurring_fs.glsl");

void generateSampleKernel();


unsigned int gPosition, gNormal, gAlbedoSpec;
unsigned int gBuffer;
unsigned int ssaoFBO, ssaoBlurFBO;
std::vector<glm::vec3> ssaoKernel;
int N = 64;
float D = 1.0f;
float f_dash;
float m;

bool blur = true;

std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
std::default_random_engine generator;

unsigned int ssaoColorBuffer, ssaoColorBufferBlur;
unsigned int noiseTexture;


GLuint geometry_shader = -1;
GLuint ssao_pass_shader = -1;
GLuint lighting_shader = -1;
GLuint blurring_shader = -1;

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;




class SceneObject {
public:
    glm::vec3 position;
    glm::vec3 rotation; // Rotation in degrees for x, y, z axes
    glm::vec3 scale;

    SceneObject(const glm::vec3& pos = glm::vec3(0.0f),
        const glm::vec3& rot = glm::vec3(0.0f),
        const glm::vec3& scl = glm::vec3(1.0f))
        : position(pos), rotation(rot), scale(scl) {}

    glm::mat4 getModelMatrix() const {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale);
        return model;
    }
};

std::vector<SceneObject> sceneObjects;

std::vector<SceneObject> initialObjects = {
    SceneObject(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f)),
    SceneObject(glm::vec3(1.5f, 0.0f, -2.0f), glm::vec3(0.0f, 45.0f, 0.0f), glm::vec3(0.5f)),
    SceneObject(glm::vec3(-0.5f, 0.0f, 1.5f), glm::vec3(0.0f, 90.0f, 0.0f), glm::vec3(0.75f))
};

glm::vec3 newObjectPosition = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 lightPos = glm::vec3(2.0, 4.0, -2.0);
glm::vec3 lightColor = glm::vec3(0.2, 0.2, 0.7);


//static const std::string mesh_name = "Amago0.obj";
//static const std::string mesh_name = "OptimusPrime.obj";
static const std::string mesh_name = "backpack.obj";
static const std::string texture_name = "AmagoT.bmp";

GLuint texture_id = -1; //Texture map for mesh
MeshData mesh_data;

float angle = 0.0f;
float scale = 1.0f;
float k_factor = 1.0f, power = 1.0f;

bool clearScreen = false;
bool enableDepthTesting = false;
bool onlyAmbientOcclusion = false;

//For an explanation of this program's structure see https://www.glfw.org/docs/3.3/quick.html 

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}


void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}



void draw_gui(GLFWwindow* window)
{
   //Begin ImGui Frame
   ImGui_ImplOpenGL3_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();

   //Draw Gui
   ImGui::Begin("Debug window");                       
      if (ImGui::Button("Quit"))                          
      {
         glfwSetWindowShouldClose(window, GLFW_TRUE);
      }    
      //Lab 2: Uncomment these 
      ImGui::SliderFloat("View angle", &angle, -180.0f, +180.0f);
      ImGui::SliderFloat("Scale", &scale, -10.0f, +10.0f);
      ImGui::SliderFloat("k_factor", &k_factor, 0.1f, +10.0f);
      ImGui::SliderFloat("Power", &power, 0.1f, +10.0f);
      ImGui::Checkbox("Clear screen", &clearScreen);
      if (clearScreen) {
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      }

      // Section to add new object positions
      ImGui::Separator();
      ImGui::Text("Add New Object");
      if (ImGui::Button("Add Object"))
      {
          sceneObjects.push_back(SceneObject());
      }

      // Display and edit existing object positions
      ImGui::Separator();
      ImGui::Text("Edit Object Transformations");

      for (size_t i = 0; i < sceneObjects.size(); ++i)
      {
          ImGui::PushID(static_cast<int>(i)); // Ensure unique ID for each slider group
          ImGui::SliderFloat3(("Position " + std::to_string(i)).c_str(), glm::value_ptr(sceneObjects[i].position), -10.0f, 10.0f);
          ImGui::SliderFloat3(("Rotation " + std::to_string(i)).c_str(), glm::value_ptr(sceneObjects[i].rotation), -180.0f, 180.0f);
          ImGui::SliderFloat3(("Scale " + std::to_string(i)).c_str(), glm::value_ptr(sceneObjects[i].scale), 0.1f, 10.0f);

          if (ImGui::Button("Remove"))
          {
              sceneObjects.erase(sceneObjects.begin() + i);
          }
          ImGui::PopID();
      }

      ImGui::Separator();

      //Add Light Parameters
      ImGui::SliderFloat3("Light Position", glm::value_ptr(lightPos), -10.0f, 10.0f);
      ImGui::SliderFloat3("Light Color", glm::value_ptr(lightColor), 0.0f, 1.0f);


      // Add controls for N (sampleCount) and D (sampleRadius)
      ImGui::Separator();
      ImGui::Text("SSAO Parameters");

      // Slider for N: Number of samples
      ImGui::SliderInt("Number of Samples (N)", &N, 1, 128);
      if (ImGui::IsItemDeactivatedAfterEdit()) {
          generateSampleKernel(); // Regenerate samples when N changes
      }

      // Slider for D: Distance range (radius)
      ImGui::SliderFloat("Sample Radius (D)", &D, 0.1f, 5.0f);
      if (ImGui::IsItemDeactivatedAfterEdit()) {
          generateSampleKernel(); // Regenerate samples when D changes
      }


      ImGui::Checkbox("Enable depth testing", &enableDepthTesting);
      if (enableDepthTesting) {
          glEnable(GL_DEPTH_TEST);
      }

      ImGui::Checkbox("Only Ambient Occlusion", &onlyAmbientOcclusion);
      ImGui::Checkbox("Blur", &blur);
      
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
   ImGui::End();

   //End ImGui Frame
   ImGui::Render();
   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


//G-buffer setting
void setGBuffer() {

    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1024, 1024, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    // normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1024, 1024, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    // color + specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1024, 1024);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    else
        std::cout << "Framebuffer complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void setSSAO() {

    glGenFramebuffers(1, &ssaoFBO);  
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1024, 1024, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Framebuffer not complete!" << std::endl;


    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    glGenFramebuffers(1, &ssaoBlurFBO);//When blur will be used
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // SSAO color buffer

    
    // and blur stage
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glGenTextures(1, &ssaoColorBufferBlur);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1024, 1024, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

float lerpFunction(float a, float b, float f)
{
    return a + f * (b - a);
}

void generateSampleKernel() {

    ssaoKernel.clear();
    for (unsigned int i = 0; i < N; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / float(N);

        // scale samples s.t. they're more aligned to center of kernel
        scale = lerpFunction(0.1f, 1.0f, scale * scale);
        sample *= scale*D;
        ssaoKernel.push_back(sample);
    }
}

void generateNoiseTexture() {
    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++)
    {
        glm::vec3 noise(
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator) * 2.0 - 1.0,
            0.0f);
        ssaoNoise.push_back(noise);
    }


    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void setBasicShaderConfig() {
    glUseProgram(lighting_shader);
    glUniform1i(glGetUniformLocation(lighting_shader, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(lighting_shader, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(lighting_shader, "gAlbedo"), 2);
    glUniform1i(glGetUniformLocation(lighting_shader, "ssao"), 3);

    glUseProgram(ssao_pass_shader);
    glUniform1i(glGetUniformLocation(ssao_pass_shader, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(ssao_pass_shader, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(ssao_pass_shader, "texNoise"), 2);

    glUseProgram(blurring_shader);
    glUniform1i(glGetUniformLocation(blurring_shader, "ssaoInput"), 0);

}






// This function gets called every time the scene gets redisplayed
void display(GLFWwindow* window)
{

    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = glm::perspective(40.0f, 1.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 model = glm::mat4(1.0f);

    glUseProgram(geometry_shader); // Use the geometry shader program
    glUniformMatrix4fv(glGetUniformLocation(geometry_shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(geometry_shader, "view"), 1, GL_FALSE, glm::value_ptr(view));

    model = glm::translate(model, glm::vec3(0.0, 7.0f, 0.0f));
    model = glm::scale(model, glm::vec3(7.5f, 7.5f, 7.5f));
    glUniformMatrix4fv(glGetUniformLocation(geometry_shader, "model"), 1, GL_FALSE, glm::value_ptr(model));


    glUniform1i(glGetUniformLocation(geometry_shader, "invertedNormals"), 1);
    renderCube();
    glUniform1i(glGetUniformLocation(geometry_shader, "invertedNormals"), 0);
    // Render the models on the floor
    for (const auto& obj : sceneObjects)
    {
        glm::mat4 model = obj.getModelMatrix();
        model = model * glm::scale(glm::mat4(1.0f), glm::vec3(mesh_data.mScaleFactor));

        glUniformMatrix4fv(glGetUniformLocation(geometry_shader, "model"), 1, GL_FALSE, glm::value_ptr(model));

        // Bind the mesh VAO and draw it
        glBindVertexArray(mesh_data.mVao);
        glDrawElements(GL_TRIANGLES, mesh_data.mSubmesh[0].mNumIndices, GL_UNSIGNED_INT, 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 2. Generate SSAO texture
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(ssao_pass_shader);

    ////////// Send kernel samples to the shader
    if (ssaoKernel.size() >= N) {
        for (unsigned int i = 0; i < N; ++i) {
            std::string sampleName = "samples[" + std::to_string(i) + "]";
            glUniform3fv(glGetUniformLocation(ssao_pass_shader, sampleName.c_str()), 1, glm::value_ptr(ssaoKernel[i]));
        }
    }
    
    glUniformMatrix4fv(glGetUniformLocation(ssao_pass_shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1i(glGetUniformLocation(ssao_pass_shader, "kernelSize"), N);
    glUniform1f(glGetUniformLocation(ssao_pass_shader, "radius"), D);
    glUniform1f(glGetUniformLocation(ssao_pass_shader, "k_factor"), k_factor);
    glUniform1f(glGetUniformLocation(ssao_pass_shader, "power"), power);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glUniform1i(glGetUniformLocation(ssao_pass_shader, "gPosition"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glUniform1i(glGetUniformLocation(ssao_pass_shader, "gNormal"), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glUniform1i(glGetUniformLocation(ssao_pass_shader, "texNoise"), 2);

    renderQuad(); // Render a screen-filling quad to generate SSAO

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ////// 3. Blur SSAO texture to remove noise


    if (blur) {
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(blurring_shader);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
        glUniform1i(glGetUniformLocation(blurring_shader, "ssaoInput"), 0);

        renderQuad(); // Render a screen-filling quad to blur the SSAO texture

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    }

    
    //////// 4. Lighting pass: Combine geometry pass with SSAO and apply lighting
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(lighting_shader);

    glm::vec3 lightPosView = glm::vec3(view * glm::vec4(lightPos, 1.0f));
    glUniform3fv(glGetUniformLocation(lighting_shader, "light.Position"), 1, glm::value_ptr(lightPosView));
    glUniform3fv(glGetUniformLocation(lighting_shader, "light.Color"), 1, glm::value_ptr(lightColor));

    //// Set light attenuation parameters
    const float linear = 0.09f;
    const float quadratic = 0.032f;
    glUniform1f(glGetUniformLocation(lighting_shader, "light.Linear"), linear);
    glUniform1f(glGetUniformLocation(lighting_shader, "light.Quadratic"), quadratic);

    glUniform1i(glGetUniformLocation(lighting_shader, "onlyAO"), onlyAmbientOcclusion?1:0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glActiveTexture(GL_TEXTURE3);
    if (blur) {
        glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    }
    else {
        glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    }

    renderQuad(); // Render the final result on the screen




    draw_gui(window);
    glfwSwapBuffers(window);
}


void reload_shader()
{
    // Reload each shader program individually
    GLuint new_geometry_shader = InitShader("geometry_vs.glsl", "geometry_fs.glsl");
    GLuint new_ssao_pass_shader = InitShader("ssao_pass_vs.glsl", "ssao_pass_fs.glsl");
    GLuint new_lighting_shader = InitShader("lighting_vs.glsl", "lighting_fs.glsl");
    GLuint new_blurring_shader = InitShader("blurring_vs.glsl", "blurring_fs.glsl");

    // Check if any shader failed to load
    if (new_geometry_shader == -1 || new_ssao_pass_shader == -1 ||
        new_lighting_shader == -1 || new_blurring_shader == -1)
    {
        glClearColor(1.0f, 0.0f, 1.0f, 0.0f); // Indicate error with magenta color
    }
    else
    {
        glClearColor(0.35f, 0.35f, 0.35f, 0.35f);

        // Delete the old shader programs if they exist
        if (geometry_shader != -1)
        {
            glDeleteProgram(geometry_shader);
        }
        if (ssao_pass_shader != -1)
        {
            glDeleteProgram(ssao_pass_shader);
        }
        if (lighting_shader != -1)
        {
            glDeleteProgram(lighting_shader);
        }
        if (blurring_shader != -1)
        {
            glDeleteProgram(blurring_shader);
        }

        // Assign the new shader programs to the global variables
        geometry_shader = new_geometry_shader;
        ssao_pass_shader = new_ssao_pass_shader;
        lighting_shader = new_lighting_shader;
        blurring_shader = new_blurring_shader;
    }
}


//This function gets called when a key is pressed
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   std::cout << "key : " << key << ", " << char(key) << ", scancode: " << scancode << ", action: " << action << ", mods: " << mods << std::endl;

   if(action == GLFW_PRESS)
   {
      switch(key)
      {
         case 'r':
         case 'R':
            reload_shader();     
         break;

         case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
         break;     
      }
   }
}

//This function gets called when the mouse moves over the window.
void mouse_cursor(GLFWwindow* window, double x, double y)
{
    //std::cout << "cursor pos: " << x << ", " << y << std::endl;
}

//This function gets called when a mouse button is pressed.
void mouse_button(GLFWwindow* window, int button, int action, int mods)
{
    //std::cout << "button : "<< button << ", action: " << action << ", mods: " << mods << std::endl;
}

//Initialize OpenGL state. This function only gets called once.
void initOpenGL()
{
   glewInit();

   //Print out information about the OpenGL version supported by the graphics driver.	
   std::cout << "Vendor: "       << glGetString(GL_VENDOR)                    << std::endl;
   std::cout << "Renderer: "     << glGetString(GL_RENDERER)                  << std::endl;
   std::cout << "Version: "      << glGetString(GL_VERSION)                   << std::endl;
   std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION)  << std::endl;
   glEnable(GL_DEPTH_TEST);

   // Try uncommenting one of these lines at a time, and rerunning the program
   //glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
   //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

   reload_shader();
   mesh_data = LoadMesh(mesh_name);
   texture_id = LoadTexture(texture_name);
}



//C++ programs start executing in the main() function.
int main(void)
{
   GLFWwindow* window;

   /* Initialize the library */
   if (!glfwInit())
   {
      return -1;
   }

   /* Create a windowed mode window and its OpenGL context */
   window = glfwCreateWindow(1024, 1024, "SSAO Implementation - GPU It!!!", NULL, NULL);
   if (!window)
   {
      glfwTerminate();
      return -1;
   }

   //Register callback functions with glfw. 
   glfwSetKeyCallback(window, keyboard);
   glfwSetCursorPosCallback(window, mouse_cursor);
   glfwSetMouseButtonCallback(window, mouse_button);

   /* Make the window's context current */
   glfwMakeContextCurrent(window);

   initOpenGL();
   
   //Init ImGui
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGui_ImplGlfw_InitForOpenGL(window, true);
   ImGui_ImplOpenGL3_Init("#version 150");


   /*Initializing everything here*/

   setGBuffer();
   setSSAO();
   generateSampleKernel();
   generateNoiseTexture();
   setBasicShaderConfig();

   sceneObjects.insert(sceneObjects.end(), initialObjects.begin(), initialObjects.end()); //Populating the scene with some initial objects, I cna add more objects via ImGui

   /* Loop until the user closes the window */
   while (!glfwWindowShouldClose(window))
   {

      display(window);

      /* Poll for and process events */
      glfwPollEvents();
   }

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

   glfwTerminate();
   return 0;
}