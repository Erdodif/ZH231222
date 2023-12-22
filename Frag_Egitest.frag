#version 430

// pipeline-ból bejövõ per-fragment attribútumok
in vec3 vs_out_pos;
in vec3 vs_out_norm;
in vec2 vs_out_tex;

// kimenõ érték - a fragment színe
out vec4 fs_out_col;

// textúra mintavételezõ objektum
uniform sampler2D texImage;

uniform vec3 cameraPos;

// fenyforras tulajdonsagok
uniform vec4 lightPos = vec4( 0.0, 0.0, 0.0, 0.0);

uniform vec3 La = vec3(0.0, 0.0, 0.0 );
uniform vec3 Ld = vec3(1.0, 1.0, 1.0 );
uniform vec3 Ls = vec3(1.0, 1.0, 1.0 );

uniform float lightConstantAttenuation    = 1.0;
uniform float lightLinearAttenuation      = 0.0;
uniform float lightQuadraticAttenuation   = 0.0;

// anyag tulajdonsagok

uniform vec3 Ka = vec3( 1.0 );
uniform vec3 Kd = vec3( 1.0 );
uniform vec3 Ks = vec3( 1.0 );

uniform float Shininess = 1.0;

void main()
{
	vec3 normal = normalize( vs_out_norm );
	if ( !gl_FrontFacing )
		normal = -normal;

	vec3 ToLight;
	float LightDistance=0.0;
	
	if ( lightPos.w == 0 ) // directional light
	{
		ToLight	= lightPos.xyz;
	}
	else				  // point light
	{
		ToLight	= lightPos.xyz - vs_out_pos;
		LightDistance = length(ToLight);
	}

	ToLight = normalize(ToLight);
	
	float Attenuation = 1.0 / ( lightConstantAttenuation + lightLinearAttenuation * LightDistance + lightQuadraticAttenuation * LightDistance * LightDistance);
	
	vec3 Ambient = La * Ka;

	float DiffuseFactor = max(dot(ToLight,normal), 0.0) * Attenuation;
	vec3 Diffuse = DiffuseFactor * Ld * Kd;
	
	vec3 viewDir = normalize( cameraPos - vs_out_pos );
	vec3 reflectDir = reflect( -ToLight, normal );
	
	float SpecularFactor = pow(max( dot( viewDir, reflectDir) ,0.0), Shininess) * Attenuation;
	vec3 Specular = SpecularFactor*Ls*Ks;

	// normal vector debug:
	// fs_out_col = vec4( normal * 0.5 + 0.5, 1.0 );
	vec4 texColor = texture(texImage, vs_out_tex);
	if ( !gl_FrontFacing ) texColor.rgb = 1.0 - texColor.rgb;

	fs_out_col = vec4( Ambient+Diffuse+Specular, 1.0 ) * texColor;
}