// =================================================================================
// Filename:     Camera.cpp
// Description:  an implementation of the Camera class;
// Revising:     25.11.22
// =================================================================================
#include "Camera.h"
#include "../Common/Log.h"


Camera::Camera() {}
Camera::~Camera() {}



// =================================================================================
//                             public methods
// =================================================================================

void Camera::HandleKeyboardEvents(const float dt)
{
	// handle pressing of W,A,S,D,space,Z keys for changing of the camera position;
	// dt - delta time

	using namespace DirectX;

	BYTE keyboardState[256];
	GetKeyboardState(keyboardState);

	isRunning_ = (1 < keyboardState[VK_LSHIFT]);


	const float speed = (isRunning_) ? runSpeed_ : walkSpeed_;  // foot speed
	const float xDistToPeak = 0.25f * speed;

	// control jump with these two variables
	const float  th = 0.3f;           // time to reach a peak
	const float  jumpHeight = 1.0f;   // how much we can jump up

	static bool  onGround = true;     // are we on the ground?
	static float velocityY = 5.0f;    // current velocity
	static float endVelocity = 0.0f;  // when velocity drop to this value we are on the ground
	static float gravity = 9.8f;      // accumulation


	// handle the position changes
	DirectX::XMVECTOR moveVec{ 0, 0, 0 };

	// move forward
	if (1 < keyboardState['W'])
		moveVec += GetForwardVector();

	// move backward
	if (1 < keyboardState['S'])
		moveVec += GetBackwardVector();

	// move left
	if (1 < keyboardState['A'])
		moveVec += GetLeftVector();

	// move right
	if (1 < keyboardState['D'])
		moveVec += GetRightVector();


	float& dx = moveVec.m128_f32[0];
	float& dy = moveVec.m128_f32[1];
	float& dz = moveVec.m128_f32[2];

#define APPLY_JUMP false
#if APPLY_JUMP 


	

	// space is pressed (jump)
	if (1 < keyboardState[' '])
	{
		if (onGround)
		{
			onGround = false;
			velocityY = (2.0f * jumpHeight) / th;           // v0 = 2h / th
			endVelocity = -velocityY;
			gravity = -(2.0f * jumpHeight) / (th * th);       // g = -2h / (th^2)
		}
	}

	// compute jumping (if necessary)
	if (!onGround)
	{
		dy = velocityY * dt + 0.5f * gravity * dt * dt;
		velocityY += gravity * dt;

		if (velocityY <= endVelocity)
		{
			dy = 0.0f;
			onGround = true;
		}
	}

#else
	// free camera mode
	if (1 < keyboardState[' '])
	{
		// pressed space (move up: increase Y)
		dy += (speed * dt);
	}
#endif

	// move down (decrease Y)
	if (1 < keyboardState['Z'])
		dy -= speed * dt;

	// update the position of the camera
	float speedCorrection = speed * dt;
	AdjustPosition(moveVec * DirectX::XMVECTOR{ speedCorrection, 1.0f, speedCorrection });
} 

///////////////////////////////////////////////////////////

void Camera::HandleMouseMovement(
	const int yawDelta,
	const int pitchDelta,
	const float deltaTime)
{
	// this function handles the changing of the camera rotation

	// update the value of camera angles based on input
	static float pitch = 0.0f;
	static float yaw = 0.0f;
	const float speedMulDelta = this->rotationSpeed_ * deltaTime;

	// make each pixel correspond to a quarter of a degree
	const float dx = DirectX::XMConvertToRadians(0.25f * static_cast<float>(yawDelta));
	const float dy = DirectX::XMConvertToRadians(0.25f * static_cast<float>(pitchDelta));
	
	yaw   += dx * speedMulDelta;     // right/left movement
	pitch += dy * speedMulDelta;     // up/down movement

	// limit the pitch value in range (-(PI/2)+0.1f < pitch < (PI/2)-0.1f)
	if (pitch > DirectX::XM_PIDIV2 - 0.1f)
	{
		pitch = DirectX::XM_PIDIV2 - 0.1f;
	}
	else if (pitch < -DirectX::XM_PIDIV2 + 0.1f)
	{
		pitch = -DirectX::XM_PIDIV2 + 0.1f;
	}

	// limit the yaw value in range (-2PI < yaw < 2PI)
	if (yaw > DirectX::XM_2PI)
	{
		yaw = -DirectX::XM_2PI;
	}
	else if (yaw < -DirectX::XM_2PI)
	{
		yaw = DirectX::XM_2PI;
	}
	
	// update the rotation angle
	SetRotationInRad({ pitch, yaw, 0.0f });
}