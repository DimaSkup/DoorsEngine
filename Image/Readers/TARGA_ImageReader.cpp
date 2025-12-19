#include "../Common/pch.h"
#include <Image.h>
#include "TARGA_ImageReader.h"
#include <D3DX11tex.h>
#include "../ImgConverter.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"


#pragma warning (disable : 4996)

namespace Img
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
    {
        LogErr(LOG, "can't load targa tex from file: input path is empty");
        return false;
    }

    HRESULT hr = S_OK;
    bool result = false;

    

    D3D11_TEXTURE2D_DESC textureDesc;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

    ID3D11Texture2D*     p2DTexture = nullptr;
    ID3D11DeviceContext* pContext   = nullptr;
    pDevice->GetImmediateContext(&pContext);

    // ----------------------------------------------------- //

    int width = 0;
    int height = 0;
    int channels = 0;
    uint8* data = stbi_load(filePath, &width, &height, &channels, 0);

    if (!data) {
        LogErr(LOG, "Failed to load image: %s\n", stbi_failure_reason());
        return 1;
    }

    LogMsg(LOG, "Loaded TGA: %dx%d, %d channels\n", width, height, channels);

    texWidth                 = (UINT)width;
    texHeight                = (UINT)height;
    UINT bytesPerPixel = (uint)channels;
    UINT rowPitch      = (texWidth * bytesPerPixel);

    uint8* pixels = nullptr;

    // we need 4 channels, but have only 3
    if (channels == 3)
    {
        pixels = new uint8[width * height * 4]{ 0 };
        bytesPerPixel = 4;
        rowPitch = texWidth * bytesPerPixel;


        for (int i = 0; i < width * height; ++i)
        {
            pixels[i*4 + 0] = data[i*3 + 0];
            pixels[i*4 + 1] = data[i*3 + 1];
            pixels[i*4 + 2] = data[i*3 + 2];
            pixels[i*4 + 3] = 255;
        }
    }
    else if (channels == 4)
    {
        pixels = data;
    }
    else
    {
        // Free the image memory
        stbi_image_free(data);

        LogErr(LOG, "unsupported number of channes %d, for image: %s", filePath);
        return false;
    }

#if 0
    Image img;
    img.LoadData(filePath);

    const uint8* targaData = img.GetData();
    texWidth  = img.GetWidth();
    texHeight = img.GetHeight();

    const UINT bytesPerPixel = img.GetBPP() / 3;
    const UINT rowPitch      = (texWidth * bytesPerPixel) * sizeof(uint8);
#endif
  

    // next we need to setup our description of the DirectX texture that we will load
    // the Targa data into. We use the height and width from the Targa image data, and 
    // set the format to be a 32-bit RGBA texture. We set the SampleDesc to default.
    // Then we set the Usage to D3D11_USAGE_DEFAULT which is better performing memory.
    // And finally, we set the MipLevels, BindFlags, and MiscFlags to the settings 
    // required for Mipmapped textures. Once the description is complete, we call
    // CreateTexture2D() to create an empty texture for us. The next step will be to 
    // copy the Targa data into that empty texture.

    Img::ImgConverter        imgConv;
    DirectX::Image           img;
    DirectX::TexMetadata     metadata;

    const UINT miscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
    const bool mipMapped = true;

    textureDesc.Width               = texWidth;
    textureDesc.Height              = texHeight;
    textureDesc.MipLevels           = 0;
    textureDesc.ArraySize           = 1;
    textureDesc.Format              = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count    = 1;
    textureDesc.SampleDesc.Quality  = 0;
    textureDesc.Usage               = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags           = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags      = 0;
    textureDesc.MiscFlags           = miscFlags;

    // setup image params
    img.width                       = width;
    img.height                      = height;
    img.format                      = DXGI_FORMAT_R8G8B8A8_UNORM;
    img.rowPitch                    = width * sizeof(uint8) * 4; // 4 bytes per pixel (32bit image)
    img.slicePitch                  = 0;
    img.pixels                      = (uint8*)pixels;

    // setup image metadata
    metadata.width                  = width;
    metadata.height                 = height;
    metadata.depth                  = 1;
    metadata.arraySize              = 1;
    metadata.mipLevels              = 0;
    metadata.miscFlags              = miscFlags;
    metadata.format                 = DXGI_FORMAT_R8G8B8A8_UNORM;
    metadata.dimension              = DirectX::TEX_DIMENSION_TEXTURE2D;

#if 0
    D3D11_SUBRESOURCE_DATA initialData = { 0 };
    initialData.pSysMem     = pixels;
    initialData.SysMemPitch = rowPitch;

    // create the empty texture
    hr = pDevice->CreateTexture2D(&textureDesc, &initialData, &p2DTexture);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create an empty 2D texture for file: %s", filePath);

        // Free the image memory
        SafeDeleteArr(pixels);
        stbi_image_free(data);
        return false;
    }
#endif

    // create texture resource
    imgConv.CreateTexture2dEx(
        pDevice,
        img,
        metadata,
        textureDesc.Usage,
        textureDesc.BindFlags,
        textureDesc.CPUAccessFlags,
        textureDesc.MiscFlags,
        mipMapped,
        ppTexture);

    // Free the image memory
    stbi_image_free(data);

    if (channels == 3)
        SafeDeleteArr(pixels);

#if 0
    // copy the targa image data into the texture
    pDeviceContext->UpdateSubresource(p2DTexture, 0, nullptr, targaData, rowPitch, 0);
#endif

    // setup the shader resource view description
    srvDesc.Format                      = textureDesc.Format;
    srvDesc.ViewDimension               = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip   = 0;
    srvDesc.Texture2D.MipLevels         = -1;

    // after the texture is loaded, we create a shader resource view which allows us to have
    // a pointer to set the texture in shaders.
    hr = pDevice->CreateShaderResourceView(*ppTexture, &srvDesc, ppTextureView);
    if (FAILED(hr))
    {
        SafeRelease(ppTexture);
        LogErr(LOG, "can't create the shader resource view for texture: %s", filePath);
        return false;
    }

    // generate mipmaps for this texture
    //pContext->GenerateMips(*ppTextureView);

    // store a ptr to the 2D texture 
    //*ppTexture = (ID3D11Texture2D*)(p2DTexture);

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
