#include<cmath>
#include<chrono>
#include<iostream>
#include<thread>
#include<SDL2/SDL.h>
#include<vector>

/*
	distance is in meters
	k is in newton per meter
	mass is in kilograms
	g = 10
*/

const float g = 10.0f;

class pos2 {
public:
	float x, y;

	pos2(float xx, float yy) {
		x = xx;
		y = yy;
	}

	pos2() {
		x = 0.0f;
		y = 0.0f;
	}

	~pos2() = default;
};

class vec2 {
public:
	float x, y;
	
	float length() {
		return sqrt(x*x + y*y);
	}

	vec2 normalized() {
		float len = length();
		if (len == 0.0f)
			len = 0.0000000000000001f;

		return vec2{x/len, y/len};
	}

	vec2(float xx, float yy) {
		x = xx;
		y = yy;
	}

	vec2(pos2 a, pos2 b) {
		x = b.x-a.x;
		y = b.y-a.y;
	}

	vec2() {
		x = 0.0f;
		y = 0.0f;
	}
	
	~vec2() = default;
};

vec2 operator*(vec2 a, float b) {
	return vec2{a.x*b, a.y*b};
}

vec2 operator/(vec2 a, float b) {
	return vec2{a.x/b, a.y/b};
}

float distance(float x1, float y1, float x2, float y2) {
	float dx = x2-x1;
	float dy = y2-y1;

	return sqrt(dx*dx + dy*dy);
}

class Spring {
public:
	float x, y;
	float default_length;
	float k;

	float force(float length) {
		float x = length-default_length;
		
		return k*x;
	}

	Spring(float xi, float yi, float default_lengthi, float ki) {
		x = xi;
		y = yi;
		default_length = default_lengthi;
		k = ki;
	}

	Spring() {
		x = 0.0f;
		y = 0.0f;
		default_length = 0.0f;
		k = 0.0f;
	}

	~Spring() = default;
};

class Ball {
public:
	float x, y;
	float mass;
	float radius;

	float x_v, y_v;
	
	float x_force, y_force;

	void reset_forces() {
		x_force = 0.0f;
		y_force = 0.0f;
	}

	void apply(Spring spring) {
		float deformation = spring.default_length - distance(x, y, spring.x, spring.y);
		float absolute_force = -(deformation * spring.k);
		vec2 vec_force = vec2(pos2{x, y}, pos2{spring.x, spring.y}).normalized() * absolute_force; 
	
		x_force += vec_force.x;
		y_force += vec_force.y;
	}

	void apply(vec2 force) {
		x_force += force.x;
		y_force += force.y;
	}

	void drag() {
		float drag_coefficient = 10.0f;
		float drag_force = drag_coefficient * sqrt(x_v*x_v + y_v*y_v);
		vec2 vector_drag_force = vec2{x_v, y_v}.normalized() * (-1) * drag_force;
		
		float xmod = vector_drag_force.x;
	       	float ymod = vector_drag_force.y;	

		x_force += xmod;
		y_force += ymod;
	}

	void apply(float delta_time) {
		x_v += x_force/mass * delta_time;
		y_v += y_force/mass * delta_time;

		x += x_v * delta_time;
		y += y_v * delta_time;
	}

	bool stationary() {
		return (x_v == 0.0f && y_v == 0.0f);
	}

	Ball(float xi, float yi, float massi, float radiusi) {
		x = xi;
		y = yi;
		mass = massi;
		radius = radiusi;

		x_v = 0.0f;
		y_v = 0.0f;
	}

	Ball() {
		x = 0.0f;
		y = 0.0f;
		mass = 0.0f;
		radius = 10.0f;
		
		x_v = 0.0f;
		y_v = 0.0f;
	}

	~Ball() = default;
};

void draw_circle(SDL_Renderer* renderer, int cx, int cy, int radius) {
	const int segments = 100;
	for (int i = 0; i < segments; i++) {
		float theta1 = 2.0f * M_PI * i / segments;
		float theta2 = 2.0f * M_PI * (i+1) / segments;

		int x1 = cx + radius * cos(theta1);
		int y1 = cy + radius * sin(theta1);
		int x2 = cx + radius * cos(theta2);
		int y2 = cy + radius * sin(theta2);

		SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
	}
}

int main(int argc, char** argv) {
	SDL_Init(SDL_INIT_VIDEO);
	
	SDL_Window* window = SDL_CreateWindow("Spring Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 2400, 1200, 0);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	bool running = true;
	SDL_Event event;

	auto before = std::chrono::high_resolution_clock::now();

	Ball ball(12.0f, 15.0f, 100.0f, 15.0f); // xpos, ypos, mass, radius

	std::vector<Spring> springs = {
		Spring(12.0f, 5.0f, 10.0f, 8000.0f)
	};

	bool mouse_holding = false, last = false;
	float lx = 0.0f, ly = 0.0f;
	int mult = 10;
	while (running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
			} else if (event.type == SDL_MOUSEBUTTONDOWN) {
				mouse_holding = true;
			} else if (event.type == SDL_MOUSEBUTTONUP) {
				mouse_holding = false;
			} else if (event.type == SDL_KEYDOWN) {
				SDL_Keycode key = event.key.keysym.sym;
				if (key == SDLK_s) {
					int xx, yy;
					SDL_GetMouseState(&xx, &yy);
					float x = (float)xx;
				       	float y = (float)yy;
					x -= 1000;
					y = 600-y;
					x /= mult;
					y /= mult;

					springs.push_back(Spring(x, y, 10.0f, 8000.0f));	
				} else if (key == SDLK_r) {
					springs = {
						Spring(12.0f, 5.0f, 10.0f, 8000.0f)
					};
					ball = Ball(12.0f, 15.0f, 100.0f, 15.0f);
				}
			}
		}

		auto now = std::chrono::high_resolution_clock::now();
		auto delta_time = (now-before).count()/1000000000.0f;

		ball.reset_forces();

		for (int i = 0; i < springs.size(); i++) {
			ball.apply(springs[i]);
		}
		ball.apply(vec2{0.0f, -ball.mass * g});
		ball.drag();
		ball.apply(delta_time);		

		if (mouse_holding) {
			int xx, yy;
			SDL_GetMouseState(&xx, &yy);
			float x = (float)xx;
			float y = (float)yy;
			x -= 1000;
			y = 600-y;
			x /= mult;
			y /= mult;
			
			if (distance(x, y, ball.x, ball.y) <= ball.radius/mult) {
				ball.x_v = 0.0f;
				ball.y_v = 0.0f;
				
				ball.x = x;
				ball.y = y;

				lx = x;
				ly = y;
	
				last = true;
			}
		} else {
			if (last) {
				int xx, yy;
				SDL_GetMouseState(&xx, &yy);
				float x = (float)xx;
				float y = (float)yy;
				x -= 1000;
				y = 600-y;
				x /= mult;
				y /= mult;
			
				vec2 velocity = vec2(pos2(lx, ly), pos2(x, y))/delta_time/2.0f;
				ball.x_v = velocity.x;
				ball.y_v = velocity.y;
			}

			last = false;
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // black background
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // white drawing

		for (int i = 0; i < springs.size(); i++)
			SDL_RenderDrawLine(renderer, 1000+springs[i].x*mult, 1200-600-springs[i].y*mult, 1000+ball.x*mult, 1200-600-ball.y*mult);
		
		draw_circle(renderer, 1000+ball.x*mult, 1200-600-ball.y*mult, ball.radius);

		SDL_RenderPresent(renderer);

		std::cout<< "FPS : " << 1/delta_time << "\n";
		std::cout<< "DELTA_TIME : " << delta_time << " S\n";
		std::cout<< "(" << ball.x << "; " << ball.y << ")\n";
		
		if (delta_time < 1/600.0f)
			std::this_thread::sleep_for(std::chrono::milliseconds((int)((1/600.0f-delta_time)*1000.0f)));	
		before = now;
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
