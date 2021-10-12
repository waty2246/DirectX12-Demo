#pragma once

class GameTimer
{
public:
	GameTimer();
	GameTimer(const GameTimer&);
	~GameTimer();

	bool Initialize();
	void Frame();

	float GetTime();

	void StartTimer();
	void StopTimer();
	int32_t GetTiming();

private:
	float m_frequency;
	INT64 m_startTime;
	float m_frameTime;
	INT64 m_beginTime, m_endTime;
};