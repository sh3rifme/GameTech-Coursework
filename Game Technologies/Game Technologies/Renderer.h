#pragma once

#include "../../nclgl/OGLRenderer.h"
#include "../../nclgl/SceneNode.h"
#include "../../nclgl/Camera.h"
#include "../../nclgl/Frustum.h"
#include "../../nclgl/Light.h"
#include "../../nclgl/TextMesh.h"
#include "Water.h"
#include <algorithm>

#define SHADOWSIZE 2048*4

class Renderer : public OGLRenderer	{
public:
	virtual void		RenderScene();
	virtual void		UpdateScene(float msec);

	void				SetCamera(Camera*c);

	void				AddNode(SceneNode* n);

	void				RemoveNode(SceneNode* n);

	//Statics
	static bool			Initialise() {
		instance = new Renderer(Window::GetWindow());
		return instance->HasInitialised();
	}

	static void			Destroy() {		delete instance;
	}
	
	static Renderer&	GetRenderer() { return *instance; }

	void				ToggleEffect(int arg);

	void				UpdateDisplayValues(int vars[]);
	void				UpdateFlockingValues(float vars[]);
protected:
	Renderer(Window &parent);
	virtual ~Renderer(void);

	bool				ActiveTex();
	void				PresentScene();
	void				RotateLight();
	void				DrawScene();
	void				ShadowPass();
	void				BuildNodeLists(SceneNode* from);
	void				SortNodeLists();
	void				ClearNodeLists();
	void				DrawNodes();
	void				DrawNode(SceneNode*n);
	void				DrawText(const std::string &text, const Vector3 &position, const float size, const bool perspective);
	void				ToggleWireFrame();
	
	string				IsEnabled(bool arg);

	Mesh*				quad;
	SceneNode*			root;
	Camera*				camera;
	Light*				light;
	Font*				tahoma;
	
	Shader*				sceneShader;
	Shader*				shadowShader;
	Shader*				skyBoxShader;
	Shader*				simpleShader;

	Frustum				frameFrustum;

	WaterMesh*			water;

	GLuint				shadowFBO;
	GLuint				bufferFBO;
	GLuint				processFBO;
	GLuint				bufferDepthTex;
	GLuint				bufferColourTex[2];
	GLuint				bufferNormalTex;
	GLuint				cubeMap;
	GLuint				shadowTex;

	bool				wireFrame;
	bool				activeTex;
	bool				gravity;

	vector<SceneNode*>	transparentNodeList;
	vector<SceneNode*>	nodeList;

	static Renderer*	instance;

	int					FPS;
	int					physFPS;
	int					objects;
	int					collisions;

	float				cohesion;
	float				align;
	float				seperation;
};