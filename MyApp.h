#pragma once

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform2.hpp>

// GLEW
#include <GL/glew.h>

// SDL
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

// Utils
#include "GLUtils.hpp"
#include "Camera.h"

struct SUpdateInfo
{
	float ElapsedTimeInSec = 0.0f; // Program indulása óta eltelt idő
	float DeltaTimeInSec   = 0.0f; // Előző Update óta eltelt idő
};

class CMyApp
{
public:
	CMyApp();
	~CMyApp();

	bool Init();
	void Clean();

	void Update( const SUpdateInfo& );
	void Render();
	void RenderGUI();

	void KeyboardDown(const SDL_KeyboardEvent&);
	void KeyboardUp(const SDL_KeyboardEvent&);
	void MouseMove(const SDL_MouseMotionEvent&);
	void MouseDown(const SDL_MouseButtonEvent&);
	void MouseUp(const SDL_MouseButtonEvent&);
	void MouseWheel(const SDL_MouseWheelEvent&);
	void Resize(int, int);
protected:
	void SetupDebugCallback();

	//
	// Adat változók
	//

	float m_ElapsedTimeInSec = 0.0f;

	// Kamera
	Camera m_camera;

	//
	// OpenGL-es dolgok
	//
	
	// uniform location lekérdezése
	GLint ul( const char* uniformName ) noexcept;

	// shaderekhez szükséges változók
	GLuint m_programID = 0;		  // shaderek programja
	GLuint m_programSunID = 0;	  // a nap shader programja
	GLuint m_programSkyboxID = 0; // skybox programja


	// Fényforrás- ...
	glm::vec4 m_lightPos = glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );

	glm::vec3 m_La = glm::vec3(0.0, 0.0, 0.0 );
	glm::vec3 m_Ld = glm::vec3(0.85, 0.8, 0.8);
	glm::vec3 m_Ls = glm::vec3(1.0, 1.0, 1.0 );

	float m_lightConstantAttenuation    = 1.0;
	float m_lightLinearAttenuation      = 0.0;
	float m_lightQuadraticAttenuation   = 0.0;

	// ... és anyagjellemzők
	glm::vec3 m_Ka = glm::vec3( 0.0125 );
	glm::vec3 m_Kd = glm::vec3( 0.85,0.8,0.8 );
	glm::vec3 m_Ks = glm::vec3( 0.9 );

	float m_Shininess = 36.0;

	// Shaderek inicializálása, és törtlése
	void InitShaders();
	void CleanShaders();
	void InitSkyboxShaders();
	void CleanSkyboxShaders();

	// Geometriával kapcsolatos változók

	OGLObject m_JonsonGPU = {};
	OGLObject m_bolygoSurfaceGPU = {};
	OGLObject m_SkyboxGPU = {};
	OGLObject m_BeltGPU = {};

	// Geometria inicializálása, és törtlése
	void InitGeometry();
	void CleanGeometry();
	void InitSkyboxGeometry();
	void CleanSkyboxGeometry();

	// Textúrázás, és változói

	GLuint m_surfaceTextureID = 0;
	GLuint m_skyboxTextureID = 0;

	void InitTextures();
	void CleanTextures();
	void InitSkyboxTextures();
	void CleanSkyboxTextures();

	// Saját definíciók

	// Uniformok leküldése
	void SendWorld(glm::mat4 matWorld);
	void SendPreset();

	// Segédfüggvények
	glm::mat4 GetOrbitPosition(glm::mat4 center, float radius, float angle);


	// Render parancsok kiszervezése
	void RenderAsteroid(glm::mat4 matWorld);
	void RenderPlanet(
		glm::mat4 matWorld, 
		GLuint textureId, 
		float size, 
		float orbitSpeed, 
		float distance, 
		float tiltAngle, 
		float rotationSpeed,
		GLuint programID);
	void RenderSkyBox();

	void RenderBelt(
		glm::mat4 matWorld,
		GLuint textureId,
		float size);

	// Változók

	float m_simulationSpeed = 1.0f;
	float m_ElapsedTimeSim = 0.0f;

	// Textúrák
	GLuint mb_sun_TextureID = 0;
	GLuint mb_mercury_TextureID = 0;
	GLuint mb_venus_TextureID = 0;
	GLuint mb_earth_TextureID = 0;
	GLuint mb_moon_TextureID = 0;
	GLuint mb_mars_TextureID = 0;
	GLuint mb_jupiter_TextureID = 0;
	GLuint mb_saturn_TextureID = 0;
	GLuint mb_uranus_TextureID = 0;
	GLuint mb_neptune_TextureID = 0;
	GLuint mb_pluto_TextureID = 0;

	GLuint mb_belt_kuiper_TextureID = 0;
	GLuint mb_belt_saturn_TextureID = 0;
	GLuint mb_belt_uranus_TextureID = 0;
	GLuint mb_belt_neptune_TextureID = 0;

	GLuint mb_rock_TextureID = 0;
};

