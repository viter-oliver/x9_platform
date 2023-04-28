#include "application.h"

#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
//#include <GL/gl3w.h>
#include <chrono>
#include <thread>
#include <GLFW/glfw3.h>
#include "SOIL.h"
//#include "texture.h"
#include "res_output.h"
#include <functional>

//#include "Resource.h"
#include "afb_load.h"
#include "af_state_manager.h"
#include "af_primitive_object.h"
#include <fstream>
using namespace std;
using namespace chrono;
//extern void instantiating_internal_shader();
static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error %d: %s\n", error, description);
}
void APIENTRY glDebugOutput(GLenum source, 
                            GLenum type, 
                            GLuint id, 
                            GLenum severity, 
                            size_t length, 
                            const char *message, 
                            const void *userParam)
{
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; // ignore these non-significant error codes
 
    printf("---------------\n");
    printf("Debug message (%d:%s)",id,message);
 
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             printf("Source: API"); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   printf("Source: Window System"); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: printf("Source: Shader Compiler"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     printf("Source: Third Party"); break;
        case GL_DEBUG_SOURCE_APPLICATION:     printf("Source: Application"); break;
        case GL_DEBUG_SOURCE_OTHER:           printf("Source: Other"); break;
    } printf("\n");
 
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               printf("Type: Error\n"); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: printf("Type: Deprecated Behaviour\n"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  printf("Type: Undefined Behaviour\n"); break; 
        case GL_DEBUG_TYPE_PORTABILITY:         printf("Type: Portability\n"); break;
        case GL_DEBUG_TYPE_PERFORMANCE:         printf("Type: Performance\n"); break;
        case GL_DEBUG_TYPE_MARKER:              printf("Type: Marker\n"); break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          printf("Type: Push Group\n"); break;
        case GL_DEBUG_TYPE_POP_GROUP:           printf("Type: Pop Group\n"); break;
        case GL_DEBUG_TYPE_OTHER:               printf("Type: Other\n"); break;
    };
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         printf("Severity: high\n"); break;
        case GL_DEBUG_SEVERITY_MEDIUM:       printf("Severity: medium\n"); break;
        case GL_DEBUG_SEVERITY_LOW:          printf("Severity: low\n"); break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: printf("Severity: notification\n"); break;
    };
}
namespace auto_future
{
	application::application(int argc, char **argv)
	{
		if (argc > 1)
		{
			_cureent_project_file_path = argv[1];//afb
			printf("argv1=%s\n",argv[1]);
			if(argc>2){
				ifstream if_tst;
				if_tst.open(argv[2],ios::binary);
				if(if_tst.is_open()){
					printf("%s is valid\n",argv[2]);
					if_tst.close();
				}
			}
			
		}
		for (int ix = 2; ix < argc;ix++)
		{
			_arg_list.emplace_back(argv[ix]);
		}
	}


	application::~application()
	{
	}

	int get_resolution(int* width,int* height)
	{
		struct fb_var_screeninfo sc_info;
		int fd=open("/dev/fb0",O_RDWR);
		if(!fd)
			return 0;
		ioctl(fd,FBIOGET_VSCREENINFO,&sc_info);
		printf("screen: %d*%d\n",sc_info.xres,sc_info.yres);
		*width=sc_info.xres;
		*height=sc_info.yres;
		close(fd);
		return 0;
	}
	bool application::prepare()
	{
		glfwSetErrorCallback(error_callback);
		if (!glfwInit())
			return false;
    glfwWindowHint(GLFW_CLIENT_API,GLFW_OPENGL_ES_API);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE);
		glfwWindowHint(GLFW_SAMPLES,4);
		//glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); 
#if __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
		//if (!_screen_width || !_screen_height)
		{
			//GLFWmonitor*  pmornitor = glfwGetPrimaryMonitor();
			//const GLFWvidmode * mode = glfwGetVideoMode(pmornitor);
			//_screen_width = mode->width;
			//_screen_height = mode->height;
      		get_resolution(&_screen_width,&_screen_height);
		}
		
		//instantiating_internal_shader();

		//ImVec2 edit_window_size = ImVec2()


		return true;
	}

	bool application::create_run()
	{
		GLFWwindow* _window = { NULL };
        //glfwGetMonitor_by_id(monitor_id)
		_window = glfwCreateWindow(_screen_width, _screen_height, "Graphics app", NULL, NULL);
        glfwSetWindowPos(_window,_screen_posx,0);
		 
		 
		//glEnable(GL_DEBUG_OUTPUT);
        //glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // makes sure errors are displayed synchronously
        //glDebugMessageCallbackKHR(glDebugOutput, nullptr);
        //glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

		//glfwSetWindowPos(_window, -_screen_posx,_screen_posy);
		glfwMakeContextCurrent(_window);

		glfwSwapInterval(1); // Enable vsync

		//gl3wInit();

		// Setup ImGui binding
		ImGui::CreateContext();
		//ImGuiIO& io = ImGui::GetIO(); //(void)io;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    ImGui_ImplGlfwGL3_Init(_window, true);
		//ImGui::StyleColorsClassic();
		ImVec4 clear_color = ImVec4(0.3f, 0.2f, 0.1f, 1.00f);

		//base_ui_component* _proot = NULL;
		if (!_cureent_project_file_path.empty())
		{
			//_proot = new ft_base;
			
			afb_load afl(_proot);
			afl.load_afb(_cureent_project_file_path.c_str());
			resLoaded();
		}
		int max_tex_size=0;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE,&max_tex_size);
		printf("max texture size=%d\n",max_tex_size);
		GLint max_samples=0;
		glGetIntegerv(GL_MAX_SAMPLES,&max_samples);
		printf("samples=%d\n",max_samples);
		//init_internal_primitive_list();
	// Setup style
		//ImGui::StyleColorsLight();
		ImGui::StyleColorsClassic();
		int tar_fps=60;
		int iper_sc=1000.f/tar_fps;
		steady_clock::time_point  t_bf_render,t_af_render;
#if 0
		const char *vertexShaderSource =R"glsl(#version 320 es
precision mediump float;
layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aColor;
out vec3 ourColor;
void main(){
	gl_Position =vec4(aPos,1.0);
	ourColor=aColor;
})glsl";

const char *fragmentShaderSource = R"glsl(#version 320 es
precision mediump float;
in vec3 ourColor;
out vec4 o_clr;
void main(){
	o_clr = vec4(ourColor, 1.0);
})glsl";
unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("vertex error:%s\n",infoLog);
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("fragment error:%s\n",infoLog);
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("link error:%s\n",infoLog);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // positions         // colors
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left
         -0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,   // top 
         0.5f, 0.5f,   0.0f,  1.0f, 1.0f, 0.0f,
    };
    unsigned int indics[]={0,1,2,2,3,0};
    unsigned int VBO, VAO,EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
		
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glGenBuffers(1,&EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,  sizeof(indics),
                 indics, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    //glEnableClientState(GL_VERTEX_ARRAY);
   glUseProgram(shaderProgram);

	for (int i = 0; i<10000; i++)
	{
		glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
			glUseProgram(shaderProgram);
			glViewport(0, 0, _screen_width, _screen_height);
			glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);

      glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
      
			glBindVertexArray(VAO);
			glScissor(0,0,0,0);
     glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
      //glDrawArrays(GL_TRIANGLES, 0, 3);

      glfwSwapBuffers(_window);
    	//eglSwapBuffers(sEGLDisplay, sEGLSurface);

	}
#endif



		//_pscr_ds = make_shared<screen_image_distortion>( _screen_width, _screen_height );
		while (1)//!glfwWindowShouldClose(_window))
		{
			// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
			// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
			// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
			// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
			//glfwPollEvents();
			static int dcnt=0;
			t_bf_render=steady_clock::now();
			ImGui_ImplGlfwGL3_NewFrame();
			ImGui::SetNextWindowSize(ImVec2(_win_width, _win_height), ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowPos(ImVec2(_wposx, _wposy));
			if(dcnt==0){
				//printf("win x=%f,y=%f, w=%f,h=%f\n",_wposx, _wposy,_win_width,_win_height);
			} 
			ImGui::SetNextWindowBgAlpha(1.f);
			static bool show_app = true;
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
			ImGui::Begin("edit window", &show_app, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoScrollbar |ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove 
				| ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoCollapse);
			//
			if (_proot)
			{
				onUpdate();
				keep_state_trans_on();
				execute_lazy_value();
        _proot->draw_frames();
				
			}
			ImGui::End();
			ImGui::PopStyleVar();
			ImGui::PopStyleVar();
			// Rendering
			int display_w, display_h;
			glfwGetFramebufferSize(_window, &display_w, &display_h);
			
			if(dcnt==0){
				printf("~~~~~~~~~~fps=%.1f\n",ImGui::GetIO().Framerate);
			} 
			
			
			glViewport(0, 0, display_w, display_h);
			glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
			glClear(GL_COLOR_BUFFER_BIT);
			ImGui::Render();
			//_pscr_ds->bind_framebuffer();
			ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
			//_pscr_ds->disbind_framebuffer();
      // _pscr_ds->draw();
			glfwSwapBuffers(_window);
			t_af_render=steady_clock::now();
			int delta = chrono::duration_cast<chrono::duration<int,std::milli>>(t_af_render - t_bf_render).count();
			if(delta<iper_sc)
			{
			    auto sp_pd=iper_sc-delta-10;
			    //this_thread::sleep_for(chrono::milliseconds(sp_pd));
			}
			dcnt++;
			dcnt%=600;
			
		}
		return true;
	}
	void application::set_screen_pos(float posx, float posy)
	{
		GLFWwindow* cur_c=glfwGetCurrentContext();
		_screen_posx=posx;
		_screen_posy=posy;
		
		glfwSetWindowPos(cur_c, posx, posy);
	}
	void application::set_image_height(int height)
	{
		//_pscr_ds->set_height(height);
	}

	void application::set_rotate_angle( float angle )
	{
		 //_pscr_ds->set_rotate_angle( angle );
	}
	void application::set_rotate_axis_pos( float px, float py )
	{
		 //_pscr_ds->set_rotate_axis_pos( px, py );
	}

	void application::destroy()
	{
		ImGui_ImplGlfwGL3_Shutdown();
		ImGui::DestroyContext();
		glfwTerminate();
	}
}
