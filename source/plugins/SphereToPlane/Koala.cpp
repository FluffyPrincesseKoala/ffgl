#include "Koala.h"
#include <math.h>//floor
using namespace ffglex;

enum ParamType : FFUInt32
{
	// koala sexy shaders
	PT_ITERATIONS,
	PT_FRACTALAMOUNT,
	PT_COLORPULSE,
	PT_SPEED,
	PT_REVERSE_TIME,
	PT_LINETHICKNESS,
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
	PT_ROT_X,
	PT_ROT_Y,
	PT_ROT_Z,
};

static CFFGLPluginInfo PluginInfo(
	PluginFactory< Koala >,                       // Create method
	"KOA4",                                       // Plugin unique ID
	"SphereToPlane",                              // Plugin name
	2,                                            // API major version number
	1,                                            // API minor version number
	1,                                            // Plugin major version number
	000,                                          // Plugin minor version number
	FF_SOURCE,                                    // Plugin type
	"SphereToPlane 3D shader plugin",             // Plugin description
	"Resolume FFGL 3d engine from fragment shader"// About
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

static const char fragmentShaderCode[] = R"(#version 410 core

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
uniform bool reverseTime;
uniform vec2 resolution;
uniform float iterations;

uniform float lineThickness;

uniform float speed;

uniform float fractalAmount;
uniform float colorPulse;

uniform float rotX;
uniform float rotY;
uniform float rotZ;

in vec2 uv;

out vec4 fragColor;
vec3 palette(float d, float p0, float p1, float p2, float p3, float p4, float p5) {
    return mix(vec3(p0, p1, p2), vec3(p3, p4, p5), d);
    // return mix(vec3(0.2, 0.7, 0.9), vec3(1., 0., 1.), d);
}

vec2 rotate(vec2 p, float a) {
    float c = cos(a);
    float s = sin(a);
    return p * mat2(c, s, -s, c);
}

float map(vec3 p) {
    for(int i = 0; i < 8; ++i) {
        float t = time * 0.2;
        p.xz = rotate(p.xz, t);
        p.xy = rotate(p.xy, t * 1.89);
        p.xz = abs(p.xz);
        p.xz -= .5;
    }
    return dot(sign(p), p) / 5.;
}

vec4 rm(vec3 ro, vec3 rd, float p0, float p1, float p2, float p3, float p4, float p5) {
    float t = 0.;
    vec3 col = vec3(0.);
    float d;
    for(float i = 0.; i < 64.; i++) {
        vec3 p = ro + rd * t;
        d = map(p) * .5;
        if(d < 0.02) {
            break;
        }
        if(d > 100.) {
            break;
        }
        //col+=vec3(0.6,0.8,0.8)/(400.*(d));
        col += palette(length(p) * .1, p0, p1, p2, p3, p4, p5) / (400. * (d));
        t += d;
    }
    return vec4(col, 1. / (d * 100.));
}

float mapValue(float value, float min, float max) {
	return min + (max - min) * value;
}

void main() {
    // vec2 uv = (fragCoord-(iResolution.xy/2.))/iResolution.x;
    vec2 uvBase = uv * 2.0 - 1.0;
	float roX = mapValue(rotX, -100.0, 100.0);
	float roY = mapValue(rotY, -100.0, 100.0);
	float roZ = mapValue(rotZ, -100.0, 100.0);
    vec3 ro = vec3(roX, roY, roZ);
	float startAngle = 0.0;
	float endAngle = 6.28318530718;
	float t = 0.5 * (1.0 + sin(time * speed));
	float angle = mix(startAngle, endAngle, t);
    ro.xz = rotate(ro.xz, angle);
    vec3 cf = normalize(-ro);
    vec3 cs = normalize(cross(cf, vec3(0., 1., 0.)));
    vec3 cu = normalize(cross(cf, cs));

    vec3 uuv = ro + cf * 3. + uvBase.x * cs + uvBase.y * cu;

    vec3 rd = normalize(uuv - ro);

	float p0 = clamp(PALETTE_0_RED, 0.0, 1.0);
	float p1 = clamp(PALETTE_0_GREEN, 0.0, 1.0);
	float p2 = clamp(PALETTE_0_BLUE, 0.0, 1.0);
	float p3 = clamp(PALETTE_1_RED, 0.0, 1.0);
	float p4 = clamp(PALETTE_1_GREEN, 0.0, 1.0);
	float p5 = clamp(PALETTE_1_BLUE, 0.0, 1.0);

    vec4 col = rm(ro, rd, p0, p1, p2, p3, p4, p5);

    fragColor = col;
}

/** SHADERDATA
{
	"title": "fractal pyramid",
	"description": "",
	"model": "car"
}
*/
)";

Koala::Koala() :
	iterations( 4.0f ),
	iterationsLocation( -1 ),
	fractalAmountLocation( -1 ),
	fractalAmount( 1.5f ),
	speedLocation( -1 ),
	colorPulseLocation( -1 ),
	colorPulse( 1.5f ),
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
	palette3Blue( 0.557f ),
	lineThicknessLocation( -1 ),
	lineThickness( 0.5f ),
	reverseTimeLocation( -1 ),
	reverseTime( false ),
	rotateXLocation( -1 ),
	rotateX( 0.0f ),
	rotateYLocation( -1 ),
	rotateY( 0.0f ),
	rotateZLocation( -1 ),
	rotateZ( 0.0f )
{
	// Input properties
	SetMinInputs( 0 );
	SetMaxInputs( 0 );

	iterations = 10.0f;

	// Parameters
	SetParamInfof( PT_ROT_X, "Rotate X", FF_TYPE_STANDARD );
	SetParamInfof( PT_ROT_Y, "Rotate Y", FF_TYPE_STANDARD );
	SetParamInfof( PT_ROT_Z, "Rotate Z", FF_TYPE_STANDARD );
	SetParamInfof( PT_ITERATIONS, "Iterations", FF_TYPE_STANDARD );
	SetParamInfof( PT_FRACTALAMOUNT, "Fractal Amount", FF_TYPE_STANDARD );
	SetParamInfof( PT_COLORPULSE, "Color Pulse", FF_TYPE_STANDARD );
	SetParamInfof( PT_SPEED, "Speed", FF_TYPE_STANDARD );
	SetParamInfof( PT_REVERSE_TIME, "Reverse Time", FF_TYPE_BOOLEAN );
	SetParamInfof( PT_LINETHICKNESS, "Line Thickness", FF_TYPE_STANDARD );
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
	reverseTimeLocation   = shader.FindUniform( "reverseTime" );
	m_ResolutionLocation  = shader.FindUniform( "resolution" );
	iterationsLocation    = shader.FindUniform( "iterations" );
	speedLocation         = shader.FindUniform( "speed" );
	fractalAmountLocation = shader.FindUniform( "fractalAmount" );
	colorPulseLocation    = shader.FindUniform( "colorPulse" );
	lineThicknessLocation = shader.FindUniform( "lineThickness" );

	// 3d rotation
	rotateXLocation = shader.FindUniform( "rotX" );
	rotateYLocation = shader.FindUniform( "rotY" );
	rotateZLocation = shader.FindUniform( "rotZ" );

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
	if( reverseTimeLocation != -1 )
	{
		glUniform1i( reverseTimeLocation, reverseTime );
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
	if( colorPulseLocation != -1 )
	{
		glUniform1f( colorPulseLocation, colorPulse );
	}
	if( speedLocation != -1 )
	{
		glUniform1f( speedLocation, speed );
	}
	if( lineThicknessLocation != -1 )
	{
		glUniform1f( lineThicknessLocation, lineThickness );
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

	// 3d rotation
	if( rotateXLocation != -1 )
	{
		glUniform1f( rotateXLocation, rotateX );
	}
	if( rotateYLocation != -1 )
	{
		glUniform1f( rotateYLocation, rotateY );
	}
	if( rotateZLocation != -1 )
	{
		glUniform1f( rotateZLocation, rotateZ );
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
	case PT_COLORPULSE:
		colorPulse = value;
		break;
	case PT_SPEED:
		speed = value;
		break;
	case PT_REVERSE_TIME:
		reverseTime = value;
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
	case PT_LINETHICKNESS:
		lineThickness = value;
		break;
	case PT_ROT_X:
		rotateX = value;
		break;
	case PT_ROT_Y:
		rotateY = value;
		break;
	case PT_ROT_Z:
		rotateZ = value;
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
	case PT_COLORPULSE:
		return colorPulse;
	case PT_SPEED:
		return speed;
	case PT_REVERSE_TIME:
		return reverseTime;
	case PT_LINETHICKNESS:
		return lineThickness;
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
	case PT_ROT_X:
		return rotateX;
	case PT_ROT_Y:
		return rotateY;
	case PT_ROT_Z:
		return rotateZ;
	}

	return 0.0f;
}
