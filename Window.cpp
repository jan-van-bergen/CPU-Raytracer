#include "Window.h"

#include <cstring>

#include <Imgui/imgui.h>
#include <Imgui/imgui_impl_sdl.h>
#include <Imgui/imgui_impl_opengl3.h>

Window::Window(int width, int height, const char * title) : 
	width(width), height(height), 
	tile_count_x((width  + tile_width  - 1) / tile_width), 
	tile_count_y((height + tile_height - 1) / tile_height)
{
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	window  = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);
	context = SDL_GL_CreateContext(window);

	SDL_GL_SetSwapInterval(0);

	GLenum status = glewInit();
	if (status != GLEW_OK) {
		printf("Glew failed to initialize!\n");
		abort();
	}

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);

	glDisable(GL_DEPTH_TEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

	frame_buffer = new unsigned[width * height];
	
	GLuint frame_buffer_handle;
	glGenTextures(1, &frame_buffer_handle);

	glBindTexture(GL_TEXTURE_2D, frame_buffer_handle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, frame_buffer);
	
#if ENABLE_FXAA
	Shader shader = Shader::load(DATA_PATH("Shaders/vertex.glsl"), DATA_PATH("Shaders/fragment_fxaa.glsl"));
	shader.bind();
	
	glUniform1i(shader.get_uniform("screen"), 0);
	glUniform2f(shader.get_uniform("inv_screen_size"), 1.0f / float(SCREEN_WIDTH), 1.0f / float(SCREEN_HEIGHT));
#else 
	Shader shader = Shader::load(DATA_PATH("Shaders/vertex.glsl"), DATA_PATH("Shaders/fragment_identity.glsl"));
	
	shader.bind();
	glUniform1i(shader.get_uniform("screen"), 0);
#endif

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// Setup Platform/Renderer bindings
	ImGui_ImplSDL2_InitForOpenGL(window, context);
	ImGui_ImplOpenGL3_Init("#version 450");

	ImGui::StyleColorsDark();
}

Window::~Window() {
	delete [] frame_buffer;

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void Window::clear() {
	memset(frame_buffer, 0, width * height * sizeof(unsigned));
}

void Window::draw_quad() const {
	glClear(GL_COLOR_BUFFER_BIT);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, frame_buffer);
	
	// Draws a single Triangle, without any buffers
	// The Vertex Shader makes sure positions + uvs work out
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Window::gui_begin() const {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();
}

void Window::gui_end() const {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Window::swap() {
	SDL_GL_SwapWindow(window);

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		ImGui_ImplSDL2_ProcessEvent(&event);

		if (event.type == SDL_QUIT) {
			is_closed = true;
		}
	}
}
