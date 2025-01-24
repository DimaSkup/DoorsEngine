////////////////////////////////////////////////////////////////////////////////////////////
// Filename:     SkyDomeClass.cpp
// Description:  this is a model class for the sky dome model
// 
// Created:      15.04.23
////////////////////////////////////////////////////////////////////////////////////////////
#include "SkyDomeClass.h"


#if 0



SkyDomeClass::SkyDomeClass(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
	: Model(pDevice, pDeviceContext)
{
	// setup a type name of the current model
	this->modelType_ = "sky_dome";  
	this->gameObjType_ = GameObject::GAME_OBJ_ZONE_ELEMENT;

	// also we init the game object's ID with the name of the model's type;
	// NOTE: DON'T CHANGE ID after this game object was added into the game objects list;
	//
	// but if you really need it you have to change the game object's ID manually inside of the game object list
	// and here as well using the SetID() function.
	this->ID_ = this->modelType_;   // default ID

	// setup colours of the sky dome
	apexColor_   = { 0.5f, 0.5f, 0.5f, 1.0f };  // grey
	centerColor_ = { 0.5f, 0.5f, 0.5f, 1.0f };  // grey
	//apexColor_ = { 1.0f, (1.0f / 255.0f) * 69.0f, 0.0f, 1.0f };    // orange-to-red
	//centerColor_ = { 1.0f, (1.0f / 255.0f) * 140.f, 0.1f, 1.0f };  // orange
}

SkyDomeClass::~SkyDomeClass()
{
	std::string debugMsg{ "destroyment of the sky dome"};
	Log::Debug(debugMsg.c_str());
}



////////////////////////////////////////////////////////////////////////////////////////////
//
//                              PUBLIC FUNCTIONS
// 
////////////////////////////////////////////////////////////////////////////////////////////


const DirectX::XMFLOAT4 & SkyDomeClass::GetApexColor() const
{
	// returns the colour of the sky dome at the very top
	return apexColor_;
}

///////////////////////////////////////////////////////////

const DirectX::XMFLOAT4 & SkyDomeClass::GetCenterColor() const
{
	// returns the colour of the sky dome at the horizon (or 0.0f to be exact)
	return centerColor_;
}

#endif