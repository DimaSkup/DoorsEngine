#pragma once

#include "GuiComponent.h"
#include "../Texture/TextureHelperTypes.h"
#include "../Render/Color.h"

class GuiSprite : public GuiComponent
{
public:

	void SetupGuiSprite(
		const DirectX::XMFLOAT3& pos,
		const TexID textureID,
		Color& sprite,
		const float width = 0.0f,
		const float height = 0.0f);

	virtual void Update(const float delta);

private:

	// contains most of the information required to render the sprite which is 
	// setup inside the GuiSprite::SetupGuiSprite() function
	//VisualSprite sprite_;    
};