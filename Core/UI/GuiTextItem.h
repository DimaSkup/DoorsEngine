// =================================================================================
// Filename:     GuiTextItem.h
// Description:  stores text data for rendering;
//               NOTE: the class only accepts wide chars strings to display text.
//               These multi-byte characters enable us to display chars from 
//               multiple languages, though various platforms may endian-swap
//               the order of the bytes.
// 
// =================================================================================
#pragma once

#include "GuiComponent.h"

class GuiTextItem : public GuiComponent
{
#if 0
public:
	void SetupGuiTextItem(const char* text, Font* pFont);

	float GetTextWidth() const;
	float GetTextHeight() const;

	void SetText(const char* text);

	// gets the calculate width and height of the text based on
	// the text and the font. This becomes useful when we want to allow
	// a piece of text to be selected with a pointing device
	void SetAlignment(GuiTextHAlign hAlign, GuiTextVAlign vAlign);

	void SetColor(const Color& color);
	virtual void Update(const float deltaTime);

private:
	float xScale_;
	float yScale_;

	VisualText visualText_;
#endif
};
