////////////////////////////////////////////////////////////////////////////////////////////
// Filename:     SpriteClass.cpp
// Description:  will be used to represent an individual 2D image
//               that needs to be rendered to the screen. For every 
//               2D image you have you will need a new BitmapClass object;
//               It also includes a frame timer assist smoothly cycling 
//               through the textures that get mapped to the 2D square.
//
// Created:      14.08.23
////////////////////////////////////////////////////////////////////////////////////////////
#include "SpriteClass.h"

#include <fstream>


#if 0

SpriteClass::SpriteClass(ID3D11Device* pDevice,	ID3D11DeviceContext* pDeviceContext)
	: Model(pDevice, pDeviceContext)
{
	try
	{
		this->modelType_ = "2D_sprite";
	}
	catch (EngineException & e)
	{
		Log::Error(e, false);
		throw EngineException("can't allocate memory for the SpriteClass members");
	}
	
}

SpriteClass::~SpriteClass()
{
	//std::string debugMsg{ "destroyment of the " + this->GetModelDataObj()->GetID() };
	//Log::Debug(debugMsg.c_str());
}




////////////////////////////////////////////////////////////////////////////////////////////
//
//                              PUBLIC FUNCTIONS
// 
////////////////////////////////////////////////////////////////////////////////////////////


bool SpriteClass::Initialize(const std::string & filePath)
{
	// we initialize a 2Â sprites's model data in a separate way as it is faster than
	// each time reading from the data file or copying buffers from some another sprite model;


	try
	{
		// this flag means that we want to create a dynamic vertex buffer for the mesh of this sprite
		const bool isVertexBufferDynamic = true;    

		// since each 2D sprite is just a plane it has 4 vertices and 6 indices
		const UINT vertexCount = 4;
		const UINT indexCount = 6;

		// arrays for vertices/indices data
		std::vector<Vertex3D> verticesArr(vertexCount);
		std::vector<UINT> indicesArr(indexCount, 0);

		/////////////////////////////////////////////////////

		// setup the vertices positions
		verticesArr[0].position = { -1,  1,  0 };    // top left
		verticesArr[1].position = {  1, -1,  0 };    // bottom right 
		verticesArr[2].position = { -1, -1,  0 };    // bottom left
		verticesArr[3].position = {  1,  1,  0 };    // top right
		
		// setup the texture coords of each vertex
		verticesArr[0].texture = { 0, 0 };
		verticesArr[1].texture = { 1, 1 };     
		verticesArr[2].texture = { 0, 1 };    
		verticesArr[3].texture = { 1, 0 };    

		// setup the indices
		indicesArr.insert(indicesArr.begin(), { 0, 1, 2, 0, 3, 1 });

		/////////////////////////////////////////////////////

		// each 2D sprite has only one mesh so create it and fill in with data
		this->InitializeOneMesh(verticesArr, indicesArr, {}, isVertexBufferDynamic);

	}
	catch (EngineException & e)
	{
		Log::Error(e, false);
		Log::Error("can't initialize a 2D sprite model");

		return false;
	}
	
	return true;

} // end Initialize

///////////////////////////////////////////////////////////


void SpriteClass::SetupSprite(const POINT & renderAtPos,
	UINT screenWidth, UINT screenHeight,
	const std::string & spriteInfoDataFile)
{
	// ATTENTION: call this function after the Initialize() function

	// check input params
	Assert::True(spriteInfoDataFile.empty() == false, "the input name of the sprite's data file is empty");
	Assert::NotZero(screenWidth, "input screen width is zero");
	Assert::NotZero(screenHeight, "input screen height is zero");

	/////////////////////////////////////////////////////

	// initialize textures for this sprite
	bool result = LoadTextures(spriteInfoDataFile);
	Assert::True(result, "can't load textures for a 2D sprite");

	this->SetRenderLocation(renderAtPos.x, renderAtPos.y);
	this->SetScreenDimensions(screenWidth, screenHeight);

	// get the dimensions of the first texture and used that as the dimensions
	// of the 2D sprite images
	bitmapWidth_ = this->meshes_[0]->GetTexturesArr()[0]->GetWidth();
	bitmapHeight_ = this->meshes_[0]->GetTexturesArr()[0]->GetHeight();

	// set the starting texture in the cylce to be the first one in the list
	currentTexture_ = 0;

} // end SetupSprite

///////////////////////////////////////////////////////////

void SpriteClass::Render(const D3D_PRIMITIVE_TOPOLOGY topologyType)
{
	// update the buffers if the position of the sprite has changed 
	// from its original position
	UpdateBuffers();


	// for rendering 2D sprites we setup rendering pipeline in a separate way;
	// since each 2D sprite MUST HAVE only one mesh we get the first from the meshes array;
	this->meshes_[0]->Draw();


	DataContainerForShaders* pDataContainer = this->GetDataContainerForShaders();

	pDataContainer->indexCount = this->meshes_[0]->GetIndexCount();
	pDataContainer->texturesMap.insert_or_assign("diffuse", this->GetTextureResourceViewAddress());

	// render this mesh using a HLSL shader
	this->pModelToShaderMediator_->Render(this->pImmediateContext_, this);

	return;
}

///////////////////////////////////////////////////////////

void SpriteClass::Update(float frameTime)
{
	// This function takes in the frame time each frame. This will usually be around 
	// 16-17 ms if you are running your program at 60fps. Each frame we add this time to
	// the frameTime_ counter. If it reaches or passes the cycle time that was defined for
	// this sprite, then we change the sprite to use the next texture in the array. We then
	// reset the timer to start from zero again.


	// increment the grame time each frame
	frameTime_ += frameTime;

	// check if the frame time has reached the cycle time
	if (frameTime_ >= cycleTime_)
	{
		// if it has then reset the frame time and cycle to the next sprite in the texture array
		frameTime_ -= cycleTime_;

		currentTexture_++;

		// if we are at the last sprite texture then go back to the beginning of the texture
		// array to the first texture again
		if (currentTexture_ == textureCount_)
		{
			currentTexture_ = 0;
		}
	}

	return;

} // end Update




///////////////////////////////////////////////////////////////////////////////////////////
//                            PUBLIC SETTERS/LOADERS
///////////////////////////////////////////////////////////////////////////////////////////

void SpriteClass::SetRenderLocation(int posX, int posY)
{
	// store where the sprite should be rendered to
	renderX_ = posX;
	renderY_ = posY;

	return;
}

///////////////////////////////////////////////////////////

void SpriteClass::SetScreenDimensions(UINT width, UINT height)
{
	// a sprite will use width and height of the screen for the rendering purpose
	screenWidth_ = width;
	screenHeight_ = height;
}

///////////////////////////////////////////////////////////

#if 0
void SetSpriteDimensions(UINT width, UINT height)
{
	
}
#endif

///////////////////////////////////////////////////////////

bool SpriteClass::LoadTextures(const std::string & spriteInfoDataFile)
{
	// initialization of textures for this sprite;
	//
	// here we open the file and read in the number of textures it uses,
	// each dds/tga/etc. file used for each of the textures, and the speed
	// at which it should cycle through the textures


	char textureFilename[128]{ '\0' };   // fill in this array with null-terminated symbols
	std::ifstream fin;
	char input = ' ';
	bool result = false;

	// open the sprite info data file
	fin.open(spriteInfoDataFile);
	if (fin.fail())
	{
		std::string errorMsg{ "can't open the sprite info data file: " + (std::string)spriteInfoDataFile };
		Log::Error(errorMsg.c_str());
		return false;
	}

	// read in the number of textures
	fin >> textureCount_;

	// read to star of the next line
	fin.get(input);

	// read in each texture file name
	for (UINT i = 0; i < textureCount_; i++)
	{
		UINT j = 0;

		// read in a texture file name
		fin.get(input);
		while (input != '\n')
		{
			textureFilename[j] = input;
			++j;
			fin.get(input);
		}

		// once we have the filename then load the texture in the mesh's 
		// (we have only one mesh for a 2D sprite) textures array
		this->meshes_[0]->SetTextureByIndex(i, (std::string)textureFilename, aiTextureType_DIFFUSE);
	}

	// read in the cycle time (sprite animation speed)
	fin >> cycleTime_;

	// close the file
	fin.close();

	return true;
} // end LoadTextures

///////////////////////////////////////////////////////////

ID3D11ShaderResourceView** SpriteClass::GetTextureResourceViewAddress()
{
	// returns the current texture for the sprite from the texture array
	return this->meshes_[0]->GetTexturesArr()[currentTexture_]->GetTextureResourceViewAddress();
}

///////////////////////////////////////////////////////////

UINT SpriteClass::GetSpriteWidth() const
{
	return bitmapWidth_;
}

///////////////////////////////////////////////////////////

UINT SpriteClass::GetSpriteHeight() const
{
	return bitmapHeight_;
}





////////////////////////////////////////////////////////////////////////////////////////////
//
//                              PRIVATE FUNCTIONS
// 
////////////////////////////////////////////////////////////////////////////////////////////



void SpriteClass::UpdateBuffers()
{
	// if a sprite's position is changed we have to update its vertex buffer 
	// for rendering this sprite at the new position

	// if the position we are rendering this bitmap to hasn't changed 
	// then don't update the vertex buffer
	if ((prevPosX_ == renderX_) && (prevPosY_ == renderY_))
	{
		return;
	}

	// the rendering location has changed then store the new position and update the vertex buffer
	prevPosX_ = renderX_;
	prevPosY_ = renderY_;

	// an array for new vertices data 
	std::vector<Vertex3D> verticesArr(4);

	// calculate the screen coordinates of the left/right/top/bottom side of the bitmap
	float left   = (float)((screenWidth_ / 2) * -1.0f) + (float)(renderX_);
	float right  = left + (float)bitmapWidth_;
	float top    = (float)(screenHeight_ / 2) - (float)(renderY_);
	float bottom = top - (float)bitmapHeight_;

	/////////////////////////////////////////////////////

	// setup the vertices positions
	verticesArr[0].position = { (float)left,  (float)top,    0.0f };  // top left
	verticesArr[1].position = { (float)right, (float)bottom, 0.0f };  // bottom right 
	verticesArr[2].position = { (float)left,  (float)bottom, 0.0f };  // bottom left
	verticesArr[3].position = { (float)right, (float)top,    0.0f };  // top right

	// setup the texture coords of each vertex
	verticesArr[0].texture = { 0, 0 };
	verticesArr[1].texture = { 1, 1 };
	verticesArr[2].texture = { 0, 1 };
	verticesArr[3].texture = { 1, 0 };

	/////////////////////////////////////////////////////

	// update the DYNAMIC vertex buffer
	this->meshes_[0]->UpdateVertexBuffer(this->pImmediateContext_, verticesArr);

	return;

} // end UpdateBuffers


#endif