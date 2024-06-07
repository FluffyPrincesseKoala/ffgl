#include "Koala.h"
#include <math.h>//floor
using namespace ffglex;

enum ParamType : FFUInt32
{
	// koala sexy shaders
	PT_ITERATIONS,
	PT_FRACTALAMOUNT,
	PT_SPEED,
	PT_PALETTE_0_RED,
	PT_PALETTE_0_GREEN,
	PT_PALETTE_0_BLUE,
	PT_PALETTE_1_RED,
	PT_PALETTE_1_GREEN,
	PT_PALETTE_1_BLUE,
	PT_PALETTE_2_RED,
	PT_PALETTE_2_GREEN,
	PT_PALETTE_2_BLUE,
	PT_PALETTE_3_RED,
	PT_PALETTE_3_GREEN,
	PT_PALETTE_3_BLUE,
};

static CFFGLPluginInfo PluginInfo(
	PluginFactory< Koala >,                                  // Create method
	"KOA2",                                                  // Plugin unique ID
	"FractalCircleGenerator",                                // Plugin name
	2,                                                       // API major version number
	1,                                                       // API minor version number
	1,                                                       // Plugin major version number
	000,                                                     // Plugin minor version number
	FF_SOURCE,                                               // Plugin type
	"FractalCircleGenerator Sample FFGL 2d shader plugin",   // Plugin description
	"Resolume FFGL generator that create neon circle fractal"// About
);

static const char vertexShaderCode[] = R"(#version 410 core
layout( location = 0 ) in vec4 vPosition;
layout( location = 1 ) in vec2 vUV;

out vec2 uv;

void main()
{
	gl_Position = vPosition;
	uv = vUV;
}
)";

static const char fragmentShaderCode[] = R"(
#version 410 core

uniform float PALETTE_0_RED;
uniform float PALETTE_0_GREEN;
uniform float PALETTE_0_BLUE;
uniform float PALETTE_1_RED;
uniform float PALETTE_1_GREEN;
uniform float PALETTE_1_BLUE;
uniform float PALETTE_2_RED;
uniform float PALETTE_2_GREEN;
uniform float PALETTE_2_BLUE;
uniform float PALETTE_3_RED;
uniform float PALETTE_3_GREEN;
uniform float PALETTE_3_BLUE;

uniform float time;
uniform vec2 resolution;
uniform float iterations;

uniform float speed;

uniform float fractalAmount;

in vec2 uv;

out vec4 fragColor;

//https://iquilezles.org/articles/palettes/ << blog post
// http://dev.thi.ng/gradients/ << palettes value generator
vec3 palette(float t, vec3 a, vec3 b, vec3 c, vec3 d) {
    return a + b * cos(6.28318 * (c * t + d));
}

//https://www.shadertoy.com/view/mtyGWy
void main() {
    vec2 uvBase = uv * 2.0 - 1.0;
    vec2 uv0 = uvBase;
    vec3 finalColor = vec3(0.0);

	float fTime = time * (speed * 2.0);
	float fAmount = 1.0 + fractalAmount;

    for(float i = 0.0; i < iterations * 10.0; i++) {
        uvBase = fract(uvBase * fAmount) - 0.5;

        float d = length(uvBase) * exp(-length(uv0));

		// clamp to 0-1
		float pal0R = clamp(PALETTE_0_RED, 0.0, 1.0);
		float pal0G = clamp(PALETTE_0_GREEN, 0.0, 1.0);
		float pal0B = clamp(PALETTE_0_BLUE, 0.0, 1.0);

		float pal1R = clamp(PALETTE_1_RED, 0.0, 1.0);
		float pal1G = clamp(PALETTE_1_GREEN, 0.0, 1.0);
		float pal1B = clamp(PALETTE_1_BLUE, 0.0, 1.0);

		float pal2R = clamp(PALETTE_2_RED, 0.0, 1.0);
		float pal2G = clamp(PALETTE_2_GREEN, 0.0, 1.0);
		float pal2B = clamp(PALETTE_2_BLUE, 0.0, 1.0);

		float pal3R = clamp(PALETTE_3_RED, 0.0, 1.0);
		float pal3G = clamp(PALETTE_3_GREEN, 0.0, 1.0);
		float pal3B = clamp(PALETTE_3_BLUE, 0.0, 1.0);


        vec3 col = palette(
			length(uv0) + i * speed + fTime,
			vec3(pal0R, pal0G, pal0B),
			vec3(pal1R, pal1G, pal1B),
			vec3(pal2R, pal2G, pal2B),
			vec3(pal3R, pal3G, pal3B)
		);
		

        // d = sin(d * 8. + time) / 8.;
        d = sin(d * 8. + fTime) / 8.;
        d = abs(d);

        d = pow(0.01 / d, 1.2);

        finalColor += col * d;
    }

    fragColor = vec4(finalColor, 1.0);
}
)";

Koala::Koala() :
	iterations( 4.0f ),
	iterationsLocation( -1 ),
	fractalAmountLocation( -1 ),
	fractalAmount( 1.5f ),
	speedLocation( -1 ),
	speed( 0.4f ),
	palette0RedLocation( -1 ),
	palette0Red( 0.5f ),
	palette0GreenLocation( -1 ),
	palette0Green( 0.5f ),
	palette0BlueLocation( -1 ),
	palette0Blue( 0.5f ),
	palette1RedLocation( -1 ),
	palette1Red( 0.5f ),
	palette1GreenLocation( -1 ),
	palette1Green( 0.5f ),
	palette1BlueLocation( -1 ),
	palette1Blue( 0.5f ),
	palette2RedLocation( -1 ),
	palette2Red( 1.0f ),
	palette2GreenLocation( -1 ),
	palette2Green( 1.0f ),
	palette2BlueLocation( -1 ),
	palette2Blue( 1.0f ),
	palette3RedLocation( -1 ),
	palette3Red( 0.263f ),
	palette3GreenLocation( -1 ),
	palette3Green( 0.416f ),
	palette3BlueLocation( -1 ),
	palette3Blue( 0.557f )
{
	// Input properties
	SetMinInputs( 0 );
	SetMaxInputs( 0 );

	iterations = 10.0f;

	// Parameters
	SetParamInfof( PT_ITERATIONS, "Iterations", FF_TYPE_STANDARD );
	SetParamInfof( PT_FRACTALAMOUNT, "Fractal Amount", FF_TYPE_STANDARD );
	SetParamInfof( PT_SPEED, "Speed", FF_TYPE_STANDARD );
	SetParamInfof( PT_PALETTE_0_RED, "Palette 0 Red", FF_TYPE_RED );
	SetParamInfof( PT_PALETTE_0_GREEN, "Palette 0 Green", FF_TYPE_GREEN );
	SetParamInfof( PT_PALETTE_0_BLUE, "Palette 0 Blue", FF_TYPE_BLUE );
	SetParamInfof( PT_PALETTE_1_RED, "Palette 1 Red", FF_TYPE_RED );
	SetParamInfof( PT_PALETTE_1_GREEN, "Palette 1 Green", FF_TYPE_GREEN );
	SetParamInfof( PT_PALETTE_1_BLUE, "Palette 1 Blue", FF_TYPE_BLUE );
	SetParamInfof( PT_PALETTE_2_RED, "Palette 2 Red", FF_TYPE_RED );
	SetParamInfof( PT_PALETTE_2_GREEN, "Palette 2 Green", FF_TYPE_GREEN );
	SetParamInfof( PT_PALETTE_2_BLUE, "Palette 2 Blue", FF_TYPE_BLUE );
	SetParamInfof( PT_PALETTE_3_RED, "Palette 3 Red", FF_TYPE_RED );
	SetParamInfof( PT_PALETTE_3_GREEN, "Palette 3 Green", FF_TYPE_GREEN );
	SetParamInfof( PT_PALETTE_3_BLUE, "Palette 3 Blue", FF_TYPE_BLUE );

	FFGLLog::LogToHost( "Created Gradient generator" );
}
FFResult Koala::InitGL( const FFGLViewportStruct* vp )
{
	if( !shader.Compile( vertexShaderCode, fragmentShaderCode ) )
	{
		DeInitGL();
		return FF_FAIL;
	}
	if( !quad.Initialise() )
	{
		DeInitGL();
		return FF_FAIL;
	}

	//FFGL requires us to leave the context in a default state on return, so use this scoped binding to help us do that.
	ScopedShaderBinding shaderBinding( shader.GetGLID() );
	m_TimeLocation        = shader.FindUniform( "time" );
	m_ResolutionLocation  = shader.FindUniform( "resolution" );
	iterationsLocation    = shader.FindUniform( "iterations" );
	speedLocation         = shader.FindUniform( "speed" );
	fractalAmountLocation = shader.FindUniform( "fractalAmount" );

	// get palette locations
	palette0RedLocation   = shader.FindUniform( "PALETTE_0_RED" );
	palette0GreenLocation = shader.FindUniform( "PALETTE_0_GREEN" );
	palette0BlueLocation  = shader.FindUniform( "PALETTE_0_BLUE" );

	palette1RedLocation   = shader.FindUniform( "PALETTE_1_RED" );
	palette1GreenLocation = shader.FindUniform( "PALETTE_1_GREEN" );
	palette1BlueLocation  = shader.FindUniform( "PALETTE_1_BLUE" );

	palette2RedLocation   = shader.FindUniform( "PALETTE_2_RED" );
	palette2GreenLocation = shader.FindUniform( "PALETTE_2_GREEN" );
	palette2BlueLocation  = shader.FindUniform( "PALETTE_2_BLUE" );

	palette3RedLocation   = shader.FindUniform( "PALETTE_3_RED" );
	palette3GreenLocation = shader.FindUniform( "PALETTE_3_GREEN" );
	palette3BlueLocation  = shader.FindUniform( "PALETTE_3_BLUE" );

	//Use base-class init as success result so that it retains the viewport.
	return CFFGLPlugin::InitGL( vp );
}
float GetTimeInSeconds()
{
	return (float)clock() / CLOCKS_PER_SEC;
}
FFResult Koala::ProcessOpenGL( ProcessOpenGLStruct* pGL )
{
	//FFGL requires us to leave the context in a default state on return, so use this scoped binding to help us do that.
	ScopedShaderBinding shaderBinding( shader.GetGLID() );
	if( m_TimeLocation != -1 )
	{
		glUniform1f( m_TimeLocation, GetTimeInSeconds() );
	}
	if( m_ResolutionLocation != -1 )
	{
		glUniform2f( m_ResolutionLocation, pGL->inputTextures[ 0 ]->Width, pGL->inputTextures[ 0 ]->Height );
	}
	if( iterationsLocation != -1 )
	{
		glUniform1f( iterationsLocation, iterations );
	}
	if( fractalAmountLocation != -1 )
	{
		glUniform1f( fractalAmountLocation, fractalAmount );
	}
	if( speedLocation != -1 )
	{
		glUniform1f( speedLocation, speed );
	}

	// pallete
	if( palette0RedLocation != -1 )
	{
		glUniform1f( palette0RedLocation, palette0Red );
	}
	if( palette0GreenLocation != -1 )
	{
		glUniform1f( palette0GreenLocation, palette0Green );
	}
	if( palette0BlueLocation != -1 )
	{
		glUniform1f( palette0BlueLocation, palette0Blue );
	}

	if( palette1RedLocation != -1 )
	{
		glUniform1f( palette1RedLocation, palette0Red );
	}

	if( palette1GreenLocation != -1 )
	{
		glUniform1f( palette1GreenLocation, palette0Green );
	}

	if( palette1BlueLocation != -1 )
	{
		glUniform1f( palette1BlueLocation, palette0Blue );
	}

	if( palette2RedLocation != -1 )
	{
		glUniform1f( palette2RedLocation, palette0Red );
	}

	if( palette2GreenLocation != -1 )
	{
		glUniform1f( palette2GreenLocation, palette0Green );
	}
	if( palette2BlueLocation != -1 )
	{
		glUniform1f( palette2BlueLocation, palette0Blue );
	}
	if( palette3RedLocation != -1 )
	{
		glUniform1f( palette3RedLocation, palette0Red );
	}
	if( palette3GreenLocation != -1 )
	{
		glUniform1f( palette3GreenLocation, palette0Green );
	}
	if( palette3BlueLocation != -1 )
	{
		glUniform1f( palette3BlueLocation, palette0Blue );
	}

	quad.Draw();

	return FF_SUCCESS;
}
FFResult Koala::DeInitGL()
{
	shader.FreeGLResources();
	quad.Release();

	return FF_SUCCESS;
}

FFResult Koala::SetFloatParameter( unsigned int dwIndex, float value )
{
	switch( dwIndex )
	{
	case PT_ITERATIONS:
		iterations = value;
		break;
	case PT_FRACTALAMOUNT:
		fractalAmount = value;
		break;
	case PT_SPEED:
		speed = value;
		break;
	case PT_PALETTE_0_RED:
		palette0Red = value;
		break;
	case PT_PALETTE_0_GREEN:
		palette0Green = value;
		break;
	case PT_PALETTE_0_BLUE:
		palette0Blue = value;
		break;
	case PT_PALETTE_1_RED:
		palette1Red = value;
		break;
	case PT_PALETTE_1_GREEN:
		palette1Green = value;
		break;
	case PT_PALETTE_1_BLUE:
		palette1Blue = value;
		break;
	case PT_PALETTE_2_RED:
		palette2Red = value;
		break;
	case PT_PALETTE_2_GREEN:
		palette2Green = value;
		break;
	case PT_PALETTE_2_BLUE:
		palette2Blue = value;
		break;
	case PT_PALETTE_3_RED:
		palette3Red = value;
		break;
	case PT_PALETTE_3_GREEN:
		palette3Green = value;
		break;
	case PT_PALETTE_3_BLUE:
		palette3Blue = value;
		break;
	default:
		return FF_FAIL;
	}

	return FF_SUCCESS;
}

float Koala::GetFloatParameter( unsigned int index )
{
	switch( index )
	{
	case PT_ITERATIONS:
		return iterations;
	case PT_FRACTALAMOUNT:
		return fractalAmount;
	case PT_SPEED:
		return speed;
	case PT_PALETTE_0_RED:
		return palette0Red;
	case PT_PALETTE_0_GREEN:
		return palette0Green;
	case PT_PALETTE_0_BLUE:
		return palette0Blue;
	case PT_PALETTE_1_RED:
		return palette1Red;
	case PT_PALETTE_1_GREEN:
		return palette1Green;
	case PT_PALETTE_1_BLUE:
		return palette1Blue;
	case PT_PALETTE_2_RED:
		return palette2Red;
	case PT_PALETTE_2_GREEN:
		return palette2Green;
	case PT_PALETTE_2_BLUE:
		return palette2Blue;
	case PT_PALETTE_3_RED:
		return palette3Red;
	case PT_PALETTE_3_GREEN:
		return palette3Green;
	case PT_PALETTE_3_BLUE:
		return palette3Blue;
	}

	return 0.0f;
}
