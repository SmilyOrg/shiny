
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>

#define _USE_MATH_DEFINES
#include <math.h>

#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/generate.h>
#include <thrust/copy.h>
#include <thrust/sort.h>

#include "geom.hpp"
#include "server.hpp"
#include "platform.hpp"

#include "proto/renderRequest.pb.h"

struct Stage {
	int id;
	std::vector<ujson::value> vectors;
};

static std::vector<Stage> stages;
static Stage* stage = NULL;

ujson::value to_json(Stage const &s) {
	std::ostringstream id, preid;
	id << "vec" << s.id;
	preid << "#vec" << s.id-1;
    return ujson::array{
		ujson::array{
			"remove", preid.str(), ujson::object{ { "duration", 100 } },
		},
		ujson::array{
			"add", "vector", ujson::object{
				{ "id", id.str() },
				{ "n", (int)s.vectors.size()/2 },
				{ "data", s.vectors },
				{ "line", true },
				{ "arrow", true },
				{ "size", 0.01 }
			}
		}
	};
}

void nextStage() {
	stages.push_back(Stage());
	stage = &stages.back();
	stage->id = stages.size()-1;
}
void addVector(Vector3 &p) {
	stage->vectors.push_back(ujson::array{ 0, 0, 0 });
	stage->vectors.push_back(ujson::array{ p.x(), p.y(), p.z() });
}
void addVector(Vector3 &a, Vector3 &b) {
	stage->vectors.push_back(ujson::array{ a.x(), a.y(), a.z() });
	stage->vectors.push_back(ujson::array{ b.x(), b.y(), b.z() });
}
void addVectorDir(Vector3 &a, Vector3 &b) {
	stage->vectors.push_back(ujson::array{ a.x(), a.y(), a.z() });
	stage->vectors.push_back(ujson::array{ a.x() + b.x(), a.y() + b.y(), a.z() + b.z() });
}






typedef unsigned char CubeSide;
struct CubeSides {
	enum side {
		x = 2,
		y = 0,
		z = 1
	};
};

/*
void updatePosition(int n,
	thrust::host_vector<Vector3f> &rpos,
	thrust::host_vector<Vector3f> &rdir,
	thrust::host_vector<Vector3i> &pos,
	thrust::host_vector<Vector3f> &tmax,
	thrust::host_vector<CubeSide> &side
){
	for (int i = 0; i < n; i++) {
		Vector3f &rp = rpos[i];
		Vector3i &p = pos[i];
		p.x = floor(rp.x);
		p.y = floor(rp.y);
		p.z = floor(rp.z);
		
		Vector3f &rd = rdir[i];
		Vector3f &tm = tmax[i];
		tm.x = rd.x >= 0.f ? (p.x+1-rp.x)/rd.x : (p.x-rp.x)/rd.x;
		tm.y = rd.y >= 0.f ? (p.y+1-rp.y)/rd.y : (p.y-rp.y)/rd.y;
		tm.z = rd.z >= 0.f ? (p.z+1-rp.z)/rd.z : (p.z-rp.z)/rd.z;
		
		side[i] =
			tm.x < tm.y ?
				tm.x < tm.z ? CubeSides::x : CubeSides::z :
				tm.y < tm.z ? CubeSides::y : CubeSides::z
		;
	}
}

void initTrace(int n,
	thrust::host_vector<Vector3f> &rpos,
	thrust::host_vector<Vector3f> &rdir,
	thrust::host_vector<Vector3i> &pos,
	thrust::host_vector<Vector3i> &step,
	thrust::host_vector<Vector3f> &tmax,
	thrust::host_vector<Vector3f> &tdelta,
	thrust::host_vector<CubeSide> &side
){
	for (int i = 0; i < n; i++) {
		Vector3f &rd = rdir[i];
		Vector3i &s = step[i];
		s.x = rd.x > 0.f ? 1 : -1;
		s.y = rd.y > 0.f ? 1 : -1;
		s.z = rd.z > 0.f ? 1 : -1;
		Vector3f &td = tdelta[i];
		float invd;
		invd = 1/rdir[i].x; td.x = invd < 0 ? -invd : invd;
		invd = 1/rdir[i].y; td.y = invd < 0 ? -invd : invd;
		invd = 1/rdir[i].z; td.z = invd < 0 ? -invd : invd;
	}
	
	updatePosition(n, rpos, rdir, pos, tmax, side);
}

void step(int n,
	thrust::host_vector<Vector3i> &pos,
	thrust::host_vector<Vector3i> &step,
	thrust::host_vector<Vector3f> &tmax,
	thrust::host_vector<Vector3f> &tdelta,
	thrust::host_vector<CubeSide> &side
){
	for (int i = 0; i < n; i++) {
		Vector3i &p = pos[i];
		Vector3i &s = step[i];
		Vector3f &tm = tmax[i];
		Vector3f &td = tdelta[i];
		if (tm.x < tm.y) {
			if (tm.x < tm.z) {
				p.x += s.x;
				tm.x += td.x;
				side[i] = CubeSides::x;
			} else {
				p.z += s.z;
				tm.z += td.z;
				side[i] = CubeSides::z;
			}
		} else {
			if (tm.y < tm.z) {
				p.y += s.y;
				tm.y += td.y;
				side[i] = CubeSides::y;
			} else {
				p.z += s.z;
				tm.z += td.z;
				side[i] = CubeSides::z;
			}
		}
	}
}
*/

/*
struct initPosition
{
	__host__ __device__
	float operator() (const float &x) const
	{
		
	}
}

void printElement(int i,
	thrust::host_vector<Vector3f> &rpos,
	thrust::host_vector<Vector3f> &rdir,
	thrust::host_vector<Vector3i> &pos,
	thrust::host_vector<Vector3i> &step,
	thrust::host_vector<Vector3f> &tmax,
	thrust::host_vector<Vector3f> &tdelta,
	thrust::host_vector<CubeSide> &side
){
	printf("rpos: "); rpos[i].print(); printf("\n");
	printf("rdir: "); rdir[i].print(); printf("\n");
	printf("pos : ");  pos[i].print(); printf("\n");
	printf("step: "); step[i].print(); printf("\n");
	printf("tmax: "); tmax[i].print(); printf("\n");
	printf("tdel: "); tdelta[i].print(); printf("\n");
	printf("side: %d\n", side[i]);
	printf("---------------------\n");
}
*/



void trace(Ray &r, Scene &scene) {
	if (r.done) return;
	Intersection nearest(INFINITY);
	for (int i = 0; i < scene.spheres.size(); i++) {
		Sphere &s = scene.spheres[i];
		Intersection inter = s.intersects(r);
		if (inter.t > 0 && inter.t < nearest.t) {
			nearest = inter;
			nearest.material = &s.material;
		}
	}
	Vector3 nearestDisp = (Vector3)(r.dir*(Number)min(nearest.t, (Number)100));
	if (nearest.t < INFINITY) {
		addVectorDir(r.pos, nearestDisp);
		addVectorDir((Vector3) (r.pos + nearestDisp), nearest.normal);
	}
	r.nearest = nearest;
}
void traceRays(thrust::host_vector<Ray> &h_rays, Scene &scene) {
	int n = h_rays.size();
	for (int i = 0; i < n; i++) {
		Ray &r = h_rays[i];
		trace(r, scene);
	}
}



void interact(Ray &r) {
	if (r.done || r.nearest.t == INFINITY) return;
	bool doEmission = (rand() % 2 == 0);
	if (doEmission) {
		// Emission
		r.color = r.weight * r.nearest.material->getEmission();
		r.done = true;
		addVectorDir(r.nearest.pos, (Vector3)-r.dir);
	} else {
		// Reflection
		r.weight *= 0.9;
		r.pos = r.nearest.pos;
		r.dir = r.nearest.material->getDirection(r.nearest.pos, r.nearest.normal);
		addVectorDir(r.pos, r.dir);
	}
}
void interactRays(thrust::host_vector<Ray> &h_rays) {
	int n = h_rays.size();
	for (int i = 0; i < n; i++) {
		Ray &r = h_rays[i];
		interact(r);
	}
}



ujson::value to_json(Sphere const &s) {
    return ujson::object{
		{ "x", s.pos.x() },
		{ "y", s.pos.y() },
		{ "z", s.pos.z() },
		{ "radius", s.radius },
	};
}

ujson::value to_json(Scene const &s) {
	std::vector<Sphere> spheres;
	for (int i = 0; i < s.spheres.size(); i++) {
		spheres.push_back(s.spheres[i]);
	}
	
    return ujson::object{ { "spheres", spheres } };
}

#define OUTPUT_FILE "vis/data/data.js"

struct Render {
	Camera cam;
	Frustum frustum;
	Vector3 eye;
	Vector3 target;
	Vector3 up;
	
	thrust::host_vector<Ray> h_rays;
	
	Scene scene;
	
	int width;
	int height;
	
	Render() :
		width(20), height(20),
		
		up(Vector3(0, 1, 0))
	{}
	
	void start() {	
		cam.lookAt(eye, target, up);
		
		cout << "View:" << endl << cam.view << endl;
		cout << "Proj:" << endl << cam.projection << endl;
		cout << "Tran:" << endl << cam.transform << endl;
		
		cout << cam.view * Vector4f(0, 0, 0, 1) << endl;
		
		frustum.update(cam.transform);
		
		addVector(frustum.nearRect.tl, frustum.nearRect.tr);
		addVector(frustum.nearRect.tr, frustum.nearRect.br);
		addVector(frustum.nearRect.br, frustum.nearRect.bl);
		addVector(frustum.nearRect.bl, frustum.nearRect.tl);
		
		h_rays = thrust::host_vector<Ray>(width*height);
	}
	
	void initRays() {
		int n = h_rays.size();
		int w = width, h = height;
		for (int i = 0; i < n; i++) {
			Ray &r = h_rays[i];
			r.done = false;
			r.color = Vector3(0, 0, 0);
			
			int ix = i%w;
			int iy = i/w;
			Number px = ((Number)ix + 0.5)/w;
			Number py = ((Number)iy + 0.5)/h;
			r.pos = frustum.pointOnNearPlane(px, py);
			r.dir = r.pos - eye;
			r.dir.normalize();
			addVector(eye, r.pos);
			addVectorDir(r.pos, r.dir);
		}
	}
	
	void simulate() {
		cout << "Simulating..." << endl;
		
		for (int i = 0; i < 5; i++) {
			cout << "#" << i << endl; 
			nextStage();
			cout << "Tracing..." << endl;
			traceRays(h_rays, scene);
			nextStage();
			cout << "Interacting..." << endl;
			interactRays(h_rays);
		}
	}
	
};

int main()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	
	shiny::RenderRequest renreq;
  
	Render render;
	
	nextStage();
	
	render.eye    = Vector3(0, 1, 0);
	render.target = Vector3(0, 1, -1);
	
	render.scene.addSphere(-3, 1, -8, 3);
	render.scene.addSphere(3, 1.5, -7, 2);
	render.scene.spheres[1].material.emission = Vector3(0.5, 0.5, 0);
	
	render.start();
	
	cout << "Serializing..." << endl;
	
	auto script = stages;
	
	/*
	[
		[
			["add", "vector", {
				"n": 1,                              // Number of vectors
				"data": [[1, 1, 1], [1, 2, 3]],                        // Array of alternating start and end points,
				"line": true,                        // Whether to draw vector lines
				"arrow": true,                       // Whether to draw arrowheads
				"size": 0.07,                         // Size of the arrowhead relative to the stage
			}],
		]
	]
	*/
	
	ujson::value jsonScript{ script };
	ujson::value jsonScene = to_json(render.scene);
	
  	ofstream dataFile;
	dataFile.open (OUTPUT_FILE);
	dataFile << "window.mathboxScript = " << jsonScript << ";" << endl;
	dataFile << "window.mathboxScene = " << jsonScene << ";" << endl;
	dataFile.close();
  	
	
	Server server;
	
	server.start();
	
	while (!server.exit) {
		shSleep(1);
	}
	
	server.stop();
	
  	google::protobuf::ShutdownProtobufLibrary();
	
	cout << "Done!" << endl;
	  
	/*
	thrust::host_vector<Vector3f> h_rpos(n);
	thrust::host_vector<Vector3f> h_rdir(n);
	
	thrust::host_vector<Vector3i> h_pos(n);
	thrust::host_vector<Vector3i> h_step(n);
	thrust::host_vector<Vector3f> h_tmax(n);
	thrust::host_vector<Vector3f> h_tdelta(n);
	thrust::host_vector<CubeSide> h_side(n);
	
	for (int i = 0; i < n; i++) {
		Vector3f *p;
		p = &h_rpos[i];
		p->set(0.5f, 0.5f, 0.5f);
		p = &h_rdir[i];
		p->set(0.01f, 1.f, -0.01f);
	}
	
	initTrace(n, h_rpos, h_rdir, h_pos, h_step, h_tmax, h_tdelta, h_side);
	
	printElement(0, h_rpos, h_rdir, h_pos, h_step, h_tmax, h_tdelta, h_side);
	
	step(n, h_pos, h_step, h_tmax, h_tdelta, h_side);
	
	printElement(0, h_rpos, h_rdir, h_pos, h_step, h_tmax, h_tdelta, h_side);
	*/
	
	return 0;
}