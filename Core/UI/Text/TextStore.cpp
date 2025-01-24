// =================================================================================
// Filename: textclass.cpp
// Revising: 04.07.22
// =================================================================================
#include "TextStore.h"

#include "../../Common/Log.h"
#include "../../Common/Utils.h"

#include <algorithm>
#include <stdexcept>

// static
SentenceID TextStore::staticID_ = 0;



TextStore::TextStore() 
{
}

TextStore::~TextStore() 
{
	Log::Debug(); 
}



// =================================================================================
//                             public modification API
// =================================================================================

SentenceID TextStore::CreateConstSentence(
	ID3D11Device* pDevice,
	FontClass& font,                     // font for the text
	const std::string& textContent,      // the content of the text
	const DirectX::XMFLOAT2& drawAt)
{
	// the content of this string won't be changed;
	// you can only change its position on the screen;

	try
	{
		Assert::True(!textContent.empty(), "the input string is empty");

		// add ID for this new sentence
		SentenceID id = staticID_++;
		ids_.push_back(id);

		// upper left rendering pos
		drawAt_.push_back({ (float)drawAt.x, (float)drawAt.y });

		maxStrSize_.push_back(std::ssize(textContent));
		textContent_.push_back(textContent);

		std::vector<VertexFont> vertices;
		std::vector<UINT> indices;

		BuildTextVerticesIndices(
			pDevice,
			maxStrSize_.back(),
			textContent,
			drawAt,
			font,
			vertices,
			indices);

		// initialize the VB/IB for this text string
		const bool isDynamic = false;
		vertexBuffers_.emplace_back(pDevice, vertices.data(), (int)vertices.size(), isDynamic);
		indexBuffers_.emplace_back(pDevice, indices.data(), (int)indices.size());

		return ids_.back();
	}
	catch (EngineException& e)
	{
		Log::Error(e, false);
		throw EngineException("can't create a sentence with the text: " + textContent);
	}
}

///////////////////////////////////////////////////////////

SentenceID TextStore::CreateSentence(
	ID3D11Device* pDevice,
	FontClass& font,                     // font for the text
	const std::string& textContent,      // the content of the text
	const size maxStrSize,               // maximal length for this string
	const DirectX::XMFLOAT2& drawAt,     // upper left position of the text in the window
	const bool isDynamic)                // will this sentence be changed from frame to frame?
{
	// the content of this string is supposed to be changed from frame to frame;
	// you can also change its position on the screen;

	try
	{
		// check input params
		Assert::True(!textContent.empty(), "the input string is empty");
		Assert::True(maxStrSize >= std::ssize(textContent), "max string size must be >= input text string");

		// add ID for this new sentence
		SentenceID id = staticID_++;
		ids_.push_back(id);

		// upper left rendering pos
		drawAt_.push_back({ (float)drawAt.x, (float)drawAt.y });
		   
		maxStrSize_.push_back(maxStrSize);       
		textContent_.push_back(textContent);

		std::vector<VertexFont> vertices;
		std::vector<UINT> indices;
		
		BuildTextVerticesIndices(
			pDevice,
			maxStrSize,
			textContent,
			drawAt,
			font, 
			vertices,
			indices);

		// initialize the VB/IB for this text string
		vertexBuffers_.emplace_back(pDevice, vertices.data(), (int)vertices.size(), isDynamic);
		indexBuffers_.emplace_back(pDevice, indices.data(), (int)indices.size());

		return id;
	}
	catch (EngineException & e)
	{
		Log::Error(e, false);
		throw EngineException("can't create a sentence with the text: " + textContent);
	}
}

///////////////////////////////////////////////////////////

void TextStore::SetKeyByID(const std::string& key, const SentenceID id)
{
	// create a semantic text key for the input sentence ID;
	// so we will be able to get this ID by key;
	const auto result = keyToID_.insert({ key, id });

	if (!result.second)
		Log::Error("can't set a key (" + key + ") by id (" + std::to_string(id));
}

///////////////////////////////////////////////////////////

void TextStore::GetRenderingData(
	std::vector<ID3D11Buffer*>& outVbPtrs,
	std::vector<ID3D11Buffer*>& outIbPtrs,
	std::vector<u32>& outIndexCounts)

{
	// get VB, IB, and index count of each sentence from the storage

	try
	{
		const size numBuffers = std::ssize(vertexBuffers_);

		outVbPtrs.resize(numBuffers);
		outIbPtrs.resize(numBuffers);
		outIndexCounts.resize(numBuffers);

		for (int i = 0; VertexBuffer<VertexFont>& vb : vertexBuffers_)
			outVbPtrs[i++] = vb.Get();

		for (int i = 0; IndexBuffer<UINT>& ib : indexBuffers_)
			outIbPtrs[i++] = ib.Get();

		for (int i = 0; IndexBuffer<UINT>& ib : indexBuffers_)
			outIndexCounts[i++] = ib.GetIndexCount();
	}
	catch (EngineException & e)
	{
		Log::Error(e, false);
		throw EngineException("can't render the sentence");
	}
}



// ====================================================================================
//                               PUBLIC UPDATE API
// ====================================================================================

void TextStore::Update(
	ID3D11DeviceContext* pContext,
	FontClass& font,
	const SystemState& sysState)

{
	// Update() changes the contents of the dynamic vertex buffer for the input text.

	try
	{
		// sentences by these keys are supposed to update if its values are
		// differ from the sysState
		std::vector<std::string> keys =
		{
			"fps", "frame_time",

			"posX",	"posY",	"posZ",  // position info
			"rotX", "rotY", "rotZ",  // rotation info

			// render info
			"models_drawn", "vertices_drawn", "faces_drawn",
			"cells_drawn", "cells_culled",
		};

		const size numDynamicSentences = std::ssize(keys);

		std::vector<SentenceID> ids(numDynamicSentences);
		std::vector<index> idxs(numDynamicSentences);
		std::vector<std::string> values(numDynamicSentences);

		// get ids by keys
		for (int i = 0; const std::string& key : keys)
			ids[i++] = keyToID_.at(key);

		CoreUtils::GetIdxsInSortedArr(ids_, ids, idxs);
		
		// set values so later we will compare them to the previous
		values[0] = std::to_string(sysState.fps);
		values[1] = std::to_string(sysState.frameTime);

		// pos info
		values[2] = std::to_string(sysState.CameraPos.x);
		values[3] = std::to_string(sysState.CameraPos.y);
		values[4] = std::to_string(sysState.CameraPos.z);

		// rotation info
		values[5] = std::to_string(sysState.CameraDir.x);
		values[6] = std::to_string(sysState.CameraDir.y);
		values[7] = std::to_string(sysState.CameraDir.z);

		// render info
		values[8]  = std::to_string(sysState.visibleObjectsCount);
		values[9]  = std::to_string(sysState.visibleVerticesCount);
		values[10] = std::to_string(sysState.visibleVerticesCount / 3);
		values[11] = std::to_string(sysState.cellsDrawn);
		values[12] = std::to_string(sysState.cellsCulled);
		

		// update the sentence if necessary
		for (index i = 0; i < numDynamicSentences; ++i)
		{
			const index idx = idxs[i];

			if (textContent_[idx] != values[i])
				UpdateSentenceByIdx(pContext, font, idx, values[i]);
		}
	}
	catch (const std::out_of_range& e)
	{
		Log::Error(e.what());
		Log::Error("can't find an ID by key");
	}
	catch (EngineException& e)
	{
		Log::Error(e);
		Log::Error("failed to update the text vertex buffer with new data");
		throw EngineException("can't update the sentence");
	}
}

///////////////////////////////////////////////////////////

void TextStore::UpdateSentenceByKey(
	ID3D11DeviceContext* pContext,
	FontClass& font,
	const std::string& key,            // semantic key
	const std::string& newStr)
{
	try
	{
		// get ids by keys
		SentenceID id = keyToID_.at(key);
		index idx = CoreUtils::GetIdxInSortedArr(ids_, id);

		UpdateSentenceByIdx(pContext, font, idx, newStr);
	}
	catch (const std::out_of_range& e)
	{
		Log::Error(e.what());
		Log::Error("there is no sentece by key: " + key);
	}
}





// ====================================================================================
//                            PRIVATE MODICATION API 
// ====================================================================================


void TextStore::BuildTextVerticesIndices(
	ID3D11Device* pDevice,
	const size maxStrSize,
	const std::string& textContent,
	const DirectX::XMFLOAT2& drawAt,
	FontClass& font,                      // font for the text
	std::vector<VertexFont>& vertices,
	std::vector<UINT>& indices)
{ 
	// THIS FUNC builds a vertex and index buffer for the input string by its 
	// textContent and places its vertices at the drawAt position;

	try
	{
		Assert::True(!textContent.empty(), "the input str is empty");
		Assert::True(maxStrSize >= std::ssize(textContent), "maxStrSize must be >= sentence size");

		const size numVerticesInSymbol = 4;
		const size numIndicesInSymbol  = 6;
		const size numVertices         = maxStrSize * numVerticesInSymbol;
		const size numIndices          = maxStrSize * numIndicesInSymbol;

		vertices.resize(numVertices);
		indices.resize(numIndices, 0);
		
		// fill in vertex and index arrays with initial data
		font.BuildVertexArray(vertices.data(), numVertices, textContent, drawAt);
		font.BuildIndexArray(indices.data(), numIndices);
	}
	catch (EngineException & e)
	{
		Log::Error(e);
		throw EngineException("can't build buffers for the sentence: " + textContent);
	}
}

///////////////////////////////////////////////////////////

void TextStore::UpdateSentenceByIdx(
	ID3D11DeviceContext* pContext,
	FontClass& font,
	const index idx,
	const std::string& newStr)
{
	// update the sentence by idx with new content;
	// also we rebuild its vertices according to this new content
	// and update its VB

	textContent_[idx] = newStr;

	std::vector<VertexFont> vertices(maxStrSize_[idx] * 4);

	// rebuild vertices for this sentence
	font.BuildVertexArray(
		vertices.data(),
		std::ssize(vertices),
		newStr,
		drawAt_[idx]);

	// update VB with new vertices
	vertexBuffers_[idx].UpdateDynamic(
		pContext,
		vertices.data(),
		std::ssize(vertices));
}

///////////////////////////////////////////////////////////
