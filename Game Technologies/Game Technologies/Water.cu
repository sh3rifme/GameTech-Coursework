#include "Water.h"
#include "Renderer.h"

WaterMesh::WaterMesh(): width(100), height(100) {
	width = max(2, width);
	height = max(2, height);
	generateGrid();
	time = 0.0f;
	cudaGLSetGLDevice(1);

	if (cudaGraphicsGLRegisterBuffer(&cudaVBO, bufferObject[VERTEX_BUFFER], cudaGraphicsMapFlagsNone) != cudaSuccess)
		printf("Failed\n");
}

WaterMesh::~WaterMesh() {
	if (cudaGraphicsUnregisterResource(cudaVBO) != cudaSuccess)
		printf("Failed\n");
}

void WaterMesh::generateGrid() {
	int loop_size = 2*height + 1;

	numVertices		= width*height;
	numIndices		= (width - 1)*loop_size;

	colours			= new Vector4[numVertices];
	vertices		= new Vector3[numVertices];
	normals			= new Vector3[numVertices];
	textureCoords	= new Vector2[numVertices];
	indices			= new unsigned int[numIndices];
	
	int offset = 0;

	type = GL_TRIANGLE_STRIP;
	for (int x = 0; x < width; x++) {
		int loops = x*loop_size;
		for (int y = 0; y < height; y++) {

			if (x != width - 1)
				indices[loops + 2*y + 1] = offset;
			if (x != 0)
				indices[loops - loop_size + 2*y] = offset;

			vertices[offset]		= Vector3(2*(x*1.0f/(width-1)) - 1, 0, 2*(y*1.0f/(height-1)) - 1);
			normals[offset]			= Vector3(0, 1, 0); 
			textureCoords[offset]	= Vector2(x*1.0f/(width-1), y*1.0f/(height-1));
			colours[offset]			= Vector4(0.0f, 0.0f, 0.0f, 0.5f);
			++offset;
		}
		if (x != width - 1)
			indices[loops + loop_size - 1] = width*height;
	}

	restart_index = width*height;

	BufferData();
	glBindVertexArray(0);
}

void WaterMesh::Draw() {
	glPrimitiveRestartIndex(restart_index);
	glEnable(GL_PRIMITIVE_RESTART);
	Mesh::Draw();
	glDisable(GL_PRIMITIVE_RESTART);
}

__global__ void vboWaterResource_update(float* ptr, int width, int height, float time) {
	int x = blockIdx.x*blockDim.x + threadIdx.x;
	int y = blockIdx.y*blockDim.y + threadIdx.y;
	int offset = y*width + x;
	if (x >= width || y >= height) return;

	float period = 30;
	float rate = 0.5;

	//Center of vertex array
	float cx = x*1.0f/width - 0.5f;
	float cy = y*1.0f/height - 0.5f;

	//Wave from the center
	float wave = sin(sqrt(cx*cx + cy*cy)*period - rate*time);

	int sign = wave>0?1:-1;
	wave = sign*sqrt(sign*wave);

	//Faster waves
	period *= 3;
	rate *= -9;
	//Add two waves, in x and y direction
	ptr[3*offset + 1] = (sin(x*period/(width - 1)) + sin(y*period/(height - 1) + rate*time) - height)/2;
}

void WaterMesh::update(float msec) {
	float* devBuff;
	size_t size;
	time += msec * 0.0005;
	dim3 threadsPerBlock(8, 8);
	dim3 numBlocks((width - 1)/threadsPerBlock.x + 1, (height - 1)/threadsPerBlock.y + 1);

	if (cudaGraphicsMapResources(1, &cudaVBO, 0) != cudaSuccess)
		printf("Failed\n");

	cudaGraphicsResourceGetMappedPointer((void**)&devBuff, &size, cudaVBO);

	vboWaterResource_update<<<numBlocks, threadsPerBlock>>>(devBuff, width, height, time);

	if (cudaGraphicsUnmapResources(1, &cudaVBO, 0) != cudaSuccess)
		printf("Failed\n");
}