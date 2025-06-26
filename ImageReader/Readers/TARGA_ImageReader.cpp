#include "../Common/pch.h"
#include "Image.h"
#include "TARGA_ImageReader.h"

#pragma warning (disable : 4996)

namespace ImgReader
{

// we define the Targa file header structure here to 
// make reading in the data easier (for .tga format)
struct TargaHeader
{
    UCHAR data1[12]{ '\0' };
    USHORT width = 0;
    USHORT height = 0;
    UCHAR bpp{ '\0' };
    UCHAR data2{ '\0' };
};

///////////////////////////////////////////////////////////

// predefinition of the helper function
void LoadTarga32Bit(
    const char* filePath,
    UCHAR** targaDataArr,   // raw image data
    UINT& textureWidth,
    UINT& textureHeight);


// =================================================================================
//                             PUBLIC FUNCTIONS
// =================================================================================
bool TARGA_ImageReader::LoadTextureFromFile(
    const char* filePath,
    ID3D11Device* pDevice,
    ID3D11Resource** ppTexture,
    ID3D11ShaderResourceView** ppTextureView,
    UINT& texWidth,
    UINT& texHeight)
{
    // this function loads a TARGA texture from the file by filePath
    // and initializes input parameters: texture resource, shader resource view,
    // width and height of the texture;

    if (filePath == nullptr || filePath[0] == '\0')
        LogErr("a path to targa texture is empty");

    HRESULT hr = S_OK;
    bool result = false;

    UINT rowPitch = 0;
    const UINT bytesOfPixel = 4;

    D3D11_TEXTURE2D_DESC textureDesc;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

    ID3D11Texture2D* p2DTexture = nullptr;
    ID3D11DeviceContext* pDeviceContext = nullptr;

    // holds the raw Targa data read straight in from the file
    UCHAR* targaData = nullptr;

    // ----------------------------------------------------- //

    Image img;
    img.LoadData(filePath);

    // load the targa image data into memory (into the targaDataArr array) 
    //LoadTarga32Bit(filePath, &targaData, texWidth, texHeight);

    targaData = img.GetData();
    texWidth = img.GetWidth();
    texHeight = img.GetHeight();

    // next we need to setup our description of the DirectX texture that we will load
    // the Targa data into. We use the height and width from the Targa image data, and 
    // set the format to be a 32-bit RGBA texture. We set the SampleDesc to default.
    // Then we set the Usage to D3D11_USAGE_DEFAULT which is better performing memory.
    // And finally, we set the MipLevels, BindFlags, and MiscFlags to the settings 
    // required for Mipmapped textures. Once the description is complete, we call
    // CreateTexture2D() to create an empty texture for us. The next step will be to 
    // copy the Targa data into that empty texture.

    textureDesc.Width               = texWidth;   // we've gotten width/height in the LoadTarga32Bit function
    textureDesc.Height              = texHeight;
    textureDesc.MipLevels           = 0;
    textureDesc.ArraySize           = 1;
    textureDesc.Format              = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count    = 1;
    textureDesc.SampleDesc.Quality  = 0;
    textureDesc.Usage               = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags           = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    textureDesc.CPUAccessFlags      = 0;
    textureDesc.MiscFlags           = D3D11_RESOURCE_MISC_GENERATE_MIPS;


    // create the empty texture
    hr = pDevice->CreateTexture2D(&textureDesc, nullptr, &p2DTexture);
    if (FAILED(hr))
    {
        SafeDeleteArr(targaData);
        sprintf(g_String, "can't create an empty 2D texture for file: %s", filePath);
        LogErr(g_String);
        return false;
    }

    // set the row pitch of the targa image data
    rowPitch = (texWidth * bytesOfPixel) * sizeof(UCHAR);

    // get the device context
    pDevice->GetImmediateContext(&pDeviceContext);

    // copy the targa image data into the texture
    pDeviceContext->UpdateSubresource(p2DTexture, 0, nullptr, targaData, rowPitch, 0);

    SafeDeleteArr(targaData);

    // setup the shader resource view description
    srvDesc.Format                      = textureDesc.Format;
    srvDesc.ViewDimension               = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip   = 0;
    srvDesc.Texture2D.MipLevels         = -1;

    // after the texture is loaded, we create a shader resource view which allows us to have
    // a pointer to set the texture in shaders.
    hr = pDevice->CreateShaderResourceView(p2DTexture, &srvDesc, ppTextureView);
    if (FAILED(hr))
    {
        SafeRelease(&p2DTexture);
        sprintf(g_String, "can't create the shader resource view for texture: %s", filePath);
        LogErr(g_String);
        return false;
    }

    // generate mipmaps for this texture
    pDeviceContext->GenerateMips(*ppTextureView);

    // store a ptr to the 2D texture 
    *ppTexture = (ID3D11Texture2D*)(p2DTexture);

    return true;
}


// =================================================================================
//                        PRIVATE HELPER FUNCTIONS
// =================================================================================
void LoadTarga32Bit(
    const char* filePath,
    UCHAR** targaDataArr,   // raw image data
    UINT& textureWidth,
    UINT& textureHeight)
{
    // this is a Targa image loading function. NOTE that Targa images are stored upside down
    // and need to be flipped before using. So here we will open the file, read it into
    // an array, and then take that array data and load it into the pTargaData_ array in
    // the correct order. Note we are purposely only dealing with 32-bit Targa files that
    // have alpha channels, this function will reject Targa's that are saved as 24-bit


    errno_t error = -1;
    //UCHAR bpp = 0;           // bites per pixel (supposed to be 32)
    size_t count = 0;
    TargaHeader targaFileHeader;
    FILE* pFile = nullptr;
    UCHAR* targaImageDataArr = nullptr;

    try
    {
        // open the targa file for reading in binary
        error = fopen_s(&pFile, filePath, "rb");
        if (error != 0)
        {
            sprintf(g_String, "can't open the targa file for reading in binary: %s", filePath);
            throw EngineException(g_String);
        }

        // read in the file header
        count = fread(&targaFileHeader, sizeof(TargaHeader), 1, pFile);
        if (count != 1)
        {
            sprintf(g_String, "can't read in the file header: %s", filePath);
            throw EngineException(g_String);
        }

        // get the important information from the header
        textureWidth  = (UINT)(targaFileHeader.width);
        textureHeight = (UINT)(targaFileHeader.height);

        // check that it is 32 bit and not 24 bit
        if (targaFileHeader.bpp != UCHAR(32))
        {
            sprintf(g_String, "this targa texture is not 32-bit: %s", filePath);
            throw EngineException(g_String);
        }

        // calculate the size of the 32 bit image data
        UINT imageSize = textureWidth * textureHeight * 4;

        // allocate memory for the targa image data
        targaImageDataArr = new UCHAR[imageSize]{ 0 };

        // allocate memory for the targa destination data
        *targaDataArr = new UCHAR[imageSize]{ 0 };

        // read in the targa image data
        count = fread(targaImageDataArr, 1, imageSize, pFile);
        if (count != (size_t)imageSize)
        {
            sprintf(g_String, "can't read in the targa image data from file: %s", filePath);
            throw EngineException(g_String);
        }

        // close the file
        error = fclose(pFile);
        if (error != 0)
        {
            sprintf(g_String, "can't close the file: %s", filePath);
            throw EngineException(g_String);
        }


        // setup the index into the targa image data
        UINT k = (imageSize)-(textureWidth * 4);

        // now copy the targa image data into the targa destination array in the correct
        // order since the targa format is stored upside down and also is not in RGBA order.
        for (int index = 0, j = 0; j < (int)textureHeight; j++)
        {
            for (int i = 0; i < (int)textureWidth; i++)
            {
                *targaDataArr[index + 0] = targaImageDataArr[k + 2];  // red
                *targaDataArr[index + 1] = targaImageDataArr[k + 1];  // green
                *targaDataArr[index + 2] = targaImageDataArr[k + 0];  // blue
                *targaDataArr[index + 3] = targaImageDataArr[k + 3];  // alpha

                k += 4;
                index += 4;
            }

            // set the targa image data index back to the preceding row at the beginning
            // of the column since its reading is upside down
            k -= (textureWidth * 8);
        }

        SafeDeleteArr(targaImageDataArr);
    }
    catch (std::bad_alloc& e)
    {
        SafeDeleteArr(targaImageDataArr);
        SafeDeleteArr(*targaDataArr);
        fclose(pFile);              // close the targa file
        LogErr(e.what());
        throw EngineException("can't allocate memory for the targa image data array / targa destination data array");
    }
    catch (EngineException & e)
    {
        SafeDeleteArr(targaImageDataArr);
        SafeDeleteArr(*targaDataArr);
        fclose(pFile);              // close the targa file
        LogErr(e);
        throw EngineException("can't read targa-image data");
    }
}

} // namespace ImgReader
