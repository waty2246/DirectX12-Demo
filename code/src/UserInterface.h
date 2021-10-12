#pragma once

class MiniMap;
class Text;
class Font;
class D3D;
class ShaderManager;

class UserInterfaceClass
{
public:
	UserInterfaceClass();
	~UserInterfaceClass();

	void Initialize(D3D* d3d, uint32_t screenHeight, uint32_t screenWidth);
	void Frame(ID3D12GraphicsCommandList* commandList, int32_t fps, float posX, float posY, float posZ, float rotX, float rotY, float rotZ);
	void Render(D3D*, ShaderManager*, XMMATRIX, XMMATRIX, XMMATRIX);

	void UpdateRenderCounts(ID3D12GraphicsCommandList* commandList, int32_t renderCount, int32_t nodesDrawn, int32_t nodesCulled);

private:
	void UpdateFpsString(ID3D12GraphicsCommandList* commandList, int32_t fps);
	void UpdatePositionStrings(ID3D12GraphicsCommandList*, float, float, float, float, float, float);

private:
	unique_ptr<Font> m_font;
	unique_ptr<Text> m_fpsString;
	unique_ptr<Text[]> m_videoStrings, m_positionStrings;
	int32_t m_previousFps;
	int32_t m_previousPosition[6];
	unique_ptr<Text[]> m_renderCountStrings;
	unique_ptr<MiniMap> m_miniMap;
};