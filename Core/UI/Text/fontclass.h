/////////////////////////////////////////////////////////////////////////////////////////////
// Filename:     fontclass.h
// Description:  1. this class will handle the texture for the font,
//               the font data from the text file, and the function
//               used to build vertex buffers with the font data.
//               2. the vertex buffers that hold the font data for 
//               individual sentences will be in the TextStore and
//               not inside this class.
//
// Revising:     10.06.22
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "../../Mesh/Vertex.h"
#include <UICommon/Types.h>
#include <d3d11.h>


namespace UI
{

class FontClass
{
private:
    // contains data about character on the font texture
    struct FontType
    {
        float left, right; // left and right edge coordinates on the symbol on the texture
        int size;          // symbol width in pixels
    };

public:
    FontClass();
    ~FontClass();

    void Initialize(
        ID3D11Device* pDevice, 
        const char* fontDataFilePath,
        const char* textureFilename);

    // builds a vertices array by font texture data which is based on 
    // input sentence and upper-left position
    void BuildVertexArray(
        Core::VertexFont* vertices,
        const size numVertices,
        const char* sentence,
        const float drawAtX,
        const float drawAtY);

    // builds an indices array according to the vertices array
    // from the BuildVertexArray func;
    void BuildIndexArray(
        UINT* indices,
        const size indicesCount);


    //
    // Public query API
    //
    inline const TexID GetFontTexID() const { return fontTexID_; }
    inline const UINT GetFontHeight() const { return fontHeight_; }

    ID3D11ShaderResourceView* const GetTextureResourceView();
    ID3D11ShaderResourceView* const* GetTextureResourceViewAddress();


    // memory allocation
    void* operator new(size_t i);
    void operator delete(void* ptr);

private:  // restrict a copying of this class instance
    FontClass(const FontClass & obj);
    FontClass & operator=(const FontClass & obj);

private:
    void LoadFontData(
        const char* fontDataFilename,
        const int numOfFontChars,
        FontType* fontData);

private:
    int fontHeight_ = 0;                // the height of character in pixels
    const int charNum_ = 95;             // num of chars in the font texture
    FontType fontDataArr_[95];           // font raw data (position/width of each symbol in the texture, etc.)
    TexID fontTexID_ = 0;                // use this ID to get a font tex from the texture manager
};

} // namespace UI
