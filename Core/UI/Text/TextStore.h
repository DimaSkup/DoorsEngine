////////////////////////////////////////////////////////////////////////////////////////////
// Filename:     textclass.h
// Description:  handles all the 2D text drawing that the application
//               will need. It renders 2D text to the screen.
//               It uses FontClass to create the vertex buffer for strings
//               and then uses FontShaderClass to render this buffer;
//
// Revising:     04.06.22
////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////////////////////////
// INCLUDES
//////////////////////////////////
#include "fontclass.h"              // text font

#include "../../Common/Types.h"

#include "../../Mesh/Vertex.h"
#include "../../Mesh/VertexBuffer.h"
#include "../../Mesh/IndexBuffer.h"

#include "../../Engine/SystemState.h"



#include <map>
#include <vector>
#include <DirectXMath.h>


// Forward declaration
//
// Stores one frame transient data. This is intermediate data used by the update
// pipeline every frame and discarded at the end of the frame.
// Note that this struct is defined in the .cpp file because only
// the store needs to see it.
/*
namespace TextDetails
{
	struct VerticesArr
	{
		std::vector<VertexFont> vertices_;
	};

	struct TextStoreTransientData 
	{
		std::vector<UINT> stringsToUpdate_;
		std::vector<POINT> positionsToUpdate_;
		std::vector<VerticesArr> verticesToUpdate_;

		void Clear()
		{
			stringsToUpdate_.clear();
			positionsToUpdate_.clear();
			verticesToUpdate_.clear();
		}
	};
}
*/

// Stores all text related data
class TextStore final
{
public:

	TextStore();	
	~TextStore();

	// restrict a copying of this class instance
	TextStore(const TextStore& obj) = delete;
	TextStore& operator=(const TextStore& obj) = delete;

	// ---------------------------------------------

	SentenceID CreateConstSentence(
		ID3D11Device* pDevice,
		FontClass& font,                     // font for the text
		const std::string& textContent,      // the content of the text
		const DirectX::XMFLOAT2& drawAt);    // upper left corner of the sentence

	SentenceID CreateSentence(
		ID3D11Device* pDevice,
		FontClass& font,      
		const std::string& textContent,       
		const size maxStrSize,
		const DirectX::XMFLOAT2& drawAt,
		const bool isDynamic);

	void SetKeyByID(const std::string& key, const SentenceID id);

	void GetRenderingData(
		std::vector<ID3D11Buffer*>& outVbPtrs,
		std::vector<ID3D11Buffer*>& outIbPtrs,
		std::vector<u32>& outIndexCounts);

	void Update(
		ID3D11DeviceContext* pContext,
		FontClass& font,
		const SystemState& systemState);

	void UpdateSentenceByKey(
		ID3D11DeviceContext* pContext,
		FontClass& font,
		const std::string& key,            // semantic key
		const std::string& newStr);

private:

	void BuildTextVerticesIndices(
		ID3D11Device* pDevice,
		const size maxStrSize,                             // maximal size for this string (if it will be bigger we will have a vertex buffer overflow)
		const std::string& textContent,               
		const DirectX::XMFLOAT2& drawAt,                   // upper left position of the str
		FontClass & font,                                  // font for the text
		std::vector<VertexFont>& vertices,
		std::vector<UINT>& indices);

	void UpdateSentenceByIdx(
		ID3D11DeviceContext* pContext,
		FontClass& font,
		const index idx,
		const std::string& newStr);

private:
	static SentenceID staticID_;

	std::vector<SentenceID>               ids_;
	std::vector<std::string>              textContent_;

	// semantic keys for navigation (is useful to find a str to update it);
	// these keys are set manually using the SetKeyByID() method;
	std::map<std::string, SentenceID>     keyToID_;       

	std::vector<DirectX::XMFLOAT2>        drawAt_;         // upper left corner of sentence
	std::vector<size>                     maxStrSize_;     // maximal number of vertices per each string
	std::vector<bool>                     isDynamic_;      // is this str modifiable?

	std::vector<VertexBuffer<VertexFont>> vertexBuffers_;
	std::vector<IndexBuffer<UINT>>        indexBuffers_;

	//std::unique_ptr<TextDetails::TextStoreTransientData> pDataToUpdate_;
};