// dear imgui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

#if defined (_MSC_VER)
#define NOMINMAX
#endif

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "matrix.h";

#include <stdio.h>
#include <vector>
#include <algorithm>
#include <numeric>
#include <string>
#include <string_view>
#include <atomic>
#include <thread>
#include <fstream>

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>    // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

template <class FibonacciRandomAccessIt>
auto get_fibonacci_points(FibonacciRandomAccessIt begin, FibonacciRandomAccessIt end) -> std::vector<ImVec2>;

std::thread render_fibonacci_spiral(
	int window_width, 
	int window_height, 
	unsigned int first_fibonacci_number, 
	unsigned int second_fibonacci_number, 
	std::atomic<bool>& started, 
	std::atomic<bool>& sequence_ready, 
	std::atomic<bool>& proceed,
	std::string_view load_filename, 
	bool save = false);

int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

	int width = 1280, height = 720;

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(width, height, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	char first_fib_buf[16] = { 0, };
	char second_fib_buf[16] = { 0, };
	char load_filename_buf[256] = { 0, };
	bool pressed = false;
	bool save = false;
	bool load = false;
	std::atomic<bool> started = false;
	std::atomic<bool> sequence_ready = false;
	std::atomic<bool> proceed = false;
	std::thread worker_thread{};

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

		ImGui::Begin("Fibonacci golden ratio approximation");
		ImGui::SetWindowSize(ImVec2((float)width, (float)height), ImGuiCond_::ImGuiCond_Always);
		ImGui::SetWindowPos(ImVec2(0.f, 0.f), ImGuiCond_::ImGuiCond_Always);

		ImGui::Begin("Input");
		ImGui::SetWindowSize(ImVec2(300.f, 150.f), ImGuiCond_::ImGuiCond_Always);

		auto window_pos = ImGui::GetWindowPos();
		ImGui::SetWindowPos(window_pos, ImGuiCond_::ImGuiCond_Always);
		auto window_size = ImGui::GetWindowSize();
		ImGui::SetWindowSize(window_size, ImGuiCond_::ImGuiCond_Always);
		
		ImGui::InputText("First fibonacci number", first_fib_buf, sizeof(first_fib_buf));
		ImGui::InputText("Second fibonacci number", second_fib_buf, sizeof(second_fib_buf));

		if (ImGui::Button(pressed ? "Stop generating from numbers" : "Generate"))
		{
			pressed = !pressed;
		}

		if (pressed)
		{
			ImGui::Text("Generating from numbers");
		}

		if (ImGui::Button(save ? "Stop saving to file" : "Save fibonacci sequence"))
		{
			save = !save;
		}

		if (save)
		{
			ImGui::Text("Saving to 'fibonacci.bin'");
		}

		if (ImGui::Button(load ? "Stop generating from file" : "Load .bin fibonacci sequence"))
		{
			load = !load;
		}

		ImGui::InputText("Path to .bin file", load_filename_buf, sizeof(load_filename_buf));

		if (load && !pressed)
		{
			ImGui::Text("Generating from file");
		}

		ImGui::End();

		auto run = [&](int f1, int f2, std::string_view filename)
		{
			if (!started)
			{
				worker_thread = render_fibonacci_spiral(
					width,
					height,
					f1,
					f2,
					started,
					sequence_ready,
					proceed,
					filename,
					save);
			}
			else if (sequence_ready)
			{
				proceed = true;

				if (worker_thread.joinable())
					worker_thread.join();

				sequence_ready = false;
				proceed = false;
				started = false;
			}
		};

		if (load && !pressed)
		{
			std::string_view filename(load_filename_buf);
			std::string_view sub = "";

			if (filename.size() > 4)
			{
				sub = filename.substr(filename.size() - 4, 4);
			}

			if (!filename.empty() && 
				sub == ".bin" && 
				std::ifstream(filename.data()).is_open())
			{
				run(0, 0, filename);
			}
			else
			{
				load = false;
			}
		}

		if (pressed)
		{
			auto f1 = std::atoi(first_fib_buf);
			auto f2 = std::atoi(second_fib_buf);
			if ((f1 >= 0 && f2 > 0 &&
				f2 > f1))
			{
				run(f1, f2, "");
			}
		}

		ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

std::thread render_fibonacci_spiral(
	int window_width,
	int window_height,
	unsigned int first_fibonacci_number,
	unsigned int second_fibonacci_number,
	std::atomic<bool>& started,
	std::atomic<bool>& sequence_ready,
	std::atomic<bool>& proceed,
	std::string_view filename, 
	bool save
	)
{
	started = true;
	return std::thread{ [&, window_width, window_height, first_fibonacci_number, second_fibonacci_number, save, filename]() 
	{ 
		using points_type = std::vector<ImVec2>;
		points_type points{};
		typename points_type::const_iterator xmin, xmax, ymin, ymax;

		while (true)
		{
			if (!sequence_ready)
			{
				std::vector<int> fibonacci;

				if (filename.empty())
				{
					fibonacci.resize(std::size_t(second_fibonacci_number) + 1, 0);
					std::fill(fibonacci.begin() + 1, fibonacci.end(), 1);
					std::adjacent_difference(fibonacci.begin() + 1, fibonacci.end() - 1, fibonacci.begin() + 2, std::plus<int>{});
				}
				else
				{
					// If reading the .txt file, it will be less performant
					//std::ifstream ifs{ "fibonacci.txt"};
					//std::copy(std::istream_iterator<int>(ifs), std::istream_iterator<int>(), std::back_inserter(fibonacci));

					std::ifstream ifs{ filename.data(), std::ios::binary | std::ios::ate };

					if (!ifs.is_open())
					{
						started = false;
						sequence_ready = false;
						proceed = false;
						return;
					}

					auto end = ifs.tellg();
					ifs.seekg(0, std::ios::beg);
					auto beg = ifs.tellg();
					auto fsize = end - beg;
					auto elem_count = fsize / sizeof(int);
					fibonacci.resize(elem_count);
					ifs.read((char*)fibonacci.data(), fsize);
				}

				if (save)
				{
					std::ofstream ofs("fibonacci.bin", std::ios::binary);
					// ImVec2 is packed, so we can just memory map it on disk
					ofs.write((char*)fibonacci.data(), fibonacci.size() * sizeof(int));

					// write human readable version
					std::ofstream ofst("fibonacci.txt");
					std::copy(fibonacci.cbegin(), fibonacci.cend(), std::ostream_iterator<int>(ofst, " "));
				}

				points = get_fibonacci_points(fibonacci.cbegin() + first_fibonacci_number, fibonacci.cend());


				auto xpair = std::minmax_element(points.cbegin(), points.cend(), [](auto const& p1, auto const& p2) { return p1.x < p2.x; });
				xmin = xpair.first;
				xmax = xpair.second;
				auto ypair = std::minmax_element(points.cbegin(), points.cend(), [](auto const& p1, auto const& p2) { return p1.y < p2.y; });
				ymin = ypair.first;
				ymax = ypair.second;
				sequence_ready = true;
			}

			bool valid = first_fibonacci_number >= second_fibonacci_number;
			if (proceed)
			{
				int xoffset = -(xmin->x), yoffset = ymax->y;

				auto world_height = std::abs(ymax->y - ymin->y);
				auto world_width = std::abs(xmax->x - xmin->x);

				auto xscale = (float)window_width / world_width;
				auto yscale = (float)window_height / world_height;

				auto draw_rect = [&xoffset, &yoffset, &xscale, &yscale](ImVec2 const& prev, ImVec2 const& next)
				{
					auto [x1, y1] = std::make_pair(prev.x, prev.y);
					auto [x2, y2] = std::make_pair(next.x, next.y);

					auto const leftmost = std::min(x1, x2);
					auto const rightmost = std::max(x1, x2);
					auto const uppermost = std::max(y1, y2);
					auto const lowermost = std::min(y1, y2);

					auto world_lower_left = fib::point2d<float>(leftmost, lowermost);
					auto world_lower_right = fib::point2d<float>(rightmost, lowermost);
					auto world_upper_left = fib::point2d<float>(leftmost, uppermost);
					auto world_upper_right = fib::point2d<float>(rightmost, uppermost);

					// change coordinate system for screen coordinates
					y1 = -y1;
					y2 = -y2;

					auto x = std::min(x1, x2);
					auto y = std::min(y1, y2);
					auto xlength = std::abs(x2 - x1);
					auto ylength = std::abs(y2 - y1);

					x += xoffset;
					y += yoffset;

					x *= xscale;
					y *= yscale;

					xlength *= xscale;
					ylength *= yscale;

					auto lower_left = fib::point2d<float>(x, y + ylength);
					auto upper_left = fib::point2d<float>(x, y);
					auto lower_right = fib::point2d<float>(x + xlength, y + ylength);
					auto upper_right = fib::point2d<float>(x + xlength, y);

					ImGui::GetWindowDrawList()->AddRect(
						ImVec2(upper_left.x(), upper_left.y()),
						ImVec2(lower_right.x(), lower_right.y()),
						ImGui::GetColorU32(ImVec4(255, 255, 255, 1.f)));

					return next;
				};

				std::accumulate(points.crbegin() + 1, points.crend(), *points.crbegin(), draw_rect);

				break;
			}
		}

		return;
	}};
}

template <class FibonacciRandomAccessIt>
auto get_fibonacci_points(FibonacciRandomAccessIt begin, FibonacciRandomAccessIt end) -> std::vector<ImVec2>
{
	static auto const segments = 10;
	std::vector<ImVec2> points{};
	auto const count = std::distance(begin, end) * segments;
	points.reserve(count);

	fib::static_matrix_2f_90d rotation;
	// start from [0, -1] unit vector
	auto iterator = fib::circular_scale_iterator_2f{};

	auto reduce_op = [&](fib::vector2d<float> const& v1, std::tuple<float, int> const& pair)
	{
		const auto [k, d] = pair;

		fib::scale_translate_matrix_2f scale = iterator->scale_translate(k, d);
		++iterator;

		auto v = rotation * (scale * v1);

		points.emplace_back(ImVec2{ v1.p2().x(), v1.p2().y() });

		return v;
	};

	auto transform_op = [](int prev, int next) -> std::tuple<float, int>
	{
		if (prev == 0) return { 1.f, 0 };
		auto scale = static_cast<float>(next) / static_cast<float>(prev);
		auto distance = next - prev;
		return { scale, distance };
	};

	fib::point2d<float> p1(0, 0), p2(1, 0);
	fib::vector2d<float> v(p1, p2);

	std::vector<std::tuple<float, int>> transformed{ static_cast<unsigned long long>(count) };
	std::transform(begin, end - 1, begin + 1, transformed.begin(), transform_op);
	std::accumulate(transformed.cbegin(), transformed.cend(), v, reduce_op);

	return points;
};
