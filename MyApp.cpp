#include "MyApp.h"
#include "SDL_GLDebugMessageCallback.h"
#include "ObjParser.h"
#include "ParametricSurfaceMesh.hpp"

#include <imgui.h>

CMyApp::CMyApp()
{
}

CMyApp::~CMyApp()
{
}

void CMyApp::SetupDebugCallback()
{
	// engedélyezzük és állítsuk be a debug callback függvényt ha debug context-ben vagyunk 
	GLint context_flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
	if (context_flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
		glDebugMessageCallback(SDL_GLDebugMessageCallback, nullptr);
	}
}

void CMyApp::InitShaders()
{
	m_programID = glCreateProgram();
	//AssembleProgram( m_programID, "Vert_PosNormTex.vert", "Frag_ZH.frag" );
	AssembleProgram(m_programID, "Vert_PosNormTex.vert", "Frag_Egitest.frag");
	
	m_programSunID = glCreateProgram();
	AssembleProgram(m_programSunID, "Vert_PosNormTex.vert", "Frag_ZH.frag");

	InitSkyboxShaders();
}

void CMyApp::InitSkyboxShaders()
{
	m_programSkyboxID = glCreateProgram();
	AssembleProgram( m_programSkyboxID, "Vert_skybox.vert", "Frag_skybox.frag" );
}

void CMyApp::CleanShaders()
{
	glDeleteProgram( m_programID );
	glDeleteProgram( m_programSunID );
	CleanSkyboxShaders();
}

void CMyApp::CleanSkyboxShaders()
{
	glDeleteProgram( m_programSkyboxID );
}

// Nyers parameterek
struct Sphere
{
	float r;
	Sphere(float _r = 1.0f) : r(_r) {}

	glm::vec3 GetPos(float u, float v) const noexcept
	{
		u *= glm::two_pi<float>();
		v *= glm::pi<float>();


		return glm::vec3(
			r * sinf(v) * cosf(u),
			r * cosf(v),
			r * sinf(v) * sinf(u));
	}

	glm::vec3 GetNorm(float u, float v) const noexcept
	{
		return GetPos(u, v);
	}
	glm::vec2 GetTex(float u, float v) const noexcept
	{
		return glm::vec2(u, v);
	}
};

void CMyApp::InitGeometry()
{

	const std::initializer_list<VertexAttributeDescriptor> vertexAttribList =
	{
		{ 0, offsetof( Vertex, position ), 3, GL_FLOAT },
		{ 1, offsetof( Vertex, normal   ), 3, GL_FLOAT },
		{ 2, offsetof( Vertex, texcoord ), 2, GL_FLOAT },
	};

	// Rock

	MeshObject<Vertex> jonsonMeshCPU = ObjParser::parse("Assets/asteroid.obj");
	m_JonsonGPU = CreateGLObjectFromMesh( jonsonMeshCPU, vertexAttribList );

	// Parametrikus felület
	MeshObject<Vertex> bolygoGombMeshCPU = GetParamSurfMesh( Sphere() );
	m_bolygoSurfaceGPU = CreateGLObjectFromMesh( bolygoGombMeshCPU, vertexAttribList );

	// Belt

	MeshObject<Vertex> beltMeshCPU;
	beltMeshCPU.vertexArray =
	{
		{ glm::vec3(-1, 0,-1),glm::vec3(0.0, 1.0, 0.0), glm::vec2(0.0, 0.0) },
		{ glm::vec3(1, 0,-1),glm::vec3(0.0, 1.0, 0.0), glm::vec2(1.0, 0.0) },
		{ glm::vec3(-1, 0, 1),glm::vec3(0.0, 1.0, 0.0), glm::vec2(0.0, 1.0) },
		{ glm::vec3(1,  0, 1),glm::vec3(0.0, 1.0, 0.0), glm::vec2(1.0, 1.0) }
	};

	beltMeshCPU.indexArray =
	{
		0, 1, 2,
		1, 3, 2
	};

	m_BeltGPU = CreateGLObjectFromMesh(beltMeshCPU, vertexAttribList);

	// Skybox
	InitSkyboxGeometry();
}

void CMyApp::CleanGeometry()
{
	CleanOGLObject( m_JonsonGPU );
	CleanOGLObject( m_bolygoSurfaceGPU );
	CleanSkyboxGeometry();
}

void CMyApp::InitSkyboxGeometry()
{
	// skybox geo
	MeshObject<glm::vec3> skyboxCPU =
	{
		std::vector<glm::vec3>
		{
			// hátsó lap
			glm::vec3(-1, -1, -1),
			glm::vec3( 1, -1, -1),
			glm::vec3( 1,  1, -1),
			glm::vec3(-1,  1, -1),
			// elülső lap
			glm::vec3(-1, -1, 1),
			glm::vec3( 1, -1, 1),
			glm::vec3( 1,  1, 1),
			glm::vec3(-1,  1, 1),
		},

		std::vector<GLuint>
		{
			// hátsó lap
			0, 1, 2,
			2, 3, 0,
			// elülső lap
			4, 6, 5,
			6, 4, 7,
			// bal
			0, 3, 4,
			4, 3, 7,
			// jobb
			1, 5, 2,
			5, 6, 2,
			// alsó
			1, 0, 4,
			1, 4, 5,
			// felső
			3, 2, 6,
			3, 6, 7,
		}
	};

	m_SkyboxGPU = CreateGLObjectFromMesh( skyboxCPU, { { 0, offsetof( glm::vec3,x ), 3, GL_FLOAT } } );
}

void CMyApp::CleanSkyboxGeometry()
{
	CleanOGLObject( m_SkyboxGPU );
}

void CMyApp::InitTextures()
{
	// diffuse textures

	glGenTextures( 1, &m_surfaceTextureID );
	TextureFromFile( m_surfaceTextureID, "Assets/color_checkerboard.png" );
	SetupTextureSampling( GL_TEXTURE_2D, m_surfaceTextureID );

	// skybox texture

	InitSkyboxTextures();

	// Bolygó textúrák

	// sun
	glGenTextures(1, &mb_sun_TextureID);
	TextureFromFile(mb_sun_TextureID, "Assets/sun.jpg");
	SetupTextureSampling(GL_TEXTURE_2D, mb_sun_TextureID);
	// mercury
	glGenTextures(1, &mb_mercury_TextureID);
	TextureFromFile(mb_mercury_TextureID, "Assets/mercury.jpg");
	SetupTextureSampling(GL_TEXTURE_2D, mb_mercury_TextureID);
	// venus
	glGenTextures(1, &mb_venus_TextureID);
	TextureFromFile(mb_venus_TextureID, "Assets/venus.jpg");
	SetupTextureSampling(GL_TEXTURE_2D, mb_venus_TextureID);
	// earth
	glGenTextures(1, &mb_earth_TextureID);
	TextureFromFile(mb_earth_TextureID, "Assets/earth.png");
	SetupTextureSampling(GL_TEXTURE_2D, mb_earth_TextureID);
	// moon
	glGenTextures(1, &mb_moon_TextureID);
	TextureFromFile(mb_moon_TextureID, "Assets/moon.jpg");
	SetupTextureSampling(GL_TEXTURE_2D, mb_moon_TextureID);
	// mars
	glGenTextures(1, &mb_mars_TextureID);
	TextureFromFile(mb_mars_TextureID, "Assets/mars.jpg");
	SetupTextureSampling(GL_TEXTURE_2D, mb_mars_TextureID);
	// jupiter
	glGenTextures(1, &mb_jupiter_TextureID);
	TextureFromFile(mb_jupiter_TextureID, "Assets/jupiter.jpg");
	SetupTextureSampling(GL_TEXTURE_2D, mb_jupiter_TextureID);
	// saturn
	glGenTextures(1, &mb_saturn_TextureID);
	TextureFromFile(mb_saturn_TextureID, "Assets/saturn.jpg");
	SetupTextureSampling(GL_TEXTURE_2D, mb_saturn_TextureID);
	// uranus
	glGenTextures(1, &mb_uranus_TextureID);
	TextureFromFile(mb_uranus_TextureID, "Assets/uranus.jpg");
	SetupTextureSampling(GL_TEXTURE_2D, mb_uranus_TextureID);
	// neptune
	glGenTextures(1, &mb_neptune_TextureID);
	TextureFromFile(mb_neptune_TextureID, "Assets/neptune.jpg");
	SetupTextureSampling(GL_TEXTURE_2D, mb_neptune_TextureID);
	// pluto
	glGenTextures(1, &mb_pluto_TextureID);
	TextureFromFile(mb_pluto_TextureID, "Assets/pluto.jpg");
	SetupTextureSampling(GL_TEXTURE_2D, mb_pluto_TextureID);
	// belt - kuiper
	glGenTextures(1, &mb_belt_kuiper_TextureID);
	TextureFromFile(mb_belt_kuiper_TextureID, "Assets/kuiper.png");
	SetupTextureSampling(GL_TEXTURE_2D, mb_belt_kuiper_TextureID);
	// belt - saturn
	glGenTextures(1, &mb_belt_saturn_TextureID);
	TextureFromFile(mb_belt_saturn_TextureID, "Assets/saturn-ring.png");
	SetupTextureSampling(GL_TEXTURE_2D, mb_belt_saturn_TextureID);
	// belt - uranus
	glGenTextures(1, &mb_belt_uranus_TextureID);
	TextureFromFile(mb_belt_uranus_TextureID, "Assets/uranus-ring.png");
	SetupTextureSampling(GL_TEXTURE_2D, mb_belt_uranus_TextureID);
	// belt - neptune
	glGenTextures(1, &mb_belt_neptune_TextureID);
	TextureFromFile(mb_belt_neptune_TextureID, "Assets/neptune-ring.png");
	SetupTextureSampling(GL_TEXTURE_2D, mb_belt_neptune_TextureID);
	// Dwayne Jonson
	glGenTextures(1, &mb_rock_TextureID);
	TextureFromFile(mb_rock_TextureID, "Assets/rock.jpg");
	SetupTextureSampling(GL_TEXTURE_2D, mb_rock_TextureID);


}

void CMyApp::CleanTextures()
{
	// diffuse textures

	glDeleteTextures( 1, &mb_rock_TextureID );

	glDeleteTextures(1, &mb_sun_TextureID);
	glDeleteTextures(1, &mb_mercury_TextureID);
	glDeleteTextures(1, &mb_venus_TextureID);
	glDeleteTextures(1, &mb_earth_TextureID);
	glDeleteTextures(1, &mb_moon_TextureID);
	glDeleteTextures(1, &mb_mars_TextureID);
	glDeleteTextures(1, &mb_jupiter_TextureID);
	glDeleteTextures(1, &mb_saturn_TextureID);
	glDeleteTextures(1, &mb_uranus_TextureID);
	glDeleteTextures(1, &mb_neptune_TextureID);
	glDeleteTextures(1, &mb_pluto_TextureID);

	glDeleteTextures(1, &mb_belt_kuiper_TextureID);
	glDeleteTextures(1, &mb_belt_saturn_TextureID);
	glDeleteTextures(1, &mb_belt_neptune_TextureID);
	glDeleteTextures(1, &mb_belt_uranus_TextureID);


	glDeleteTextures( 1, &m_surfaceTextureID );

	// skybox texture

	CleanSkyboxTextures();
}

void CMyApp::InitSkyboxTextures()
{
	// skybox texture

	glGenTextures( 1, &m_skyboxTextureID );
	TextureFromFile( m_skyboxTextureID, "Assets/space_xpos.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_POSITIVE_X );
	TextureFromFile( m_skyboxTextureID, "Assets/space_xneg.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_NEGATIVE_X );
	TextureFromFile( m_skyboxTextureID, "Assets/space_ypos.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_POSITIVE_Y );
	TextureFromFile( m_skyboxTextureID, "Assets/space_yneg.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y );
	TextureFromFile( m_skyboxTextureID, "Assets/space_zpos.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_POSITIVE_Z );
	TextureFromFile( m_skyboxTextureID, "Assets/space_zneg.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z );
	SetupTextureSampling( GL_TEXTURE_CUBE_MAP, m_skyboxTextureID, false );

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void CMyApp::CleanSkyboxTextures()
{
	glDeleteTextures( 1, &m_skyboxTextureID );
}

bool CMyApp::Init()
{
	SetupDebugCallback();

	// törlési szín legyen kékes
	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

	InitShaders();
	InitGeometry();
	InitTextures();

	//
	// egyéb inicializálás
	//

	glEnable(GL_CULL_FACE); // kapcsoljuk be a hátrafelé néző lapok eldobását
	glCullFace(GL_BACK);    // GL_BACK: a kamerától "elfelé" néző lapok, GL_FRONT: a kamera felé néző lapok

	glEnable(GL_DEPTH_TEST); // mélységi teszt bekapcsolása (takarás)

	// kamera
	m_camera.SetView(
		glm::vec3(0.0, 7.0, 7.0),// honnan nézzük a színteret	   - eye
		glm::vec3(0.0, 0.0, 0.0),   // a színtér melyik pontját nézzük - at
		glm::vec3(0.0, 1.0, 0.0));  // felfelé mutató irány a világban - up


	return true;
}

void CMyApp::Clean()
{
	CleanShaders();
	CleanGeometry();
	CleanTextures();
}

void CMyApp::Update( const SUpdateInfo& updateInfo )
{
	m_ElapsedTimeInSec = updateInfo.ElapsedTimeInSec;
	m_ElapsedTimeSim += updateInfo.DeltaTimeInSec * (1.0f / m_simulationSpeed);

	m_camera.Update( updateInfo.DeltaTimeInSec );
}

// <Saját dolgok>
void CMyApp::SendWorld(glm::mat4 matWorld) {
	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(matWorld));
	glUniformMatrix4fv(ul("worldIT"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(matWorld))));
}

glm::mat4 CMyApp::GetOrbitPosition(glm::mat4 center, float radius, float angle) {
	return center 
		* glm::rotate(angle ,glm::vec3(0.0,1.0,0.0)) 
		* glm::translate(glm::vec3(radius, 0.0, 0.0)) 
		* glm::rotate(-angle , glm::vec3(0.0, 1.0, 0.0));
}

void CMyApp::SendPreset() {

	// - textúraegységek beállítása
	glUniform1i(ul("texImage"), 0);

	glUniformMatrix4fv(ul("viewProj"), 1, GL_FALSE, glm::value_ptr(m_camera.GetViewProj()));

	// - Fényforrások beállítása
	glUniform3fv(ul("cameraPos"), 1, glm::value_ptr(m_camera.GetEye()));
	glUniform4fv(ul("lightPos"), 1, glm::value_ptr(m_lightPos));

	glUniform3fv(ul("La"), 1, glm::value_ptr(m_La));
	glUniform3fv(ul("Ld"), 1, glm::value_ptr(m_Ld));
	glUniform3fv(ul("Ls"), 1, glm::value_ptr(m_Ls));

	glUniform1f(ul("lightConstantAttenuation"), m_lightConstantAttenuation);
	glUniform1f(ul("lightLinearAttenuation"), m_lightLinearAttenuation);
	glUniform1f(ul("lightQuadraticAttenuation"), m_lightQuadraticAttenuation);

	// - Anyagjellemzők beállítása
	glUniform3fv(ul("Ka"), 1, glm::value_ptr(m_Ka));
	glUniform3fv(ul("Kd"), 1, glm::value_ptr(m_Kd));
	glUniform3fv(ul("Ks"), 1, glm::value_ptr(m_Ks));

	glUniform1f(ul("Shininess"), m_Shininess);

}

void CMyApp::RenderAsteroid(glm::mat4 matWorld) {
	// Suzanne - soon // Asteroid

	glBindVertexArray(m_JonsonGPU.vaoID);

	// - Textúrák beállítása, minden egységre külön
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mb_rock_TextureID);

	SendWorld(matWorld);

	// - textúraegységek beállítása
	//glUniform1i(ul("texImage"), 0);

	glDrawElements(GL_TRIANGLES,
		m_JonsonGPU.count,
		GL_UNSIGNED_INT,
		nullptr);
}

void CMyApp::RenderPlanet(
		glm::mat4 matWorld, 
		GLuint textureId, 
		float size = 1.0, 
		float orbitDays = 1.0, 
		float distance = 1.0, 
		float tiltAngle = 15.0, 
		float rotationDays = 2.0,
		GLuint programID = 0
	) {
	if (programID == 0) {
		programID = m_programID;
	}
	if (rotationDays < 0.001 && orbitDays < 0.001) {
		matWorld = matWorld
			* glm::scale(glm::vec3(size, size, size))
			* glm::rotate(tiltAngle, glm::vec3(1.0, 0.0, 1.0));
	}
	else if (rotationDays <= 0.001) {
		matWorld = GetOrbitPosition(matWorld, distance, (float)(glm::two_pi<float>() / orbitDays) * m_ElapsedTimeSim)
			* glm::scale(glm::vec3(size, size, size))
			* glm::rotate(tiltAngle, glm::vec3(1.0, 0.0, 1.0));
	}
	else if (orbitDays < 0.001)
	{
		matWorld = matWorld
			* glm::scale(glm::vec3(size, size, size))
			* glm::rotate(tiltAngle, glm::vec3(1.0, 0.0, 1.0))
			* glm::rotate((glm::two_pi<float>() / rotationDays * m_ElapsedTimeSim), glm::vec3(0.0, 1.0, 0.0));
	}
	else {
		matWorld = GetOrbitPosition(matWorld, distance, (float)(glm::two_pi<float>() / orbitDays) * m_ElapsedTimeSim)
			* glm::scale(glm::vec3(size, size, size))
			* glm::rotate(tiltAngle, glm::vec3(1.0, 0.0, 1.0))
			* glm::rotate((glm::two_pi<float>() / rotationDays * m_ElapsedTimeSim), glm::vec3(0.0,1.0,0.0));
	}

	glUseProgram(programID);
	SendPreset();
	SendWorld(matWorld);

	glBindVertexArray(m_bolygoSurfaceGPU.vaoID);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureId);


	glDrawElements(GL_TRIANGLES,
		m_bolygoSurfaceGPU.count,
		GL_UNSIGNED_INT,
		nullptr);
}


void CMyApp::RenderBelt(
	glm::mat4 matWorld,
	GLuint textureId,
	float size = 1.0) {

	glUseProgram(m_programID);
	SendPreset();

	matWorld = matWorld * scale(glm::vec3(size));

	SendWorld(matWorld);

	glBindVertexArray(m_BeltGPU.vaoID);

	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	// meghatározza, hogy az átlátszó objektum az adott pixelben hogyan módosítsa a korábbi fragmentekből oda lerakott színt: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glBlendFunc.xhtml
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureId);


	glDrawElements(GL_TRIANGLES,
		m_BeltGPU.count,
		GL_UNSIGNED_INT,
		nullptr);
	glEnable(GL_CULL_FACE);
}

void CMyApp::RenderSkyBox() {
	// - VAO
	glBindVertexArray(m_SkyboxGPU.vaoID);

	// - Textura
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTextureID);

	// - Program
	glUseProgram(m_programSkyboxID);

	// - uniform parameterek
	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(glm::translate(m_camera.GetEye())));
	glUniformMatrix4fv(ul("viewProj"), 1, GL_FALSE, glm::value_ptr(m_camera.GetViewProj()));

	// - textúraegységek beállítása
	glUniform1i(ul("skyboxTexture"), 1);

	// mentsük el az előző Z-test eredményt, azaz azt a relációt, ami alapján update-eljük a pixelt.
	GLint prevDepthFnc;
	glGetIntegerv(GL_DEPTH_FUNC, &prevDepthFnc);

	// most kisebb-egyenlőt használjunk, mert mindent kitolunk a távoli vágósíkokra
	glDepthFunc(GL_LEQUAL);

	// - Rajzolas
	glDrawElements(GL_TRIANGLES, m_SkyboxGPU.count, GL_UNSIGNED_INT, nullptr);

	glDepthFunc(prevDepthFnc);

	// shader kikapcsolasa
	glUseProgram(0);

	// - Textúrák kikapcsolása, minden egységre külön
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	// VAO kikapcsolása
	glBindVertexArray(0);
}
// </Saját dolgok>

void CMyApp::Render()
{
	// töröljük a frampuffert (GL_COLOR_BUFFER_BIT)...
	// ... és a mélységi Z puffert (GL_DEPTH_BUFFER_BIT)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(m_programID);

	glm::mat4 matWorld = glm::identity<glm::mat4>();

	SendPreset();
	SendWorld(matWorld);

	matWorld = GetOrbitPosition(matWorld
		,4, (float)(glm::two_pi<float>() / 110.0) * m_ElapsedTimeSim)
		* glm::scale(glm::vec3(0.1, 0.1, 0.1))
		* glm::rotate(glm::two_pi<float>() / (6.0f * m_ElapsedTimeSim),glm::vec3(1.0,1.0,1.0));

	RenderAsteroid(matWorld);

	// Parametrikus felület

	matWorld = glm::identity<glm::mat4>();

	RenderPlanet(matWorld, mb_sun_TextureID,1,
		0,0,7.25,27.0, m_programSunID);

	RenderPlanet(matWorld, mb_mercury_TextureID,0.15,
		89.0,1.5, 0.0, 58.7);

	RenderPlanet(matWorld, mb_venus_TextureID,0.13,
		255.0,2.5, 177.4,243.0);

	float EarthSpeed = 365.0;
	float EarthSize = 0.2;
	float EarthOrbitDistace = 3.5;
	glUniform1f(ul("Shininess"), 85.0);
	RenderPlanet(
		matWorld, 
		mb_earth_TextureID,
		EarthSize, EarthSpeed, EarthOrbitDistace , 
		23.44 + 180.0, 1.0);
	glUniform1f(ul("Shininess"), m_Shininess);
	RenderPlanet(
		GetOrbitPosition(matWorld,EarthOrbitDistace, (float)(glm::two_pi<float>() / EarthSpeed) * m_ElapsedTimeSim),
		mb_moon_TextureID,
		EarthSize * (1.0 / 3.0), EarthSpeed / 12.0 , (EarthSize * 0.5) + 0.2,
		1.54, EarthSpeed +3 );

	RenderPlanet(matWorld, mb_mars_TextureID, 0.19, 
		687.0, 4.5, 25.19,1.04);

	RenderPlanet(matWorld, mb_jupiter_TextureID, 0.4, 
		4329.0, 5.5, 3.13,0.42);

	RenderPlanet(matWorld, mb_saturn_TextureID, 0.35, 
		10753.0, 6.5, 26.73,0.46);

	RenderPlanet(matWorld, mb_uranus_TextureID, 0.26, 
		30664.0, 7.5, 97.77,0.71);

	RenderPlanet(matWorld, mb_neptune_TextureID, 0.26, 
		60148.0, 8.5, 28.32,0.66 );

	RenderPlanet(matWorld, mb_pluto_TextureID, 0.1, 
		90520.0, 9.5, 119.61, 6.37);
	
	//RenderAsteroid(matWorld);

	//RenderBelt(matWorld, mb_belt_kuiper_TextureID, 30);

	RenderSkyBox();
}

void CMyApp::RenderGUI()
{
	if (ImGui::Begin("Beállítások"))
	{
		ImGui::SeparatorText("Szimuláció");

		static float simulationSpeed = 1.0;
		ImGui::SliderFloat("Földi nap ideje", &m_simulationSpeed, 0.25f, 10.0f);

	}

	ImGui::End();
}

GLint CMyApp::ul( const char* uniformName ) noexcept
{
	GLuint programID = 0;

	// Kérdezzük le az aktuális programot!
	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGet.xhtml
	glGetIntegerv( GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>( &programID ) );
	// A program és a uniform név ismeretében kérdezzük le a location-t!
	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetUniformLocation.xhtml
	return glGetUniformLocation( programID, uniformName );
}

// https://wiki.libsdl.org/SDL2/SDL_KeyboardEvent
// https://wiki.libsdl.org/SDL2/SDL_Keysym
// https://wiki.libsdl.org/SDL2/SDL_Keycode
// https://wiki.libsdl.org/SDL2/SDL_Keymod

void CMyApp::KeyboardDown(const SDL_KeyboardEvent& key)
{	
	if ( key.repeat == 0 ) // Először lett megnyomva
	{
		if ( key.keysym.sym == SDLK_F5 && key.keysym.mod & KMOD_CTRL )
		{
			CleanShaders();
			InitShaders();
		}
		if ( key.keysym.sym == SDLK_F1 )
		{
			GLint polygonModeFrontAndBack[ 2 ] = {};
			// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGet.xhtml
			glGetIntegerv( GL_POLYGON_MODE, polygonModeFrontAndBack ); // Kérdezzük le a jelenlegi polygon módot! Külön adja a front és back módokat.
			GLenum polygonMode = ( polygonModeFrontAndBack[ 0 ] != GL_FILL ? GL_FILL : GL_LINE ); // Váltogassuk FILL és LINE között!
			// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glPolygonMode.xhtml
			glPolygonMode( GL_FRONT_AND_BACK, polygonMode ); // Állítsuk be az újat!
		}
	}
	m_camera.KeyboardDown( key );
}

void CMyApp::KeyboardUp(const SDL_KeyboardEvent& key)
{
	m_camera.KeyboardUp( key );
}

// https://wiki.libsdl.org/SDL2/SDL_MouseMotionEvent

void CMyApp::MouseMove(const SDL_MouseMotionEvent& mouse)
{
	m_camera.MouseMove( mouse );
}

// https://wiki.libsdl.org/SDL2/SDL_MouseButtonEvent

void CMyApp::MouseDown(const SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseUp(const SDL_MouseButtonEvent& mouse)
{
}

// https://wiki.libsdl.org/SDL2/SDL_MouseWheelEvent

void CMyApp::MouseWheel(const SDL_MouseWheelEvent& wheel)
{
	m_camera.MouseWheel( wheel );
}


// a két paraméterben az új ablakméret szélessége (_w) és magassága (_h) található
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);
	m_camera.Resize( _w, _h );
}

