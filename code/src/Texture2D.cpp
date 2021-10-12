#include "stdafx.h"
#include "Texture2D.h"
#include "D3D.h"

Texture2D::Texture2D()
{
}

Texture2D::~Texture2D()
{
}

void Texture2D::Initialize(D3D* d3d, const char* fileName)
{
	ComPtr<ID3D12Resource> cpuUploadTexture;

	auto device = d3d->GetDevice();
	auto commandList = d3d->GetCommandList();

	std::string fileNameStr(fileName);
	auto ext = fileNameStr.substr(fileNameStr.find_last_of(".") + 1);
	if (ext == "dds")
	{
		auto length = strnlen_s(fileName, 1024);
		unique_ptr<wchar_t[]> ddsFileName = make_unique<wchar_t[]>(1024);
		mbstowcs_s(&length, ddsFileName.get(),1024, fileName, length);

		ThrowIfFailed(CreateDDSTextureFromFile12(device, commandList, ddsFileName.get(), m_texture, cpuUploadTexture));
	}
	else
	{
		uint32_t height = 0, width = 0;
		unique_ptr<uint8_t[]> imageData;

		if (ext == "tga")
			imageData = LoadTarga(fileName, height, width);
		else if (ext == "bmp")
			imageData = LoadBMP(fileName, height, width);

		// Describe texture-2D
		D3D12_RESOURCE_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(D3D12_RESOURCE_DESC));

		textureDesc.MipLevels = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		textureDesc.DepthOrArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		// Create the empty default texture.
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),	// DEFAULT heap is accessed only by GPU
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,						// COPY_DEST state is needed for heap can be the target of a GPU copy operation
			nullptr,
			IID_PPV_ARGS(&m_texture)
		));

		// Texture in GPU have different memory layout than buffer accessed by CPU in system memory
		// We need to find the size needed for upload buffer
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, 1);

		// Create the CPU upload buffer.
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),	// UPLOAD heap is accessed for CPU write and GPU read
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,					// GENERIC_READ state is needed for heap that only read by GPU
			nullptr,
			IID_PPV_ARGS(&cpuUploadTexture)
		));

		D3D12_SUBRESOURCE_DATA textureData;
		ZeroMemory(&textureData, sizeof(D3D12_SUBRESOURCE_DATA));

		textureData.pData = imageData.get();
		textureData.RowPitch = width * 4;
		textureData.SlicePitch = textureData.RowPitch * height;

		UpdateSubresources(commandList, m_texture.Get(), cpuUploadTexture.Get(), 0, 0, 1, &textureData);

		// Transition texture to be used on pixel shader
		commandList->ResourceBarrier(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
		);
	}
	
	// Make copy in GPU and wait for it finish
	d3d->ExecuteAndWaitForGPU();
}

ID3D12Resource* Texture2D::GetTexture()
{
	return m_texture.Get();
}

unique_ptr<uint8_t[]> Texture2D::LoadBMP(const char* fileName, uint32_t& height, uint32_t& width)
{
	unique_ptr<uint8_t[]> bitmapData;
	std::ifstream bitmapFileStream;
	bitmapFileStream.open(fileName, std::ios::in | std::ios::binary);

	ThrowIfTrue(bitmapFileStream.fail());

	std::stringstream bitmapBuffer;
	bitmapBuffer << bitmapFileStream.rdbuf();

	auto data = bitmapBuffer.str();
	auto dataPtr = reinterpret_cast<const unsigned char*>(data.c_str());

	auto bitmapHeader = reinterpret_cast<const BITMAPFILEHEADER*>(dataPtr);
	auto bitmapInfo = reinterpret_cast<const BITMAPINFOHEADER*>(dataPtr + sizeof(BITMAPFILEHEADER));

	ThrowIfTrue(bitmapHeader->bfType != 0x4D42);
	ThrowIfTrue(bitmapInfo->biBitCount != 24 || bitmapInfo->biCompression != BI_RGB);

	// Bitmap row size must be multiple of four
	auto bitmapImage = dataPtr + bitmapHeader->bfOffBits;
	uint32_t bitmapHeight = std::abs(bitmapInfo->biHeight);
	uint32_t bitmapWidth = bitmapInfo->biWidth;
	int32_t indexBitmapData = 0;
	uint32_t rowSize = bitmapWidth * 3;
	uint32_t padding = (4 - rowSize % 4) & 0x03;
	rowSize += padding;

	auto bitmapSize = rowSize * bitmapHeight;
	std::unique_ptr<uint8_t[]> textureData{};

	// Bottom - Left else Top - Left.
	auto bottomLeft = false;
	if (bitmapInfo->biHeight > 0)
	{
		bottomLeft = true;
	}

	int32_t nextColor = 0;

	bitmapData = std::make_unique<uint8_t[]>(bitmapWidth * bitmapHeight * 4);
	for (uint32_t row = 0; row < bitmapHeight; ++row)
	{
		if (bottomLeft)
		{
			nextColor = bitmapSize - rowSize * (row + 1);
		}

		for (size_t col = 0; col < bitmapWidth; ++col)
		{
			bitmapData[indexBitmapData] = bitmapImage[nextColor + 2];
			bitmapData[indexBitmapData + 1] = bitmapImage[nextColor + 1];
			bitmapData[indexBitmapData + 2] = bitmapImage[nextColor];
			bitmapData[indexBitmapData + 3] = uint8_t(-1);
			nextColor += 3;
			indexBitmapData += 4;
		}
	}

	width = bitmapWidth;
	height = bitmapHeight;

	return bitmapData;
}

unique_ptr<uint8_t[]> Texture2D::LoadTarga(const char* filename, uint32_t& height, uint32_t& width)
{
	unique_ptr<uint8_t[]> targaData;
	std::ifstream targaFileStream;
	targaFileStream.open(filename, std::ios::in | std::ios::binary);

	ThrowIfTrue(targaFileStream.fail());

	std::stringstream targaBuffer;
	targaBuffer << targaFileStream.rdbuf();

	auto data = targaBuffer.str();
	auto dataPtr = reinterpret_cast<const uint8_t*>(data.c_str());
	auto targaHeader = reinterpret_cast<const TargaHeader*>(dataPtr);

	int32_t indexTargaData = 0, nextColor = 0;

	// Get the important information from the header.
	auto targaWidth = static_cast<uint32_t>(targaHeader->width);
	auto targaHeight = static_cast<uint32_t>(targaHeader->height);

	// Check that it is 32 bit and not 24 bit.
	ThrowIfTrue(targaHeader->bpp != 32);

	auto targaImage = dataPtr + sizeof(TargaHeader);

	// Allocate memory for the targa destination data.
	targaData = make_unique<uint8_t[]>(targaWidth * targaHeight * 4);

	// Initialize the index into the targa image data.
	nextColor = (targaWidth * targaHeight * 4) - (targaWidth * 4);

	// Now copy the targa image data into the targa destination array in the correct order since the targa format is stored upside down.
	for(uint32_t j=0; j < targaHeight; j++)
	{
		for(uint32_t i=0; i< targaWidth; i++)
		{
			targaData[indexTargaData + 0] = targaImage[nextColor + 2];  // Red.
			targaData[indexTargaData + 1] = targaImage[nextColor + 1];  // Green.
			targaData[indexTargaData + 2] = targaImage[nextColor + 0];  // Blue
			targaData[indexTargaData + 3] = targaImage[nextColor + 3];  // Alpha

			// Increment the indexes into the targa data.
			nextColor += 4;
			indexTargaData += 4;
		}

		// Set the targa image data index back to the preceding row at the beginning of the column since its reading it in upside down.
		nextColor -= (targaWidth * 8);
	}
	
	width = targaWidth;
	height = targaHeight;

	return targaData;
}