//#include <windows.h>
//
//#include "imgui.h"
//#include "imgui_impl_glfw.h"
//#include "imgui_impl_opengl3.h"
//
//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
//
//#include <GL/glext.h>
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/transform.hpp>
//#include <glm/gtc/type_ptr.hpp>
//#include <iostream>
//
//#include "InitShader.h"    //Functions for loading shaders from text files
////#include "LoadMesh.h"      //Functions for creating OpenGL buffers from mesh files
////#include "LoadTexture.h"   //Functions for creating OpenGL textures from image files
//
//static const std::string vertex_shader("triangle_vs.glsl");
//static const std::string fragment_shader("triangle_fs.glsl");
//GLuint shader_program = -1;
//
//static const std::string mesh_name = "Amago0.obj";
//static const std::string texture_name = "AmagoT.bmp";
//
//GLuint texture_id = -1; //Texture map for mesh
////MeshData mesh_data;
//
//float angle = 0.0f;
//float scale = 1.0f;
//
//bool clearScreen = false;
//bool enableDepthTesting = false;
//
//
////For an explanation of this program's structure see https://www.glfw.org/docs/3.3/quick.html 
//
//void framebuffer_size_callback(GLFWwindow* window, int width, int height)
//{
//    // make sure the viewport matches the new window dimensions; note that width and 
//    // height will be significantly larger than specified on retina displays.
//    glViewport(0, 0, width, height);
//}
//void draw_gui(GLFWwindow* window)
//{
//   //Begin ImGui Frame
//   ImGui_ImplOpenGL3_NewFrame();
//   ImGui_ImplGlfw_NewFrame();
//   ImGui::NewFrame();
//
//   //Draw Gui
//   ImGui::Begin("Debug window");                       
//      if (ImGui::Button("Quit"))                          
//      {
//         glfwSetWindowShouldClose(window, GLFW_TRUE);
//      }    
//      //Lab 2: Uncomment these 
//      ImGui::SliderFloat("View angle", &angle, -180.0f, +180.0f);
//      ImGui::SliderFloat("Scale", &scale, -10.0f, +10.0f);
//      ImGui::Checkbox("Clear screen", &clearScreen);
//      if (clearScreen) {
//          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//      }
//      ImGui::Checkbox("Enable depth testing", &enableDepthTesting);
//      if (enableDepthTesting) {
//          glEnable(GL_DEPTH_TEST);
//      }
//      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
//   ImGui::End();
//
//   //End ImGui Frame
//   ImGui::Render();
//   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
//}
//
//// This function gets called every time the scene gets redisplayed
////void display(GLFWwindow* window)
////{
////   //Clear the screen to the color previously specified in the glClearColor(...) call.
////   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
////   
////   glm::mat4 M = glm::rotate(angle, glm::vec3(0.0f, 1.0f, 0.0f))*glm::scale(glm::vec3(scale*mesh_data.mScaleFactor));
////   glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
////   glm::mat4 P = glm::perspective(40.0f, 1.0f, 0.1f, 100.0f);
////
////   glUseProgram(shader_program);
////
////   glActiveTexture(GL_TEXTURE0);
////   glBindTexture(GL_TEXTURE_2D, texture_id);
////   int tex_loc = glGetUniformLocation(shader_program, "diffuse_tex");
////   if (tex_loc != -1)
////   {
////      glUniform1i(tex_loc, 0); // we bound our texture to texture unit 0
////   }
////
////   //Get location for shader uniform variable
////   int PVM_loc = glGetUniformLocation(shader_program, "PVM");
////   if(PVM_loc != -1)
////   {
////      glm::mat4 PVM = P*V*M;
////      //Set the value of the variable at a specific location
////      glUniformMatrix4fv(PVM_loc, 1, false, glm::value_ptr(PVM));
////   }
////
////   glBindVertexArray(mesh_data.mVao);
////   glDrawElements(GL_TRIANGLES, mesh_data.mSubmesh[0].mNumIndices, GL_UNSIGNED_INT, 0);
////   //For meshes with multiple submeshes use mesh_data.DrawMesh(); 
////
////   draw_gui(window);
////
////   /* Swap front and back buffers */
////   glfwSwapBuffers(window);
////
////}
//
//
//
//
//void idle()
//{
//   float time_sec = static_cast<float>(glfwGetTime());
//
//   //Pass time_sec value to the shaders
//   int time_loc = glGetUniformLocation(shader_program, "time");
//   if (time_loc != -1)
//   {
//      glUniform1f(time_loc, time_sec);
//   }}
//
//void reload_shader()
//{
//   GLuint new_shader = InitShader(vertex_shader.c_str(), fragment_shader.c_str());
//
//   if (new_shader == -1) // loading failed
//   {
//      glClearColor(1.0f, 0.0f, 1.0f, 0.0f); //change clear color if shader can't be compiled
//   }
//   else
//   {
//      glClearColor(0.35f, 0.35f, 0.35f, 0.0f);
//
//      if (shader_program != -1)
//      {
//         glDeleteProgram(shader_program);
//      }
//      shader_program = new_shader;
//   }
//}
//
////This function gets called when a key is pressed
//void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
//{
//   std::cout << "key : " << key << ", " << char(key) << ", scancode: " << scancode << ", action: " << action << ", mods: " << mods << std::endl;
//
//   if(action == GLFW_PRESS)
//   {
//      switch(key)
//      {
//         case 'r':
//         case 'R':
//            reload_shader();     
//         break;
//
//         case GLFW_KEY_ESCAPE:
//            glfwSetWindowShouldClose(window, GLFW_TRUE);
//         break;     
//      }
//   }
//}
//
////This function gets called when the mouse moves over the window.
//void mouse_cursor(GLFWwindow* window, double x, double y)
//{
//    //std::cout << "cursor pos: " << x << ", " << y << std::endl;
//}
//
////This function gets called when a mouse button is pressed.
//void mouse_button(GLFWwindow* window, int button, int action, int mods)
//{
//    //std::cout << "button : "<< button << ", action: " << action << ", mods: " << mods << std::endl;
//}
//
////Initialize OpenGL state. This function only gets called once.
//void initOpenGL()
//{
//   glewInit();
//
//   //Print out information about the OpenGL version supported by the graphics driver.	
//   std::cout << "Vendor: "       << glGetString(GL_VENDOR)                    << std::endl;
//   std::cout << "Renderer: "     << glGetString(GL_RENDERER)                  << std::endl;
//   std::cout << "Version: "      << glGetString(GL_VERSION)                   << std::endl;
//   std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION)  << std::endl;
//   glEnable(GL_DEPTH_TEST);
//
//   // Try uncommenting one of these lines at a time, and rerunning the program
//   //glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
//   //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//
//   reload_shader();
//   //mesh_data = LoadMesh(mesh_name);
//   //texture_id = LoadTexture(texture_name);
//}
//
//
//
////C++ programs start executing in the main() function.
//int main(void)
//{
//   GLFWwindow* window;
//
//   /* Initialize the library */
//   if (!glfwInit())
//   {
//      return -1;
//   }
//
//   /* Create a windowed mode window and its OpenGL context */
//   window = glfwCreateWindow(1024, 1024, "Lab 2", NULL, NULL);
//   if (!window)
//   {
//      glfwTerminate();
//      return -1;
//   }
//
//   //Register callback functions with glfw. 
//   glfwSetKeyCallback(window, keyboard);
//   glfwSetCursorPosCallback(window, mouse_cursor);
//   glfwSetMouseButtonCallback(window, mouse_button);
//
//   /* Make the window's context current */
//   glfwMakeContextCurrent(window);
//   glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
//
//   initOpenGL();
//   
//   //Init ImGui
//   IMGUI_CHECKVERSION();
//   ImGui::CreateContext();
//   ImGui_ImplGlfw_InitForOpenGL(window, true);
//   ImGui_ImplOpenGL3_Init("#version 150");
//   float vertices[] = {
//    -0.5f, -0.5f, 0.0f, // left  
//     0.5f, -0.5f, 0.0f, // right 
//     0.0f,  0.5f, 0.0f  // top   
//   };
//
//   unsigned int VBO, VAO;
//   glGenVertexArrays(1, &VAO);
//   glGenBuffers(1, &VBO);
//   // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
//   glBindVertexArray(VAO);
//
//   glBindBuffer(GL_ARRAY_BUFFER, VBO);
//   glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//
//   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
//   glEnableVertexAttribArray(0);
//
//   // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
//   glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//   // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
//   // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
//   glBindVertexArray(0);
//
//
//   // uncomment this call to draw in wireframe polygons.
//   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//
//   /* Loop until the user closes the window */
//   while (!glfwWindowShouldClose(window))
//   {
//      idle();
//      //display(window);
//
//       glClearColor(1.0f, 0.3f, 0.3f, 1.0f);
//   glClear(GL_COLOR_BUFFER_BIT);
//      glUseProgram(shader_program);
//      glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
//      glDrawArrays(GL_TRIANGLES, 0, 3);
//       glBindVertexArray(0); // no need to unbind it every time 
//
//      // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
//      // -------------------------------------------------------------------------------
//      glfwSwapBuffers(window);
//      /* Poll for and process events */
//      glfwPollEvents();
//   }
//
//    // Cleanup ImGui
//    ImGui_ImplOpenGL3_Shutdown();
//    ImGui_ImplGlfw_Shutdown();
//    ImGui::DestroyContext();
//
//   glfwTerminate();
//   return 0;
//}