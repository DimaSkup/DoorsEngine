#include "GameObject.h"


#if 0

GameObject::GameObject()
	: worldMatrix_(DirectX::XMMatrixIdentity()), // by default we set the game obj at the beginning of the world
	  position_(0, 0, 0),
	  scale_(1, 1, 1),
	  rotation_(0, 0, 0),
	  color_(1, 0, 1)
{
	// compute beginning matrices for the game object
	rotationMatrix_    = DirectX::XMMatrixRotationRollPitchYaw(rotation_.z, rotation_.y, rotation_.x);
	scalingMatrix_     = DirectX::XMMatrixScaling(scale_.x, scale_.y, scale_.z);
	translationMatrix_ = DirectX::XMMatrixTranslation(position_.x, position_.y, position_.z);

	// compute the final world matrix for the game object
	//this->UpdateMatrix();

	// update the directions vectors of the game object
	this->UpdateDirectionsVectors();
}

GameObject::GameObject(const GameObject & originGameObj)
{
	this->Copy(originGameObj);
}

GameObject::~GameObject()
{
}

///////////////////////////////////////////////////////////

void GameObject::Copy(const GameObject & originGameObj)
{
	// THIS FUNCTION copies all the data from the input 
	// origin game object into the current one;

	// copy all the data of the origin game object
	worldMatrix_       = originGameObj.worldMatrix_;
	rotationMatrix_    = originGameObj.rotationMatrix_;
	scalingMatrix_     = originGameObj.scalingMatrix_;
	translationMatrix_ = originGameObj.translationMatrix_;

	// model properties in the world
	position_ = originGameObj.position_;        // position of the model in the world
	scale_    = originGameObj.scale_;           // scale of the model
	rotation_ = originGameObj.rotation_;        // current angles of the model rotation (in radians)
	color_    = originGameObj.color_;           // color of the model

	vecForward_  = originGameObj.vecForward_;   // the current forward direction of the game obj
	vecLeft_     = originGameObj.vecLeft_;
	vecRight_    = originGameObj.vecRight_;
	vecBackward_ = originGameObj.vecBackward_;
}

///////////////////////////////////////////////////////////

void* GameObject::operator new(size_t i)
{
	// memory allocation (we need it because we use DirectX::XM-objects)
	if (void* ptr = _aligned_malloc(i, 16))
	{
		return ptr;
	}

	Log::Error("can't allocate the memory for object");
	throw std::bad_alloc{};
}

///////////////////////////////////////////////////////////

void GameObject::operator delete(void* p)
{
	_aligned_free(p);
}



////////////////////////////////////////////////////////////////////////////////////////////
//
//                                PUBLIC GETTERS
//
////////////////////////////////////////////////////////////////////////////////////////////


const DirectX::XMMATRIX & GameObject::GetWorldMatrix()
{
	// returns a world matrix
	return worldMatrix_;

}

///////////////////////////////////////////////////////////

//
// GETTERS for a position (in world)/scale/rotation/color/etc.
//
const DirectX::XMFLOAT3 & GameObject::GetPosition() const { return position_; }
const DirectX::XMFLOAT3 & GameObject::GetScale()    const { return scale_; }
const DirectX::XMFLOAT3 & GameObject::GetRotation() const { return rotation_; }
const DirectX::XMFLOAT3 & GameObject::GetColor()    const { return color_; };

/////////////////////////////////////////////////

///////////////////////////////////////////////////////////

const GameObject::GameObjectType GameObject::GetType() const
{
	return this->gameObjType_;
}

///////////////////////////////////////////////////////////

const std::string & GameObject::GetID() const
{
	// returns a game object's ID
	return this->ID_;
}

///////////////////////////////////////////////////////////

void GameObject::SetID(const std::string & newID)
{
	// set an identifier of the game object;
	//
	// NOTE: the ID of the game object can be changed when we add this game object
	//       into the GameObjectsList because there can't be the same IDs;
	Assert::True(newID.empty() == false, "wrong ID");
	ID_ = newID;
}



////////////////////////////////////////////////////////////////////////////////////////////
//
//               PUBLIC MODIFICATORS (SETTERS) FOR A GAME OBJECT IN THE WORLD
//
////////////////////////////////////////////////////////////////////////////////////////////

void GameObject::SetWorldMatrix(const DirectX::XMMATRIX & newWorldMatrix)
{
	// set a new world matrix for the game object
	this->worldMatrix_ = newWorldMatrix;
}

void GameObject::SetPosition(const float x, const float y, const float z)
{
	// set game object's position in the world
	position_.x = x;
	position_.y = y;
	position_.z = z;

	// compute a translation matrix for new translation values
	translationMatrix_ = DirectX::XMMatrixTranslation(position_.x, position_.y, position_.z);

	// compute the final world matrix for the game object
	this->UpdateMatrix();

	return;
}

///////////////////////////////////////////////////////////

void GameObject::SetPosition(const DirectX::XMFLOAT3 & position)
{
	// sets game object's position in the world using XMFLOAT3 type

	position_ = position;

	// compute a translation matrix for new translation values
	translationMatrix_ = DirectX::XMMatrixTranslation(position_.x, position_.y, position_.z);

	// compute the final world matrix for the game object
	this->UpdateMatrix();

	return;
}

///////////////////////////////////////////////////////////

void GameObject::SetPosition(const DirectX::XMFLOAT4 & position)
{
	// sets game object's position in the world using XMFLOAT4 type

	position_.x = position.x;
	position_.y = position.y;
	position_.z = position.z;

	// compute a translation matrix for new translation values
	translationMatrix_ = DirectX::XMMatrixTranslation(position_.x, position_.y, position_.z);

	// compute the final world matrix for the game object
	this->UpdateMatrix();

	return;
}

///////////////////////////////////////////////////////////

void GameObject::AdjustPosition(const DirectX::XMVECTOR & translationVector)
{
	// this functions adjusts the current position of a game object;

	position_.x += translationVector.m128_f32[0];
	position_.y += translationVector.m128_f32[1];
	position_.z += translationVector.m128_f32[2];

	// compute a translation matrix for new translation values
	translationMatrix_ = DirectX::XMMatrixTranslation(position_.x, position_.y, position_.z);

	// compute the final world matrix for the game object
	this->UpdateMatrix();

	return;
}

void GameObject::AdjustPosition(const DirectX::XMFLOAT3 & translatePos)
{
	// this functions adjusts the current position of a game object

	position_.x += translatePos.x;
	position_.y += translatePos.y;
	position_.z += translatePos.z;

	// compute a translation matrix for new translation values
	translationMatrix_ = DirectX::XMMatrixTranslation(position_.x, position_.y, position_.z);

	// compute the final world matrix for the game object
	this->UpdateMatrix();

	return;
}

///////////////////////////////////////////////////////////

void GameObject::AdjustPosition(const float x, const float y, const float z)
{
	// this functions adjusts the current position of a game object

	position_.x += x;
	position_.y += y;
	position_.z += z;

	// compute a translation matrix for new translation values
	translationMatrix_ = DirectX::XMMatrixTranslation(position_.x, position_.y, position_.z);

	// compute the final world matrix for the game object
	this->UpdateMatrix();

	return;
}

///////////////////////////////////////////////////////////

void GameObject::SetScale(const float x, const float y, const float z)
{
	// set game object's scaling

	scale_.x = x;
	scale_.y = y;
	scale_.z = z;

	// compute a scaling matrix for new scaling values
	scalingMatrix_ = DirectX::XMMatrixScaling(scale_.x, scale_.y, scale_.z);

	// compute the final world matrix for the game object
	this->UpdateMatrix();

	return;
}

///////////////////////////////////////////////////////////

void GameObject::SetRotationInRad(const float radiansX, const float radiansY, const float radiansZ)
{
	// set game object's rotation (takes angles in radians as input)
	// ATTENTION: rotation is performed around the corresponding axis;
	rotation_.x = radiansX;
	rotation_.y = radiansY;
	rotation_.z = radiansZ;

	// compute a rotation matrix for new rotation values
	rotationMatrix_ = DirectX::XMMatrixRotationRollPitchYaw(rotation_.x, rotation_.y, rotation_.z);

	// compute the final world matrix for the game object
	this->UpdateMatrix();

	// update the directions vectors of the game object
	this->UpdateDirectionsVectors();

	return;
}

///////////////////////////////////////////////////////////

void GameObject::SetRotationInDeg(const float angleX, const float angleY, const float angleZ)
{
	// set game object's rotation (takes angles in degrees as input);
	// ATTENTION: rotation is performed around the corresponding axis

	rotation_.x = DirectX::XMConvertToRadians(angleX);
	rotation_.y = DirectX::XMConvertToRadians(angleY);
	rotation_.z = DirectX::XMConvertToRadians(angleZ);

	// compute a rotation matrix for new rotation values
	rotationMatrix_ = DirectX::XMMatrixRotationRollPitchYaw(rotation_.z, rotation_.y, rotation_.x);

	// compute the final world matrix for the game object
	this->UpdateMatrix();

	// update the directions vectors of the game object
	this->UpdateDirectionsVectors();

	return;
}

///////////////////////////////////////////////////////////

void GameObject::AdjustRotationInRad(const float ax, const float ay, const float az)
{
	// this function adjusts game object's rotation (takes angles in radians as input)
	// ATTENTION: rotation is performed around the corresponding axis;
	rotation_.x += ax;
	rotation_.y += ay;
	rotation_.z += az;

	// compute a rotation matrix for new rotation values
	rotationMatrix_ = DirectX::XMMatrixRotationRollPitchYaw(rotation_.z, rotation_.y, rotation_.x);

	// compute the final world matrix for the game object
	this->UpdateMatrix();

	// update the directions vectors of the game object
	this->UpdateDirectionsVectors();
}

///////////////////////////////////////////////////////////

void GameObject::AdjustRotationInRad(const DirectX::XMFLOAT3 & angles)
{
	// this function adjusts game object's rotation (takes angles in radians as input)
	// ATTENTION: rotation is performed around the corresponding axis;
	rotation_.x += angles.x;
	rotation_.y += angles.y;
	rotation_.z += angles.z;

	// compute a rotation matrix for new rotation values
	rotationMatrix_ = DirectX::XMMatrixRotationRollPitchYaw(rotation_.z, rotation_.y, rotation_.x);

	// compute the final world matrix for the game object
	this->UpdateMatrix();

	// update the directions vectors of the game object
	this->UpdateDirectionsVectors();
}

///////////////////////////////////////////////////////////

void GameObject::AdjustRotationInDeg(const float ax, const float ay, const float az)
{
	// this function adjusts game object's rotation (takes angles in degrees as input);
	// ATTENTION: rotation is performed around the corresponding axis

	rotation_.x += DirectX::XMConvertToRadians(ax);
	rotation_.y += DirectX::XMConvertToRadians(ay);
	rotation_.z += DirectX::XMConvertToRadians(az);

	// compute a rotation matrix for new rotation values
	rotationMatrix_ = DirectX::XMMatrixRotationRollPitchYaw(rotation_.z, rotation_.y, rotation_.x);

	// compute the final world matrix for the game object
	this->UpdateMatrix();

	// update the directions vectors of the game object
	this->UpdateDirectionsVectors();
}

///////////////////////////////////////////////////////////

void GameObject::AdjustRotationInDeg(const DirectX::XMFLOAT3 & angles)
{
	// this function adjusts game object's rotation (takes angles in degrees as input);
	// ATTENTION: rotation is performed around the corresponding axis

	rotation_.x += DirectX::XMConvertToRadians(angles.x);
	rotation_.y += DirectX::XMConvertToRadians(angles.y);
	rotation_.z += DirectX::XMConvertToRadians(angles.z);

	// compute a rotation matrix for new rotation values
	rotationMatrix_ = DirectX::XMMatrixRotationRollPitchYaw(rotation_.z, rotation_.y, rotation_.x);

	// compute the final world matrix for the game object
	this->UpdateMatrix();

	// update the directions vectors of the game object
	this->UpdateDirectionsVectors();
}

///////////////////////////////////////////////////////////

void GameObject::SetColor(const float red, const float green, const float blue)
{
	// set a color

	color_.x = red;
	color_.y = green;
	color_.z = blue;

	return;
}

///////////////////////////////////////////////////////////

void GameObject::SetColor(const DirectX::XMFLOAT3 & color)
{
	// set a color
	color_ = color;
}

///////////////////////////////////////////////////////////

void GameObject::SetType(const GameObject::GameObjectType type)
{
	// set what kind of type this game object
	this->gameObjType_ = type;
}

///////////////////////////////////////////////////////////

void GameObject::UpdateMatrix()
{
	assert("UpdateMatrix must be overriden." && 0);
}

///////////////////////////////////////////////////////////

void GameObject::UpdateDirectionsVectors()
{
	
}

#endif