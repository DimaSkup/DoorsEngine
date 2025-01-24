// ************************************************************************************
// Filename: fontclass.cpp
// ************************************************************************************
#include "fontclass.h"

#include "../Common/MemHelpers.h"
#include "../Common/Assert.h"
#include "../Common/Log.h"
#include "../../Texture/TextureMgr.h"

#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;



FontClass::FontClass()
{
}

FontClass::~FontClass() 
{
	Log::Debug();
}



// ************************************************************************************
//                           PUBLIC MODIFICATION API
// ************************************************************************************

void FontClass::Initialize(
	ID3D11Device* pDevice,
	const std::string& fontDataFilePath,
	const std::string& fontTexFilePath)
{
	// this function will load the font data and the font texture

	Log::Debug();

	try
	{
		// check input params
		fs::path fontDataPath = fontDataFilePath;
		fs::path fontTexPath = fontTexFilePath;

		Assert::True(fs::exists(fontDataPath), "there is no file for font data: " + fontDataFilePath);
		Assert::True(fs::exists(fontTexPath), "there is no file for font texture" + fontTexFilePath);


		// load and initialize a texture for this font
		TextureMgr& texMgr = *TextureMgr::Get();
		fontTexID_ = texMgr.LoadFromFile(fontTexFilePath);

		// we need to have a height of the font texture for proper building of the vertices data
		fontHeight_ = texMgr.GetTexPtrByID(fontTexID_)->GetHeight();
		
		// load the data into the font data array
		LoadFontData(fontDataFilePath, charNum_, fontDataArr_);
	}
	catch (const std::bad_alloc& e)
	{
		this->~FontClass();
		Log::Error(e.what());
		throw EngineException("can't initialize the FontClass object");
	}
	catch (EngineException & e)
	{
		this->~FontClass();
		Log::Error(e, true);
		throw EngineException("can't initialize the FontClass object");
	}
}

///////////////////////////////////////////////////////////

void FontClass::BuildVertexArray(
	VertexFont* vertices,
	const size numVertices,
	const std::string& sentence,
	const DirectX::XMFLOAT2& drawAt)
{
	// BuildVertexIndexArrays() builds a vertices array by texture data which is based on 
	// input sentence and upper-left position
	// (this function is called by a TextStore object)

	Assert::True((vertices != nullptr) & (numVertices > 0), "wrong input vertices buffer");
	Assert::True(std::ssize(sentence) <= numVertices, "input vertices buffer is too small");

	float drawX = drawAt.x;
	const float topY = drawAt.y;
	const float bottomY = topY - fontHeight_;

	// go through each character of the input sentence
	for (int index = 0; const int ch : sentence)
	{
		const int symbol = ch - 32;

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
			const float texLeft = fontDataArr_[symbol].left;
			const float texRight = fontDataArr_[symbol].right;
			const float width = static_cast<float>(fontDataArr_[symbol].size);

			// set pos(x,y) and texture(tu,tv) for each font vertex:
			// top left, bottom right, bottom left, top right
			vertices[index++] = { { drawX, topY },            { texLeft, 0.0f } };
			vertices[index++] = { { drawX + width, bottomY }, { texRight, 1.0f } };
			vertices[index++] = { { drawX, bottomY },         { texLeft, 1.0f } };
			vertices[index++] = { { drawX + width, topY },    { texRight, 0.0f } };

#if 0       // OLD CODE

			// top left
			vertices[index].position     = { drawX, topY };
			vertices[index].texture      = { texLeft, 0.0f };

			// bottom right
			vertices[index + 1].position = { drawX + width, bottomY };
			vertices[index + 1].texture  = { texRight, 1.0f };

			// bottom left
			vertices[index + 2].position = { drawX, bottomY };
			vertices[index + 2].texture  = { texLeft, 1.0f };

			// top right
			vertices[index + 3].position = { drawX + width, topY };
			vertices[index + 3].texture  = { texRight, 0.0f };

			index += 4;
#endif
			
			// shift the drawing position by (char width + 1) pixel
			drawX += (width + 1.0f);
		}
	}
}

///////////////////////////////////////////////////////////

void FontClass::BuildIndexArray(UINT* indices, const size numIndices)
{
	// NOTE: the input indices array must be empty before initialization
	Assert::True(bool(indices) & (numIndices > 0), "wrong num of indices (must be > 0): " + std::to_string(numIndices));
	
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

#if 0
		indices.insert(indices.end(),  
		{
			v_idx, v_idx+1, v_idx+2,  
			v_idx, v_idx+3, v_idx+1, 
		});
#endif

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

	Log::Error("can't allocate the memory for object");
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
	return TextureMgr::Get()->GetTexByID(fontTexID_).GetTextureResourceView();
}

ID3D11ShaderResourceView* const* FontClass::GetTextureResourceViewAddress()
{
	return TextureMgr::Get()->GetTexByID(fontTexID_).GetTextureResourceViewAddress();
}


// ************************************************************************************
//                             PRIVATE MODIFICATION API 
// ************************************************************************************

void FontClass::LoadFontData(
	const std::string& fontDataFilePath,
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
		Log::Error(e);
		throw EngineException("can't load the font data from the file");
	}
}
