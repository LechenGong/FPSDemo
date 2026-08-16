#pragma once

struct KeyButtonState
{
	// whether the key/button is down this frame
	bool currDown = false;
	// whether the key/button is down previous frame
	bool prevDown = false;

	float timePressed = 0.f;
};