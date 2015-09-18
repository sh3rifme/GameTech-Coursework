#include "Renderer.h"

Renderer* Renderer::instance = NULL;

Renderer::Renderer(Window &parent) : OGLRenderer(parent)	{	
	camera			= NULL;

	root			= new SceneNode();

	quad			= Mesh::GenerateQuad();

	water			= new WaterMesh();
	water->SetTexture(SOIL_load_OGL_texture(TEXTUREDIR"water.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));
	water->SetBumpMap(SOIL_load_OGL_texture(TEXTUREDIR"waterDOT3.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	simpleShader	= new Shader(SHADERDIR"QuadVertex.glsl", SHADERDIR"QuadFrag.glsl");
	sceneShader		= new Shader(SHADERDIR"SceneVertex.glsl", SHADERDIR"SceneFragment.glsl");
	shadowShader	= new Shader(SHADERDIR"ShadowVertex.glsl", SHADERDIR"ShadowFragment.glsl");
	skyBoxShader	= new Shader(SHADERDIR"SkyBoxVertex.glsl", SHADERDIR"SkyBoxFragment.glsl");

	tahoma			= new Font(SOIL_load_OGL_texture(TEXTUREDIR"tahoma.tga",SOIL_LOAD_AUTO,SOIL_CREATE_NEW_ID,SOIL_FLAG_COMPRESS_TO_DXT),16,16);
	
	cubeMap			= SOIL_load_OGL_cubemap(TEXTUREDIR"rusted_west.jpg",TEXTUREDIR"rusted_east.jpg",
											TEXTUREDIR"rusted_up.jpg",TEXTUREDIR"rusted_down.jpg",
											TEXTUREDIR"rusted_south.jpg",TEXTUREDIR"rusted_north.jpg",
											SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	light			= new Light(Vector3(2000, 1000, 2000), Vector4(1,1,1,1), 5000.0f);

	if (!sceneShader->LinkProgram())
		return;
	if (!shadowShader->LinkProgram())
		return;
	if (!skyBoxShader->LinkProgram())
		return;
	if (!simpleShader->LinkProgram())
		return;
	if (!cubeMap)
		return;
	if (!water->GetTexture())
		return;
	if (!water->GetBumpMap())
		return;

	//Generate scene depth texture.
	glGenTextures(1, &bufferDepthTex);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	//And colour texture.
	for (int i = 0; i < 2; ++i) {
		glGenTextures(1, &bufferColourTex[i]);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}

	//Generate shadow buffer.
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &shadowFBO); //Shadow pre-render in this one.
	glGenFramebuffers(1, &bufferFBO);
	glGenFramebuffers(1, &processFBO);

	GLenum buffers[2];
	buffers[0] = GL_COLOR_ATTACHMENT0;
	buffers[1] = GL_COLOR_ATTACHMENT1;

	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, bufferNormalTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glDrawBuffers(1, buffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !bufferDepthTex || !bufferColourTex[0]) {
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !shadowTex) {
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	FPS				= 0;
	physFPS			= 0;
	collisions		= 0;
	objects			= 0;

	activeTex		= false;
	wireFrame		= false;
	gravity			= true;
	instance		= this;
	init			= true;
}

Renderer::~Renderer(void)	{
	delete root;

	delete sceneShader;
	delete skyBoxShader;
	delete shadowShader;
	delete simpleShader;
	delete light;
	delete water;

	currentShader = NULL;

	//Clear buffers
	glDeleteTextures(1, &shadowTex);
	glDeleteTextures(2, bufferColourTex);
	glDeleteTextures(1, &bufferDepthTex);
	glDeleteTextures(1, &bufferNormalTex);
	glDeleteFramebuffers(1, &bufferFBO);
	glDeleteFramebuffers(1, &processFBO);
	glDeleteFramebuffers(1, &shadowFBO);

	currentShader = NULL;
}

void Renderer::UpdateScene(float msec)	{
	if(camera) {
		camera->UpdateCamera(msec); 
	}
	root->Update(msec);
	water->update(msec);

	Vector3 translate = Vector3(0, 1000, 0);
	Matrix4 pushMatrix = Matrix4::Translation(translate);
	Matrix4 popMatrix = Matrix4::Translation(-translate);
	float radius = light->GetRadius();
	Matrix4 lightMatrix = pushMatrix * Matrix4::Rotation(0.5, Vector3(0, 1, 0)) * popMatrix *
							Matrix4::Translation(light->GetPosition()) *
							Matrix4::Scale(Vector3(radius, radius, radius));
	light->SetPosition(lightMatrix.GetPositionVector());
}

void Renderer::ShadowPass() {
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	
	SetCurrentShader(shadowShader);
	projMatrix		= Matrix4::Perspective(900.0f, 4000.0f, (float)width / (float)height, 45.0f);
	viewMatrix		= Matrix4::BuildViewMatrix(light->GetPosition(), Vector3(0, 0, 0));
	shadowMatrix	= biasMatrix*(projMatrix * viewMatrix);
	frameFrustum.FromMatrix(projMatrix * viewMatrix);

	UpdateShaderMatrices();

	BuildNodeLists(root);
	SortNodeLists();
	DrawNodes();
	ClearNodeLists();

	glUseProgram(0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawScene() {
	ShadowPass();
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glDepthMask(GL_FALSE);
	SetCurrentShader(skyBoxShader);

	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	viewMatrix = camera->BuildViewMatrix();
	modelMatrix.ToIdentity();
	UpdateShaderMatrices();

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	quad->Draw();

	glUseProgram(0);
	glDepthMask(GL_TRUE);

	if(camera) {
		SetCurrentShader(sceneShader);
		SetShaderLight(*light);
		glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "shadowTex"), 9);
		glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);

		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, shadowTex);
		
		projMatrix		= Matrix4::Perspective(1.0f,10000.0f,(float)width / (float) height, 45.0f);
		frameFrustum.FromMatrix(projMatrix * viewMatrix);
		modelMatrix = Matrix4::Scale(Vector3(3000, 25, 3000)) * Matrix4::Translation(Vector3(0,40,0));
		UpdateShaderMatrices();

		glDisable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if (wireFrame)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);			

		water->Draw();

		modelMatrix.ToIdentity();
		UpdateShaderMatrices();

		BuildNodeLists(root);
		SortNodeLists();
		DrawNodes();
		ClearNodeLists();

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
}

void Renderer::PresentScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	SetCurrentShader(simpleShader);
	projMatrix = Matrix4::Orthographic(-1, 1, 1, -1, -1, 1);
	viewMatrix.ToIdentity();
	modelMatrix.ToIdentity();
	UpdateShaderMatrices();

	quad->SetTexture(bufferColourTex[0]);
	quad->Draw();

	int spacing = 20;

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	DrawText("Physics FPS: " + to_string(physFPS) + "Hz", Vector3(0,0,0), 16.0f, false);
	DrawText("Graphical FPS: " + to_string(FPS) + "Hz", Vector3(0,spacing,0), 16.0f, false);
	DrawText("Objects in scene: " + to_string(objects), Vector3(0,spacing*2,0), 16.0f, false);
	DrawText("Collisions: " + to_string(collisions), Vector3(0,spacing*3,0), 16.0f, false);
	DrawText("Gravity: " + IsEnabled(gravity), Vector3(0,spacing*4,0), 16.0f, false);
	DrawText("Flocking modifiers:", Vector3(0,spacing*5,0), 16.0f, false);
	DrawText("Cohesion (UP / DOWN keys): " + to_string(cohesion), Vector3(0,spacing*6,0), 16.0f, false);
	DrawText("Alignment (LEFT / RIGHT keys): " + to_string(align), Vector3(0,spacing*7,0), 16.0f, false);
	DrawText("Seperation (8 / 9 keys): " + to_string(seperation), Vector3(0,spacing*8,0), 16.0f, false);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	glUseProgram(0);
}


void Renderer::RenderScene()	{
	DrawScene();
	PresentScene();
	SwapBuffers();
}

void	Renderer::DrawNode(SceneNode*n)	{
	if(n->GetMesh()) {
		glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "modelMatrix"),	1,false, (float*)&(n->GetWorldTransform()*Matrix4::Scale(n->GetModelScale())));
		Matrix4 temp = shadowMatrix * n->GetWorldTransform()*Matrix4::Scale(n->GetModelScale());
		glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "shadowMatrix"), 1, false, (float*)&temp);
		glUniform4fv(glGetUniformLocation(currentShader->GetProgram(), "colour"),1,(float*)&n->GetColour());

		n->Draw(*this);
	}
}

void	Renderer::BuildNodeLists(SceneNode* from)	{
	Vector3 direction = from->GetWorldTransform().GetPositionVector() - camera->GetPosition();
	from->SetCameraDistance(Vector3::Dot(direction,direction));

	if(frameFrustum.InsideFrustum(*from)) {
		if(from->GetColour().w < 1.0f) {
			transparentNodeList.push_back(from);
		}
		else{
			nodeList.push_back(from);
		}
	}

	for(vector<SceneNode*>::const_iterator i = from->GetChildIteratorStart(); i != from->GetChildIteratorEnd(); ++i) {
		BuildNodeLists((*i));
	}
}

void	Renderer::DrawNodes()	 {
	for(vector<SceneNode*>::const_iterator i = nodeList.begin(); i != nodeList.end(); ++i ) {
		DrawNode((*i));
	}

	for(vector<SceneNode*>::const_reverse_iterator i = transparentNodeList.rbegin(); i != transparentNodeList.rend(); ++i ) {
		DrawNode((*i));
	}
}

void	Renderer::SortNodeLists()	{
	std::sort(transparentNodeList.begin(),	transparentNodeList.end(),	SceneNode::CompareByCameraDistance);
	std::sort(nodeList.begin(),				nodeList.end(),				SceneNode::CompareByCameraDistance);
}

void	Renderer::ClearNodeLists()	{
	transparentNodeList.clear();
	nodeList.clear();
}

void	Renderer::SetCamera(Camera*c) {
	camera = c;
}

void	Renderer::AddNode(SceneNode* n) {
	root->AddChild(n);
}

void	Renderer::RemoveNode(SceneNode* n) {
	root->RemoveChild(n);
}

void Renderer::DrawText(const std::string &text, const Vector3 &position, const float size, const bool perspective) {
	TextMesh* txMesh = new TextMesh(text, *tahoma);

	if (perspective) {
		modelMatrix = Matrix4::Translation(position) * Matrix4::Scale(Vector3(size, size, 1));
		viewMatrix	= camera->BuildViewMatrix();
		projMatrix	= Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	}
	else {
		modelMatrix	= Matrix4::Translation(Vector3(position.x, height - position.y, position.z)) * Matrix4::Scale(Vector3(size,size,1));
		viewMatrix.ToIdentity();
		projMatrix = Matrix4::Orthographic(-1.0f,1.0f,(float)width, 0.0f,(float)height, 0.0f);
	}

	UpdateShaderMatrices();
	txMesh->Draw();
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	delete txMesh;
}

void Renderer::RotateLight() {
	Vector3 translate = Vector3(1000.0f, 1000.0f, 1000.0f);

	Matrix4 pushMatrix = Matrix4::Translation(translate);
	Matrix4 popMatrix = Matrix4::Translation(-translate);

	float radius = light->GetRadius();

	Matrix4 lightMatrix = pushMatrix * Matrix4::Rotation(0.5, Vector3(0, 1, 0)) * popMatrix *
							Matrix4::Translation(light->GetPosition()) *
							Matrix4::Scale(Vector3(radius, radius, radius));
	
	light->SetPosition(lightMatrix.GetPositionVector());
}

void Renderer::ToggleEffect(int arg) {
	switch (arg)
	{
	case (1):
		wireFrame = !wireFrame;
		break;
	case (2):
		gravity = !gravity;
		break;
	default:
		break;
	}
}

bool Renderer::ActiveTex() {
	activeTex = !activeTex;
	return activeTex;
}

void Renderer::UpdateDisplayValues(int vars[]) {
	FPS			= vars[0];
	physFPS		= vars[1];
	objects		= vars[2];
	collisions	= vars[3];
}

void Renderer::UpdateFlockingValues(float vars[]) {
	cohesion	= vars[0];
	align		= vars[1];
	seperation	= vars[2];
}


string Renderer::IsEnabled(bool arg) {
	if (arg)
		return "Enabled";

	return "Disabled";
}
