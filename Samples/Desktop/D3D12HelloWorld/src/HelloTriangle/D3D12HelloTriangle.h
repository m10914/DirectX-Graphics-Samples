//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

#include "DXSample.h"

#include <dxcapi.h>
#include <vector>

#include "nv_helpers_dx12/TopLevelASGenerator.h"
#include "nv_helpers_dx12/ShaderBindingTableGenerator.h"

using namespace DirectX;
typedef UINT uint;
typedef ID3D12Resource id3d12resource;
typedef ID3D12RootSignature id3d12rootsignature;
typedef ID3D12StateObjectProperties id3d12stateobjectproperties;
typedef ID3D12StateObject id3d12stateobject;
typedef IDxcBlob idxcblob;
typedef ID3D12DescriptorHeap id3d12descriptorheap;


// #DXR
struct AccelerationStructureBuffers
{
	ComPtr<id3d12resource> pScratch;      // Scratch memory for AS builder
	ComPtr<id3d12resource> pResult;       // Where the AS is
	ComPtr<id3d12resource> pInstanceDesc; // Hold the matrices of the instances
};


// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

class D3D12HelloTriangle : public DXSample
{
public:
    D3D12HelloTriangle(UINT width, UINT height, std::wstring name);

    virtual void OnInit();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnDestroy();

private:
    static const UINT FrameCount = 2;

    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT4 color;
    };

    // Pipeline objects.
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12Device5> m_device;
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12PipelineState> m_pipelineState;
    ComPtr<ID3D12GraphicsCommandList4> m_commandList;
    UINT m_rtvDescriptorSize;

    // App resources.
    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    // Synchronization objects.
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;

    // #DXR
    bool m_raster = true;

    ComPtr<ID3D12Resource> m_bottomLevelAS;
	AccelerationStructureBuffers m_topLevelASBuffers;
	std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX >> m_instances;
	ComPtr<idxcblob> m_rayGenLibrary;
	ComPtr<idxcblob> m_hitLibrary;
	ComPtr<idxcblob> m_missLibrary;

	ComPtr<id3d12rootsignature> m_rayGenSignature;
	ComPtr<id3d12rootsignature> m_hitSignature;
	ComPtr<id3d12rootsignature> m_missSignature;

	// Ray tracing pipeline state
	ComPtr<id3d12stateobject> m_rtStateObject;
	// Ray tracing pipeline state properties, retaining the shader identifiers
	// to use in the Shader Binding Table
	ComPtr<id3d12stateobjectproperties> m_rtStateObjectProps;

	ComPtr<id3d12resource> m_outputResource;
	ComPtr<id3d12descriptorheap> m_srvUavHeap;
	ComPtr<id3d12resource> m_sbtStorage;

    nv_helpers_dx12::ShaderBindingTableGenerator m_sbtHelper;
    nv_helpers_dx12::TopLevelASGenerator m_topLevelASGenerator;

    //--------------------------------------------------------------------------
    // methods

    void LoadPipeline();
    void LoadAssets();
    void PopulateCommandList();
    void WaitForPreviousFrame();
    void CheckRaytracingSupport();

    virtual void OnKeyUp(UINT8 key);

    // #DXR
	/// Create the acceleration structure of an instance
    ///
    /// \param     vVertexBuffers : pair of buffer and vertex count
    /// \return    AccelerationStructureBuffers for TLAS
	AccelerationStructureBuffers CreateBottomLevelAS(std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t >> vVertexBuffers);

	/// Create the main acceleration structure that holds
	/// all instances of the scene
	/// \param     instances : pair of BLAS and transform
	void CreateTopLevelAS(const std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX >> &instances);

	/// Create all acceleration structures, bottom and top
	void CreateAccelerationStructures();

    void CreateRaytracingPipeline();

	ComPtr<id3d12rootsignature> CreateRayGenSignature();
	ComPtr<id3d12rootsignature> CreateMissSignature();
	ComPtr<id3d12rootsignature> CreateHitSignature();

	void CreateRaytracingOutputBuffer();
	void CreateShaderResourceHeap();
	void CreateShaderBindingTable();
};
