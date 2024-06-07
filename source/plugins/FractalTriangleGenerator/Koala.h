#pragma once
#include <FFGLSDK.h>

class Koala : public CFFGLPlugin
{
public:
	Koala();

	//CFFGLPlugin
	FFResult InitGL( const FFGLViewportStruct* vp ) override;
	FFResult ProcessOpenGL( ProcessOpenGLStruct* pGL ) override;
	FFResult DeInitGL() override;

	FFResult SetFloatParameter( unsigned int dwIndex, float value ) override;

	float GetFloatParameter( unsigned int index ) override;

	FFUInt32 m_TimeLocation;
	FFUInt32 m_ResolutionLocation;
	float iterations = 0.4f;
	GLint iterationsLocation;
	GLint shapeLocation;
	float shape = 0.4f;
	GLint fractalAmountLocation;
	float fractalAmount = 1.5f;
	GLint distortionAmountLocation;
	float distortionAmount = 0.0f;
	GLint displacementAmountLocation;
	float displacementAmount = 0.0f;
	GLint colorPulseLocation;
	float colorPulse = 1.5f;

	GLint uvOffsetXLocation;
	float uvOffsetX = 0.0f;
	GLint uvOffsetYLocation;
	float uvOffsetY = 0.0f;

	GLint lineThicknessLocation;
	float lineThickness = 0.5f;

	//palette colors
	GLint palette0RedLocation;
	float palette0Red = 0.5f;
	GLint palette0GreenLocation;
	float palette0Green = 0.5f;
	GLint palette0BlueLocation;
	float palette0Blue = 0.5f;
	GLint palette1RedLocation;
	float palette1Red = 0.5f;
	GLint palette1GreenLocation;
	float palette1Green = 0.5f;
	GLint palette1BlueLocation;
	float palette1Blue = 0.5f;
	GLint palette2RedLocation;
	float palette2Red = 1.0f;
	GLint palette2GreenLocation;
	float palette2Green = 1.0f;
	GLint palette2BlueLocation;
	float palette2Blue = 1.0f;
	GLint palette3RedLocation;
	float palette3Red = 0.263f;
	GLint palette3GreenLocation;
	float palette3Green = 0.416f;
	GLint palette3BlueLocation;
	float palette3Blue = 0.557f;
	GLint reverseTimeLocation;
	bool reverseTime = false;

private:
	struct RGBA
	{
		float red   = 1.0f;
		float green = 1.0f;
		float blue  = 0.0f;
		float alpha = 1.0f;
	};
	struct HSBA
	{
		float hue   = 0.0f;
		float sat   = 1.0f;
		float bri   = 1.0f;
		float alpha = 1.0f;
	};
	HSBA hsba2;

	ffglex::FFGLShader shader;  //!< Utility to help us compile and link some shaders into a program.
	ffglex::FFGLScreenQuad quad;//!< Utility to help us render a full screen quad.
};
