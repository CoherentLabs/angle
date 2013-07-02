// Copyright (c) 2013 Coherent Labs AD, Stoyan Nikolov. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Dx11State.h: Captures almost all Dx11 state

#ifndef LIBGLESV2_DX11STATE_H_
#define LIBGLESV2_DX11STATE_H_

#include "common/angleutils.h"

namespace rx
{

class DX11State
{
public:
	DX11State();
	void Capture(ID3D11DeviceContext* context);
	void Apply(ID3D11DeviceContext* context);

	void Release();
private:
	bool mHasCapture;
	ID3D11BlendState* mpBlendState;
	FLOAT mBlendFactor[4];
	UINT mSampleMask;

	ID3D11RasterizerState* mpRasterizerState;
	ID3D11DepthStencilState* mpDepthStencilState;
	UINT mStencilRef;

	ID3D11RenderTargetView* mppRenderTargetViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
	ID3D11DepthStencilView* mpDepthStencilView;

	template<typename ShaderType>
	class ShaderState
	{
	public:
		ShaderState();
		void Capture(ID3D11DeviceContext* context);
		void Apply(ID3D11DeviceContext* context);
		void Release();
	private:
		ShaderType* mpShader;
		ID3D11ClassInstance* mppShaderClassInstances[D3D11_SHADER_MAX_INTERFACES];
		UINT mShaderClassInstancesCount;
		ID3D11SamplerState* mppShaderSamplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		ID3D11ShaderResourceView* mppShaderResources[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
		ID3D11Buffer* mppConstantBuffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
	};

	ShaderState<ID3D11ComputeShader> mComputeShaderState;
	ShaderState<ID3D11DomainShader> mDomainShaderState;
	ShaderState<ID3D11VertexShader> mVertexShaderState;
	ShaderState<ID3D11HullShader> mHullShaderState;
	ShaderState<ID3D11GeometryShader> mGeometryShaderState;
	ShaderState<ID3D11PixelShader> mPixelShaderState;

	ID3D11InputLayout* mpInputLayout;

	ID3D11Buffer* mpIndexBuffer;
	DXGI_FORMAT mIndexBufferFormat;
	UINT mIndexBufferOffset;

	ID3D11Buffer* mpVertexBuffers[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	UINT mpVertexBufferStrides[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	UINT mpVertexBufferOffsets[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];

	D3D11_PRIMITIVE_TOPOLOGY mPrimitiveTopology;

	D3D11_VIEWPORT mpViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	UINT mViewportsCount;
	D3D11_RECT mpScissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	UINT mScissorRectsCount;
	
	DISALLOW_COPY_AND_ASSIGN(DX11State);
};

}
#endif // LIBGLESV2_DX11STATE_H_
