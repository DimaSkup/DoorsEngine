// =================================================================================
// Filename:    GuiComponent.h
// 
// Created:     26.12.24
// =================================================================================
#pragma once

#include "../Common/Types.h"

#include <cstdint>
#include <string>
#include <DirectXMath.h>

class GuiComponent
{
	using u32 = uint32_t;
	using TypeID = uint32_t;

public:

	GuiComponent();
	virtual ~GuiComponent();

	virtual TypeID GetTypeID() const = 0;
	static TypeID GetStaticTypeID();

	u32 GetID() const;

	void SetupGuiComponent(const std::string& name, bool active = true);
	//void SetVisual(Visual* pVisual, bool makeVisible = true);
	void SetPosition(const DirectX::XMFLOAT3& pos);

	virtual void Update(const float deltaTime) = 0;
	virtual void Render();

private:
	static TypeID sTypeID_;
	std::string   name_;
	u32           id_;

	// a node that specifies a local translation to apply to this object, as well
	// as its position inside a hierarchy. This allows to attach together 
	// GuiComponents into groups which can the be moved togethere as a whole.
	//Frame         frame_;             

	//Visual*       pVisual_ = nullptr;

	// is the GuiComponent taking input and being updated etc.?
	bool          active_;

	// default to invisible false until we have a valid Visual
	bool          visible_;
};
