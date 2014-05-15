#include "precompiled.h"

// Copyright (c) 2013 Coherent Labs AD, Stoyan Nikolov. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libGLESv2/renderer/Dx11State.h"

namespace rx
{

template<typename ShaderType>
DX11State::ShaderState<ShaderType>::ShaderState()
	: mShaderClassInstancesCount(0)
{}

#define __SAFE_RELEASE(VALUE) if(VALUE) { VALUE->Release(); VALUE = NULL; }
template<typename ShaderType>
void DX11State::ShaderState<ShaderType>::Release()
{
	__SAFE_RELEASE(mpShader);
	for (UINT i = 0; i < mShaderClassInstancesCount; ++i)
	{
		__SAFE_RELEASE(mppShaderClassInstances[i]);
	}
	for (UINT i = 0; i < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT; ++i)
	{
		__SAFE_RELEASE(mppShaderSamplers[i]);
	}
	for (UINT i = 0; i < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT; ++i)
	{
		__SAFE_RELEASE(mppShaderResources[i]);
	}
	for (UINT i = 0; i < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT; ++i)
	{
		__SAFE_RELEASE(mppConstantBuffers[i]);
	}
}

#define DEF_CAPTURE_STATE(TYPE, PREF)																				\
template<>																											\
void DX11State::ShaderState<TYPE>::Capture(ID3D11DeviceContext* context)								\
{																													\
	mShaderClassInstancesCount = D3D11_SHADER_MAX_INTERFACES;														\
	context->PREF##GetShader(&mpShader, mppShaderClassInstances, &mShaderClassInstancesCount);						\
	context->PREF##GetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, mppShaderSamplers);						\
	context->PREF##GetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, mppShaderResources);			\
	context->PREF##GetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT , mppConstantBuffers);	\
}

#define DEF_APPLY_STATE(TYPE, PREF)																					\
template<>																											\
void DX11State::ShaderState<TYPE>::Apply(ID3D11DeviceContext* context)									\
{																													\
	context->PREF##SetShader(mpShader, mppShaderClassInstances, mShaderClassInstancesCount);						\
	context->PREF##SetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, mppShaderSamplers);						\
	context->PREF##SetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, mppShaderResources);			\
	context->PREF##SetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT , mppConstantBuffers);	\
}																													

#define DEF_STATE(TYPE, PREF) \
	DEF_CAPTURE_STATE(TYPE, PREF) \
	DEF_APPLY_STATE(TYPE, PREF)

DEF_STATE(ID3D11ComputeShader, CS)
DEF_STATE(ID3D11DomainShader, DS)
DEF_STATE(ID3D11VertexShader, VS)
DEF_STATE(ID3D11GeometryShader, GS)
DEF_STATE(ID3D11PixelShader, PS)
DEF_STATE(ID3D11HullShader, HS)

DX11State::DX11State()
	: mHasCapture(false)
	, mViewportsCount(0)
	, mScissorRectsCount(0)
{}

void DX11State::Capture(ID3D11DeviceContext* context)
{
	context->OMGetBlendState(&mpBlendState, mBlendFactor, &mSampleMask);
	context->OMGetDepthStencilState(&mpDepthStencilState, &mStencilRef);
	context->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, mppRenderTargetViews, &mpDepthStencilView);
	context->RSGetState(&mpRasterizerState);
	context->IAGetPrimitiveTopology(&mPrimitiveTopology);
	context->IAGetIndexBuffer(&mpIndexBuffer, &mIndexBufferFormat, &mIndexBufferOffset);
	context->IAGetInputLayout(&mpInputLayout);
	context->IAGetVertexBuffers(0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT, mpVertexBuffers, mpVertexBufferStrides, mpVertexBufferOffsets);
	mViewportsCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	context->RSGetViewports(&mViewportsCount, mpViewports);
	mScissorRectsCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	context->RSGetScissorRects(&mScissorRectsCount, mpScissorRects);

	mComputeShaderState.Capture(context);
	mDomainShaderState.Capture(context);
	mVertexShaderState.Capture(context);
	mGeometryShaderState.Capture(context);
	mHullShaderState.Capture(context);
	mPixelShaderState.Capture(context);

	mHasCapture = true;
}

void DX11State::Apply(ID3D11DeviceContext* context)
{
	if(!mHasCapture)
		return;

	// Apply saved state
	context->OMSetBlendState(mpBlendState, mBlendFactor, mSampleMask);
	context->OMSetDepthStencilState(mpDepthStencilState, mStencilRef);
	context->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, mppRenderTargetViews, mpDepthStencilView);
	context->RSSetState(mpRasterizerState);
	context->IASetPrimitiveTopology(mPrimitiveTopology);
	context->IASetIndexBuffer(mpIndexBuffer, mIndexBufferFormat, mIndexBufferOffset);
	context->IASetInputLayout(mpInputLayout);
	context->IASetVertexBuffers(0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT, mpVertexBuffers, mpVertexBufferStrides, mpVertexBufferOffsets);
	context->RSSetViewports(mViewportsCount, mpViewports);
	context->RSSetScissorRects(mScissorRectsCount, mpScissorRects);

	mComputeShaderState.Apply(context);
	mDomainShaderState.Apply(context);
	mVertexShaderState.Apply(context);
	mGeometryShaderState.Apply(context);
	mHullShaderState.Apply(context);
	mPixelShaderState.Apply(context);

	Release();
}

void DX11State::Release()
{
	if(!mHasCapture)
		return;

	mComputeShaderState.Release();
	mDomainShaderState.Release();
	mVertexShaderState.Release();
	mGeometryShaderState.Release();
	mHullShaderState.Release();
	mPixelShaderState.Release();

	// Release references
	__SAFE_RELEASE(mpBlendState);
	__SAFE_RELEASE(mpDepthStencilState);
	__SAFE_RELEASE(mpRasterizerState);
	__SAFE_RELEASE(mpIndexBuffer);
	__SAFE_RELEASE(mpInputLayout);
	__SAFE_RELEASE(mpDepthStencilView);
	for (UINT i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
	{
		__SAFE_RELEASE(mppRenderTargetViews[i]);
	}
	for (UINT i = 0; i < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT; ++i)
	{
		__SAFE_RELEASE(mpVertexBuffers[i]);
	}
}

}
