////////////////////////////////////////////////////////////////////
// Filename:     textclass.h
// Description:  (1) a DTO for data of the text;
//               (2) The SentenceType is the class that holds all
//                   the data for rendering the text;
//               (3) this class is used in the TextStore
//
// Revising:     01.06.23
////////////////////////////////////////////////////////////////////
#pragma once




class SentenceType final
{
public:
	SentenceType(const int stringSize,         // maximal size of the string
		const char* textContent,               // the content of the text
		const int posX, const int posY,        // upper left position of the text in the window
		const float red, const float green, const float blue)    // colour of the text
		: text_(textContent),
		  maxLength_(stringSize),
		  pos_{ posX, posY },
		  color_(red, green, blue)
	{
	}

	// memory allocation because of using the XM-formats

	void* operator new(std::size_t size)  // a replaceable allocation
	{
		if (void* ptr = _aligned_malloc(size, 16))
		{
			return ptr;
		}

		Log::Error("can't allocate the memory for the object");
		throw std::bad_alloc{};
	}

	// ordinary delete
	void operator delete(void* p) noexcept
	{
		_aligned_free(p);
	}

	//
	// GETTERS
	//
	const std::string & GetText() const
	{
		return text_;
	}

	const POINT & GetPosition() const
	{
		return pos_;
	}

	const size_t GetMaxTextLength() const
	{
		return maxLength_;
	}

	const DirectX::XMFLOAT3 & GetColor() const
	{
		return color_;
	}

	//
	// SETTERS
	//
	void SetText(const std::string & newText)
	{
		text_ = newText;
	}

	void SetColor(const DirectX::XMFLOAT3 & newColor)
	{
		color_ = newColor;
	}

private:
	std::string text_{ "" };                              // text content
	size_t  maxLength_ = 0;                               // maximal length of this sentence
	POINT pos_{ 0, 0 };                                   // the left upper position of the whole sentence on the screen
	DirectX::XMFLOAT3 color_{ 1.0f, 1.0f, 1.0f };         // colour of the sentence
};