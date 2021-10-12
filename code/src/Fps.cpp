#include "stdafx.h"
#include "Fps.h"


Fps::Fps() :
	m_fps(0),
	m_count(0),
	m_startTime(0)
{
}

Fps::~Fps()
{
}


void Fps::Initialize()
{
	m_fps = 0;
	m_count = 0;
	m_startTime = timeGetTime();
}


void Fps::Frame()
{
	m_count++;

	if (timeGetTime() >= (m_startTime + 1000))
	{
		m_fps = m_count;
		m_count = 0;

		m_startTime = timeGetTime();
	}
}


int32_t Fps::GetFps()
{
	return m_fps;
}