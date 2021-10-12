#include "stdafx.h"
#include "Font.h"
#include "D3D.h"
#include "Texture2D.h"


Font::Font()
{
}

Font::~Font()
{
}


void Font::Initialize(D3D* d3d, const char* fontFileName, const char* textureFileName, float fontHeight, uint32_t spaceSize)
{
	// Store the height of the font.
	m_fontHeight = fontHeight;

	// Store the size of spaces in pixel size.
	m_spaceSize = spaceSize;

	// Load in the text file containing the font data.
	LoadFontData(fontFileName);

	// Load the texture that has the font characters on it.
	LoadTexture(d3d, textureFileName);
}

void Font::LoadFontData(const char* filename)
{
	ifstream fin;
	int32_t i;
	char temp;


	// Create the font spacing buffer.
	m_Font = make_unique<FontType[]>(95);

	// Read in the font size and spacing between chars.
	fin.open(filename);
	ThrowIfTrue(fin.fail());

	// Read in the 95 used ascii characters for text.
	for(i=0; i<95; i++)
	{
		fin.get(temp);
		while(temp != ' ')
		{
			fin.get(temp);
		}
		fin.get(temp);
		while(temp != ' ')
		{
			fin.get(temp);
		}

		fin >> m_Font[i].left;
		fin >> m_Font[i].right;
		fin >> m_Font[i].size;
	}

	// Close the file.
	fin.close();
}

void Font::LoadTexture(D3D* d3d, const char* filename)
{
	// Create the texture object.
	m_texture2D = make_unique<Texture2D>();

	// Initialize the texture object.
	m_texture2D->Initialize(d3d, filename);
}

ID3D12Resource* Font::GetTexture()
{
	return m_texture2D->GetTexture();
}


void Font::BuildVertexArray(void* vertices, const char* sentence, float drawX, float drawY)
{
	VertexType* vertexPtr;
	int32_t numLetters, index, i, letter;


	// Coerce the input vertices into a VertexType structure.
	vertexPtr = (VertexType*)vertices;

	// Get the number of letters in the sentence.
	numLetters = (int)strlen(sentence);

	// Initialize the index to the vertex array.
	index = 0;

	// Draw each letter onto a quad.
	for(i=0; i<numLetters; i++)
	{
		letter = ((int)sentence[i]) - 32;

		// If the letter is a space then just move over three pixels.
		if(letter == 0)
		{
			drawX = drawX + (float)m_spaceSize;
		}
		else
		{
			// First triangle in quad.
			vertexPtr[index].position = XMFLOAT3(drawX, drawY, 0.0f);  // Top left.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].left, 0.0f);
			index++;

			vertexPtr[index].position = XMFLOAT3((drawX + m_Font[letter].size), (drawY - m_fontHeight), 0.0f);  // Bottom right.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].right, 1.0f);
			index++;

			vertexPtr[index].position = XMFLOAT3(drawX, (drawY - m_fontHeight), 0.0f);  // Bottom left.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].left, 1.0f);
			index++;

			// Second triangle in quad.
			vertexPtr[index].position = XMFLOAT3(drawX, drawY, 0.0f);  // Top left.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].left, 0.0f);
			index++;

			vertexPtr[index].position = XMFLOAT3(drawX + m_Font[letter].size, drawY, 0.0f);  // Top right.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].right, 0.0f);
			index++;

			vertexPtr[index].position = XMFLOAT3((drawX + m_Font[letter].size), (drawY - m_fontHeight), 0.0f);  // Bottom right.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].right, 1.0f);
			index++;

			// Update the x location for drawing by the size of the letter and one pixel.
			drawX = drawX + m_Font[letter].size + 1.0f;
		}
	}

	return;
}


int32_t Font::GetSentencePixelLength(const char* sentence)
{
	int32_t pixelLength, numLetters, i, letter;


	pixelLength = 0;
	numLetters = (int)strlen(sentence);

	for (i = 0; i<numLetters; i++)
	{
		letter = ((int)sentence[i]) - 32;

		// If the letter is a space then count it as three pixels.
		if (letter == 0)
		{
			pixelLength += m_spaceSize;
		}
		else
		{
			pixelLength += (m_Font[letter].size + 1);
		}
	}

	return pixelLength;
}


uint32_t Font::GetFontHeight()
{
	return static_cast<uint32_t>(m_fontHeight);
}