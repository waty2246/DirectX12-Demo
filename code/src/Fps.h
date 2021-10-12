#pragma once

class Fps
{
public:
	Fps();
	~Fps();

	void Initialize();
	void Frame();
	int32_t GetFps();

private:
	int32_t m_fps, m_count;
	unsigned long m_startTime;
};