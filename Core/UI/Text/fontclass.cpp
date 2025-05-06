// ************************************************************************************
// Filename: fontclass.cpp
// ************************************************************************************
#include "fontclass.h"

#include <CoreCommon/MemHelpers.h>
#include <CoreCommon/Assert.h>
#include <CoreCommon/Log.h>
#include <CoreCommon/FileSystem.h>
#include "../../Texture/TextureMgr.h"

#include <fstream>

using namespace Core;


namespace UI
{

FontClass::FontClass() {}
FontClass::~FontClass() {}


// ************************************************************************************
//                           PUBLIC MODIFICATION API
// ************************************************************************************
void FontClass::Initialize(
	ID3D11Device* pDevice,
	const char* fontDataFilePath,
	const char* fontTexFilePath)
{
	// this function will load the font data and the font texture

	LogDbg("init");

	try
	{
        bool canInit = true;

		// check input params
        canInit &= FileSys::Exists(fontDataFilePath);
        canInit &= FileSys::Exists(fontTexFilePath);

        if (!canInit)
        {
            LogErr("isn't able to initialize the font class: input args are invalid");
            return;
        }

		// load and initialize a texture for this font
		fontTexID_ = g_TextureMgr.LoadFromFile(fontTexFilePath);

		// we need to have a height of the font texture for proper building of the vertices data
		fontHeight_ = g_TextureMgr.GetTexPtrByID(fontTexID_)->GetHeight();
		
		// load the data into the font data array
		LoadFontData(fontDataFilePath, charNum_, fontDataArr_);
	}
	catch (EngineException & e)
	{
		this->~FontClass();
		LogErr(e, true);
		throw EngineException("can't initialize the FontClass object");
	}
}

///////////////////////////////////////////////////////////

void FontClass::BuildVertexArray(
	Core::VertexFont* vertices,
	const size numVertices,
	const char* sentence,
	const float drawAtX,
    const float drawAtY)
{
	// BuildVertexIndexArrays() builds a vertices array by texture data which is based on 
	// input sentence and upper-left position
	// (this function is called by a TextStore object)

	Assert::True(vertices && (numVertices > 0),             "wrong input vertices buffer");
    Assert::True(sentence && (sentence[0] != '\0'),         "input sentence is empty");
	Assert::True((size)strlen(sentence) <= (numVertices/4), "input vertices buffer is too small");

	float       drawX   = drawAtX;
	const float topY    = drawAtY;
	const float bottomY = topY - fontHeight_;

	// go through each character of the input sentence
	for (int i = 0, symbolIdx = 0; symbolIdx < strlen(sentence); ++symbolIdx)
	{
		const int symbol = sentence[symbolIdx] - 32;

		// if there is a space (symbol == 0)
		if (!symbol) 
		{
			drawX += 3.0f; // skip 3 pixels
			continue;
		}
		// else we build a polygon for this symbol 
		else  
		{
			// the symbol texture params
			const float texLeft  = fontDataArr_[symbol].left;
			const float texRight = fontDataArr_[symbol].right;
			const float width    = (float)(fontDataArr_[symbol].size);

			// set pos(x,y) and texture(tu,tv) for each font vertex:
			// top left, bottom right, bottom left, top right
			vertices[i++] = { { drawX, topY },            { texLeft, 0.0f } };
			vertices[i++] = { { drawX + width, bottomY }, { texRight, 1.0f } };
			vertices[i++] = { { drawX, bottomY },         { texLeft, 1.0f } };
			vertices[i++] = { { drawX + width, topY },    { texRight, 0.0f } };
			
			// shift the drawing position by (char width + 1) pixel
			drawX += (width + 1.0f);
		}
	}
}

///////////////////////////////////////////////////////////

void FontClass::BuildIndexArray(UINT* indices, const size numIndices)
{
	// NOTE: the input indices array must be empty before initialization

	Assert::True((indices != nullptr) && (numIndices > 0), "invalid input params");
	
	for (UINT vIdx = 0, arrIdx = 0; arrIdx < (UINT)numIndices;)
	{
		// insert 6 indices at this position

		// first triangle 
		indices[arrIdx++] = vIdx + 0;
		indices[arrIdx++] = vIdx + 1;
		indices[arrIdx++] = vIdx + 2;

		// second triangle
		indices[arrIdx++] = vIdx + 0;
		indices[arrIdx++] = vIdx + 3;
		indices[arrIdx++] = vIdx + 1;

		vIdx += 4;  // stride by 4 (the number of vertices in a symbol)
	}
}

///////////////////////////////////////////////////////////

void* FontClass::operator new(size_t i)
{
	// memory allocation
	// any FontClass object is aligned on 16 in the memory

	if (void* ptr = _aligned_malloc(i, 16))
	{
		return ptr;
	}

	LogErr("can't allocate the memory for object");
	throw std::bad_alloc{};
}

void FontClass::operator delete(void* p)
{
	_aligned_free(p);
}


// ************************************************************************************
//                               PUBLIC QUERY API
// ************************************************************************************

ID3D11ShaderResourceView* const FontClass::GetTextureResourceView()
{
	return g_TextureMgr.GetTexByID(fontTexID_).GetTextureResourceView();
}

ID3D11ShaderResourceView* const* FontClass::GetTextureResourceViewAddress()
{
	return g_TextureMgr.GetTexByID(fontTexID_).GetTextureResourceViewAddress();
}


// ************************************************************************************
//                             PRIVATE MODIFICATION API 
// ************************************************************************************
void FontClass::LoadFontData(
	const char* fontDataFilePath,
	const int numOfFontChars,
	FontType* fontData)
{
	// LoadFontData() loads from the file texture left, right coordinates
	// for each symbol and the width in pixels of each symbol

	std::ifstream fin;

	try 
	{
		fin.open(fontDataFilePath, std::ifstream::in);
		Assert::True(fin.is_open(), "can't open the file with font data");

		// read in data from the buffer
		for (int i = 0; i < numOfFontChars - 2; i++)
		{
			// skip the ASCII-code of the character and the character itself
			while (fin.get() != ' ') {}
			while (fin.get() != ' ') {}

			// read in the character font data
			fin >> fontData[i].left;
			fin >> fontData[i].right;
			fin >> fontData[i].size;
		}

		fin.close();
	}
	catch (std::ifstream::failure e)
	{
		fin.close();
		throw EngineException("exception opening/reading/closing file");
	}
	catch (EngineException & e)
	{
		fin.close();
		LogErr(e);
		throw EngineException("can't load the font data from the file");
	}
}

} // namespace UI
