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
	GLint speedLocation;
	float speed = 0.4f;
	GLint fractalAmountLocation;
	float fractalAmount = 1.5f;
	GLint colorPulseLocation;
	float colorPulse = 1.5f;

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

	GLint positionXLocation;
	float positionX = 0.5f;
	GLint positionYLocation;
	float positionY = 0.5f;
	GLint positionZLocation;
	float positionZ = 0.5f;
	GLint positionXboxLocation;
	float positionXbox = 0.5f;
	GLint positionYboxLocation;
	float positionYbox = 0.5f;
	GLint positionZboxLocation;
	float positionZbox = 0.5f;

	GLint scaleSphereLocation;
	float scaleSphere = 0.5f;

	GLint scaleSphereXLocation;
	float scaleSphereX = 0.5f;
	GLint scaleSphereYLocation;
	float scaleSphereY = 0.5f;
	GLint scaleSphereZLocation;
	float scaleSphereZ = 0.5f;

	GLint sizeSphereLocation;
	float sphereSize = 0.5f;

	GLint scaleBoxLocation;
	float scaleBox = 0.5f;

	GLint scaleBoxXLocation;
	float scaleBoxX = 0.5f;
	GLint scaleBoxYLocation;
	float scaleBoxY = 0.5f;
	GLint scaleBoxZLocation;
	float scaleBoxZ = 0.5f;

	GLint sizeBoxLocation;
	float boxSize = 0.5f;

	GLint groundHeightLocation;
	float groundHeight = 0.5f;

	GLint fieldOfViewLocation;
	float fieldOfView = 0.5f;

	// boxLeftRightCrossfade
	GLint boxLeftRightCrossfadeLocation;
	float boxLeftRightCrossfade = 0.5f;

	// boxUpDownCrossfade
	GLint boxUpDownCrossfadeLocation;
	float boxUpDownCrossfade = 0.5f;

	// sphereLeftRightCrossfade
	GLint sphereLeftRightCrossfadeLocation;
	float sphereLeftRightCrossfade = 0.5f;

	// sphereUpDownCrossfade
	GLint sphereUpDownCrossfadeLocation;
	float sphereUpDownCrossfade = 0.5f;

	GLint cameraXLocation;
	float cameraX = 0.0f;//!< Camera X position.
	GLint cameraYLocation;
	float cameraY = 0.0f;//!< Camera Y position.

	GLint brightnessLocation;
	float brightness = 0.05f;

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
