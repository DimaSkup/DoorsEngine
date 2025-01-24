////////////////////////////////////////////////////////////////////////////////////////////
// Filename:      SkyPlaneClass.h
// Description:   encapsulates everything related to the plane used
//                for rendering the clouds. It holds the geometry for
//                the sky plane, the two bitmaps textures for the 
//                clouds, and all the variables for the shader that
//                relate to how to draw the sky plane.
// 
// Created:       24.06.23
////////////////////////////////////////////////////////////////////////////////////////////
#pragma once


//////////////////////////////////
// Class name: SkyPlaneClass.h
//////////////////////////////////
class SkyPlaneClass final
{
#if 0
public:
	SkyPlaneClass(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
	~SkyPlaneClass();

	virtual bool Initialize(const std::string & filePath) override;

	void Update(float deltaTime);

	bool LoadCloudTextures(ID3D11Device* pDevice,
		const std::string & textureFilename1,
		const std::string & textureFilename2);

	const std::vector<float> & GetTranslationData() const;
	const float GetBrightness() const;

private:  // restrict a copying of this class instance
	SkyPlaneClass(const SkyPlaneClass & obj);
	SkyPlaneClass & operator= (const SkyPlaneClass & obj);

private:
	bool BuildSkyPlaneGeometry(ID3D11Device* pDevice,
		const int skyPlaneResolution, 
		const float skyPlaneWidth, 
		const float skyPlaneTop,
		const float skyPlaneBottom,
		const int textureRepeat);

	bool FillSkyPlaneArrays(ID3D11Device* pDevice,
		const int skyPlaneResolution,
		std::vector<Vertex3D> & verticesArr,
		std::vector<UINT> & indicesArr);
private:
	// clouds translation data where index: 
	// 0 - 1st cloud X-axis; 1 - 1st cloud Z-axis; 
	// 2 - 2nd cloud X-axis; 3 - 2nd cloud Z-axis;
	std::vector<float> translationSpeed_ { 0, 0, 0, 0 };

	// the current translation for the two textures
	std::vector<float> textureTranslation_{ 0, 0, 0, 0 };

	// the brightness of the clouds is stores here 
	// and set in the pixel shader during rendering
	float brightness_ = 0.0f;        

	Vertex3D* pSkyPlaneRawData_ = nullptr;
#endif
};
