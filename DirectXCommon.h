#pragma once
#include "WinApp.h"
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <array>
#include "StringUtility.h"
#include "Logger.h"
#include "externals/DirectXTex/DirectXTex.h"
#include <dxcapi.h>

class DirectXCommon
{
public:


    void Initialize(WinApp* winApp);

    // デバイスの生成
    void CreateDevice();
    // コマンド関連の初期化
    void InitializeCommandObjects();
    // スワップチェーンの生成
    void CreateSwapChain();
    // 深度バッファの生成
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthBuffer();
    // 各種デスクリプタヒープの生成
    void CreateDescriptorHeaps();
    // レンダーターゲットビューの初期化
    void InitializeRenderTargetView();
    // 深度ステンシルビューの初期化
    void InitializeDepthStencilView();
    // フェンスの初期化
    void InitializeFence();
    // ビューポート矩形の初期化
    void InitializeViewportAndScissorRect();
    // シザリング矩形の初期化
    void InitializeScissorRect();
    // DXCコンパイラの生成
    void CreateDXCCompiler();
    // ImGuiの初期化
    void InitializeImGui();
    //描画前処理
    void PreDraw();
    //描画後処理
    void PostDraw();

    //Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metadata);
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12Device> device,
        D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

    /// <summary>
    /// 指定番号のCPUデスクリプタハンドルを取得する
    /// </summary>
    static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);

    /// <summary>
    /// 指定番号のGPLUデスクリプタハンドルを取得する
    /// </summary>
    static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap,
        uint32_t descriptorSize, uint32_t index);
    ///<summary>
    ///SRVの指定番号のCPUデスクリプタハンドルを取得する    
    /// </summary>
    D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);
    /// <summary>
    /// SRVの指定番号のGPUデスクリプタハンドルを取得する
    /// </summary>
    D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);


    //Getter
    Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const { return device; }
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() const { return commandList; }
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDSVDescriptorHeap() const { return dsvDescriptorHeap; }
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetRTVDescriptorHeap() const { return rtvDescriptorHeap; }
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetSRVDescriptorHeap() const { return srvDescriptorHeap; }
    uint32_t GetDescriptorSizeRTV() const { return descriptorSizeRTV; }
    uint32_t GetDescriptorSizeSRV() const { return descriptorSizeSRV; }

    Microsoft::WRL::ComPtr<IDxcBlob> compileShader(const std::wstring& filePath, const wchar_t* profile);

    /// <summary>
    /// バッファリソースの生成
    /// </summary>
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

    /// <summary>
    /// テクスチャリソースの生成
    /// </summary>
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(
        Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metadata);
    /// <summary>
    /// テクスチャデータの転送
    /// </summary>

    [[nodiscard]]
    Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages);

    /// <summary>
    /// テクスチャファイルの読み込み
    /// </summary>
    /// <param name="filePath">テクスチャファイルのパス</param>
    /// <returns>画像イメージデータ</returns>
    static DirectX::ScratchImage LoadTexture(const std::string& filePath);

    /// <summary>
    /// 深度ステンシルテクスチャリソースの生成
    /// 
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height);

    //コマンドキュー生成
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
    //コマンドアロケータを生成する
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>commandAllocator = nullptr;
    //コマンドリストを生成する
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
private:

    //// デバイスの生成
    //Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
    //Microsoft::WRL::ComPtr<IDXGIAdapter4> useadapter = nullptr;
    //dxgiFactoryの生成

    Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;
    //Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;


    //SwapChain
    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
    //D3D12_RESOURCE_DESC depthStencilDesc = {};
    //swapchainからリソースを引っ張る
    std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    //RTVを二つ作るのでディスクリプタを二つ用意
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = nullptr;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = nullptr;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = nullptr;

    Microsoft::WRL::ComPtr <ID3D12Resource> depthStencilResource;

    uint32_t descriptorSizeSRV = 0;
    uint32_t descriptorSizeRTV = 0;
    uint32_t descriptorSizeDSV = 0;

    //DXCの初期化
    Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils = nullptr;
    Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler = nullptr;
    //include対応のため設定しておく
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler = nullptr;

    //フェンスの生成
    Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
    UINT64 fenceValue = 0;
    HANDLE fenceEvent = nullptr;

    D3D12_VIEWPORT viewport{};
    D3D12_RECT scissorRect{};

    WinApp* winApp_ = nullptr;

};