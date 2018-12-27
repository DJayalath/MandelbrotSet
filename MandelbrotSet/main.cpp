#include <SDL.h>
#include <iostream>

// Standard library
using namespace std;

// Graphics
SDL_Window *window = NULL;
SDL_Surface* surface = NULL;

// Window dimensions
unsigned int s_width = 640;
unsigned int s_height = 480;

//Definitions
float min_re = -1.5;
float max_re = 0.7;
float min_im = -1.0;
float max_im = 1.0;

// Reduce if lagging
unsigned int MAX_ITERATIONS = 50;
unsigned int j, k;
double c_re, c_im;
int N;

double zoom = 0.004;
double x_offset = 0.0;
double y_offset = 0.0;

void DrawPixel(unsigned int x, unsigned int y, unsigned int r, unsigned int g, unsigned int b)
{
	// THIS IS WRONG
	Uint8* pixel_ptr = (Uint8*)surface->pixels + (y * s_width + x) * 4;
	*(pixel_ptr) = r;
	*(pixel_ptr + 1) = g;
	*(pixel_ptr + 2) = b;
}

double MapReal(int x)
{
	return x * zoom - s_width / 2.0 * zoom + x_offset;
}

double MapImaginary(int y)
{
	return y * zoom - s_height / 2.0 * zoom + y_offset;
}

int FindMandelbrot(double c_re, double c_im)
{
	int i = 0;
	double z_re = 0.0, z_im = 0.0;
	double z_z_re = 0.0, z_z_im = 0.0;

	while (i < MAX_ITERATIONS && z_z_re + z_z_im < 4.0)
	{
		double temp = z_z_re - z_z_im + c_re;
		z_im = 2.0 * z_re * z_im + c_im;
		z_re = temp;
		i++;

		z_z_re = z_re * z_re;
		z_z_im = z_im * z_im;
	}

	return i;
}

void Render()
{
	for (unsigned int j = 0; j < s_width; j++)
		for (unsigned int k = 0; k < s_height; k++)
		{
			c_re = MapReal(j);
			c_im = MapImaginary(k);

			N = FindMandelbrot(c_re, c_im);

			if (N == MAX_ITERATIONS)
				DrawPixel(j, k, 0, 0, 0);
			else
				DrawPixel(j, k, (N * (int)sin(N)) % 256, (N * N) % 256, N % 256);
		}

	SDL_UpdateWindowSurface(window);
}

int main(int argc, char* argv[])
{
	// SDL boilerplate
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("Mandelbrot Set Viewer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, s_width, s_height, SDL_WINDOW_SHOWN);
	surface = SDL_GetWindowSurface(window);

	// SDL Event handler
	SDL_Event e;

	Render();

	cout << "---- CONTROLS ----" << endl;
	cout << " - Arrow Keys for PANNING" << endl;
	cout << " - Z for ZOOM IN / X for ZOOM OUT" << endl;

	bool quit = false;
	while (!quit)
	{
		while (SDL_PollEvent(&e) != 0)
			if (e.type == SDL_QUIT) quit = true;
			else if (e.type == SDL_KEYDOWN)
				switch (e.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					quit = true;
					break;
				case SDLK_UP:
					y_offset -= 40 * zoom;
					Render();
					break;
				case SDLK_DOWN:
					y_offset += 40 * zoom;
					Render();
					break;
				case SDLK_LEFT:
					x_offset -= 40 * zoom;
					Render();
					break;
				case SDLK_RIGHT:
					x_offset += 40 * zoom;
					Render();
					break;
				case SDLK_z:
					zoom *= 0.9;
					Render();
					break;
				case SDLK_x:
					zoom /= 0.9;
					Render();
					break;
				}
	}

	// Destroy window 
	SDL_DestroyWindow(window);
	// Quit SDL subsystems 
	SDL_Quit();

	return 0;
}