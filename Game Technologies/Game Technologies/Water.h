#pragma once
#pragma comment (lib, "cudart.lib")

#include "../../nclgl/Mesh.h"

#include <minmax.h>
#include <cuda_gl_interop.h>
#include <cuda_runtime.h>

using namespace std;

class WaterMesh : public Mesh {
public:
	WaterMesh();
	~WaterMesh();

	virtual void Draw();

	void update(float msec);
private:
	void generateGrid();
	float time;
	unsigned int restart_index;
	int width, height;
	struct cudaGraphicsResource* cudaVBO;
};