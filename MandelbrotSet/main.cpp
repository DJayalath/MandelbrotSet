#include <SDL.h>
#include <iostream>
#include <thread>
#include <vector>

// Standard library
using namespace std;

// Window dimensions
static constexpr unsigned int s_width = 1280;
static constexpr unsigned int s_height = 700;
// Number of iterations of search
unsigned int MAX_ITERATIONS = 20;
// Target number of threads (actual depends on nearest factor of height)
static constexpr unsigned int TARGET_THREADS = 6;
// Step by which one can zoom or pan
static constexpr double ZOOM_FACTOR = 0.9;
static constexpr unsigned int PAN_AMOUNT = 40;
// 256 colour RGBA
static constexpr unsigned int COLOUR_DEPTH = 256;
// Each pixel stores an int for R, G, B and A
static constexpr unsigned int PIXEL_DEPTH = 4;

SDL_Window* window = NULL;
SDL_Surface* surface = NULL;

void PrintIterations()
{
	cout << "MAX_ITERATIONS: " << MAX_ITERATIONS << endl;
}

int GetClosestFactor(int target, int number)
{
	for (int i = 0; i < number; i++) 
		if (number % (target + i) == 0)
			return target + i;
		else if (number % (target - i) == 0)
			return target - i;

	return number;
}

void DrawPixel(unsigned int x, unsigned int y, unsigned int r, unsigned int g, unsigned int b)
{
	Uint8* pixel_ptr = (Uint8*)surface->pixels + (y * s_width + x) * PIXEL_DEPTH;
	*(pixel_ptr) = r;
	*(pixel_ptr + 1) = g;
	*(pixel_ptr + 2) = b;
}

int FindMandelbrot(double c_re, double c_im)
{
	double z_re = c_re, z_im = c_im, re_sq, im_sq;
	unsigned int counter;

	for (counter = 0; counter < MAX_ITERATIONS; ++counter)
	{
		re_sq = z_re * z_re;
		im_sq = z_im * z_im;

		if (re_sq + im_sq > 4.0)
			return counter;

		z_im = 2.0 * z_re * z_im + c_im;
		z_re = re_sq - im_sq + c_re;
	}

	return MAX_ITERATIONS;
}

void UpdateImageSlice(double zoom, double x_offset, double y_offset, unsigned int min_im, unsigned int max_im)
{
	double imagstart, c_re, c_im;
	unsigned int N, j, k;

	c_re = 0 * zoom - s_width / 2.0 * zoom + x_offset;
	imagstart = min_im * zoom - s_height / 2.0 * zoom + y_offset;

	for (j = 0; j < s_width; j++, c_re += zoom)
	{
		c_im = imagstart;
		for (k = min_im; k < max_im; k++, c_im += zoom)
		{
			N = FindMandelbrot(c_re, c_im);

			if (N == MAX_ITERATIONS)
				DrawPixel(j, k, 0, 0, 0);
			else
				DrawPixel(j, k, (N * N) % COLOUR_DEPTH, N % COLOUR_DEPTH, (N * N * N) % COLOUR_DEPTH);
		}
	}
}

void UpdateImage(double zoom, double x_offset, double y_offset, unsigned int slice_size)
{
	vector<thread> threads;

	for (int i = 0; i < s_height; i += slice_size)
		threads.push_back(thread(UpdateImageSlice, zoom, x_offset, y_offset, i, i + slice_size));

	for (auto &t : threads)
		t.join();

	SDL_UpdateWindowSurface(window);
}

int main(int argc, char* argv[])
{
	// SDL Video initialisation
	SDL_Init(SDL_INIT_VIDEO);
	// Create SDL window with given dimensions and title
	window = SDL_CreateWindow("Mandelbrot Set Viewer", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, s_width, s_height, SDL_WINDOW_SHOWN);
	// Setup surface for updating pixels
	surface = SDL_GetWindowSurface(window);
	// Create SDL Event handler
	SDL_Event e;

	/* Assign initial values for magnification
	and panning offsets	*/
	double zoom = 0.004;
	double x_offset = -0.7;
	double y_offset = 0.0;

	// Calculate optimal slice size from target number of threads
	unsigned int slice_size = s_height / GetClosestFactor(TARGET_THREADS, s_height);

	// Draw set using initial values
	UpdateImage(zoom, x_offset, y_offset, slice_size);

	cout << "---- CONTROLS ----" << endl;
	cout << " - Arrow Keys for PANNING" << endl;
	cout << " - Z for ZOOM IN / X for ZOOM OUT" << endl;
	cout << " - Q for + RES / W for - RES" << endl;
	PrintIterations();

	bool quit = false;
	bool state_change = false;
	while (!quit)
	{
		while (SDL_PollEvent(&e) != 0)
			if (e.type == SDL_QUIT) quit = true;
			else if (e.type == SDL_KEYDOWN)
			{
				state_change = true;
				switch (e.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					quit = true;
					break;
				case SDLK_UP:
					y_offset -= PAN_AMOUNT * zoom;
					break;
				case SDLK_DOWN:
					y_offset += PAN_AMOUNT * zoom;
					break;
				case SDLK_LEFT:
					x_offset -= PAN_AMOUNT * zoom;
					break;
				case SDLK_RIGHT:
					x_offset += PAN_AMOUNT * zoom;
					break;
				case SDLK_z:
					zoom *= ZOOM_FACTOR;
					break;
				case SDLK_x:
					zoom /= ZOOM_FACTOR;
					break;
				case SDLK_q:
					MAX_ITERATIONS += 10;
					PrintIterations();
					break;
				case SDLK_w:
					MAX_ITERATIONS -= (MAX_ITERATIONS > 10) ? 10 : 0;
					PrintIterations();
					break;
				default:
					state_change = false;
					break;
				}
			}

		if (state_change)
			UpdateImage(zoom, x_offset, y_offset, slice_size);
	}

	// Destroy window 
	SDL_DestroyWindow(window);
	// Quit SDL subsystems 
	SDL_Quit();

	return 0;
}