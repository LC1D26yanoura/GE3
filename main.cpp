#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include "DirectXCommon.h"
#include "Input.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include <fstream>
#include <sstream>
#include <cassert>
#include <vector>
#include <string>
#include "affine.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

#pragma region 単位行列とTransform
// 単位行列の作成
Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 result = {};
	for (int i = 0; i < 4; ++i) {
		result.m[i][i] = 1;
	}
	return result;
}

struct TransformVector3
{
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;

};

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 nomal;
};

struct Material
{
	Vector4 color;
	int32_t endleLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};

struct TransformationMatrix
{
	Matrix4x4 WVP;
	Matrix4x4 world;

};

struct DirectionaLight
{
	Vector4 color;
	Vector3 direction;
	float intensity;
};

struct Transform1 {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

Transform1 uvTransformSprite{
	{1.0f, 1.0f, 1.0f},
	{0.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 0.0f},
};

struct MaterialData {
	std::string textureFilePath;
};


struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};


#pragma endregion

#pragma region ツール

//#pragma region DescriptorHeap
//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {
//
//	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
//
//	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
//	descriptorHeapDesc.Type = heapType;
//	descriptorHeapDesc.NumDescriptors = numDescriptors;
//	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
//
//	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
//
//	assert(SUCCEEDED(hr));
//	return descriptorHeap;
//}
//#pragma endregion
//
//#pragma region Textureデータ読み込み
//
//DirectX::ScratchImage LoadTexture(const std::string& filePath)
//{
//	//テクスチャファイルを読んでプログラムで扱えるようにする
//	DirectX::ScratchImage image{};
//	std::wstring filePathW = ConvertString(filePath);
//	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
//	assert(SUCCEEDED(hr));
//
//	//ミップマップの作成
//	DirectX::ScratchImage mipImages{};
//	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
//	assert(SUCCEEDED(hr));
//
//	//ミップマップ付きのデータを返す
//	return mipImages;
//}

//Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height) 
//{
//
//	D3D12_RESOURCE_DESC resourceDesc{};
//	resourceDesc.Width = width;
//	resourceDesc.Height = height;
//	resourceDesc.MipLevels = 1;
//	resourceDesc.DepthOrArraySize = 1;
//	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
//	resourceDesc.SampleDesc.Count = 1;
//	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
//	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
//
//
//	D3D12_HEAP_PROPERTIES heapProperties{};
//	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
//
//	D3D12_CLEAR_VALUE depthClearValue{};
//	depthClearValue.DepthStencil.Depth = 1.0f;
//	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
//
//	//Resourceの生成
//	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
//	HRESULT hr = device->CreateCommittedResource(
//		&heapProperties,
//		D3D12_HEAP_FLAG_NONE,
//		&resourceDesc,
//		D3D12_RESOURCE_STATE_DEPTH_WRITE,
//		&depthClearValue,
//		IID_PPV_ARGS(&resource));
//	assert(SUCCEEDED(hr));
//	return resource;
//}
//
//
//#pragma endregion
//
MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
	// 1.2.必要な変数の宣言とファイルを開く
	MaterialData materialData; // 構築するMaterialData
	std::string line; // ファイルから読んだ1行を格納するもの
	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
	assert(file.is_open()); // とりあえず開けなかったら止める
	// 3.ファイルを読み、MaterialDataを構築
	while (std::getline(file, line))
	{
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		// identifierに応じた処理
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			// 連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}
	return materialData;
}

ModelData LoaObjFile(const std::string& directoryPath, const std::string& filename) {
	
	// 1. 中で必要となる変数の宣言
	ModelData modelData; // 構築するModalData
	std::vector<Vector4> positions; // 位置
	std::vector<Vector3> normals; // 法線
	std::vector<Vector2> texcoords; // テクスチャ座標
	std::string line; // ファイルから読んだ1行を格納するもの
	
	// 2. ファイルを開く
	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
	assert(file.is_open()); // とりあえず開けなかったら止める
	
	// 3. 実際のファイルを読み込み、ModelDataを構築していく
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier; // 先頭の識別子を読む
		// identifierに応じた処理
		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;

			position.x *= -1.0f;

			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;

			texcoord.y = 1.0f - texcoord.y;
			
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {
			Vector3 normal;

			s >> normal.x >> normal.y >> normal.z;

			normal.x *= -1.0f;

			normals.push_back(normal);
		}
		else if (identifier == "f") {
			
			VertexData triangle[3];
			
			// 面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;

				// 頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndeices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/'); // 区切りでインデックスを読んでいく
					elementIndeices[element] = std::stoi(index);
				}

				// 要素へのIndexから、実際の要素の値をを取得して頂点を構築する
				Vector4 position = positions[elementIndeices[0] - 1];
				Vector2 texcoord = texcoords[elementIndeices[1] - 1];
				Vector3 normal = normals[elementIndeices[2] - 1];
				VertexData vertex = { position,texcoord,normal };
				modelData.vertices.push_back(vertex);
				triangle[faceVertex] = { position,texcoord,normal };
			}

			// 頂点を逆順で登録することで、回り順を逆にする
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		}
		else if (identifier == "mtllib") {
			// mateialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			// 基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}
	return modelData;

}TransformVector3 transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

// コールバック関数のプロトタイプ宣言
typedef void (*Callback)(int result);

void judge_result(int result) {
	transform.rotate.x += 0.1f;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	//D3DResourceLeakChecker LeakCheak;

	WinApp* winApp_ = nullptr;
	winApp_ = new WinApp();
	winApp_->Initialize();

#pragma region Windowの生成

	//GE3
	Input* input = nullptr;
	input = new Input();
	input->Initialize(winApp_);

	DirectXCommon* dxCommon = nullptr;
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp_);

	//ウインドウを表示する
	ShowWindow(winApp_->GetHwnd(), SW_SHOW);
#pragma endregion


	
	//#ifdef _DEBUG
	//
	//	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
	//	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	//	{
	//		debugController->EnableDebugLayer();
	//
	//		debugController->SetEnableSynchronizedCommandQueueValidation(TRUE);
	//
	//	}
	//
	//#endif
	//
	//#pragma region DXGIFactryの生成
	//	//dxgiFactoryの生成
	//	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	//	//HRESULT→Windows系のエラーコード
	//	//関数が成功したかどうかをSUCCEEDEDマクロで判定できる
	//	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	//	Microsoft::WRL::ComPtr<IDXGIAdapter4> useadapter = nullptr;
	//	assert(SUCCEEDED(hr));
	//#pragma endregion
	//
	//
	//#pragma region 使用アダプタ(GPU)の決定
	//
	//	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,
	//		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useadapter)) !=
	//		DXGI_ERROR_NOT_FOUND; ++i) {
	//
	//		DXGI_ADAPTER_DESC3 adapterDesc{};
	//		hr = useadapter->GetDesc3(&adapterDesc);
	//		assert(SUCCEEDED(hr));
	//
	//		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
	//			Log(StringUitilty::ConvertString(std::format(L"Use Adapater:{}\n", adapterDesc.Description)));
	//			break;
	//		}
	//		useadapter = nullptr;
	//	}
	//
	//	assert(useadapter != nullptr);
	//
	//#pragma endregion
	//
	//#pragma region D3D12Deviceの生成
	//
	//	Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;
	//	D3D_FEATURE_LEVEL featureLevels[] = {
	//
	//		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	//
	//	};
	//	const char* featureLevelStrrings[] = { "12.2","12.1","12.0" };
	//
	//	for (size_t i = 0; i < _countof(featureLevels); i++)
	//	{
	//		hr = D3D12CreateDevice(useadapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));
	//
	//		if (SUCCEEDED(hr))
	//		{
	//			Log(std::format("FeatureLevel : {}\n", featureLevelStrrings[i]));
	//			break;
	//		}
	//	}
	//	//デバイスの生成がうまくいかなかったので起動できない
	//	assert(device != nullptr);
	//
	//	Log("Complete create D3D12Device!!!\n");//初期化完了のログを出す
	//
	//#pragma endregion
	//
	//#ifdef _DEBUG
	//
	//	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
	//	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
	//	{
	//		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
	//		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
	//		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
	//
	//		//抑制するメッセージのID
	//		D3D12_MESSAGE_ID denyIds[] = {
	//
	//			//Windows11でのDXGIでバックプレイヤーとDX12デバッグレイヤーの互換作用バグによるエラーメッセージ
	//			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
	//		};
	//
	//		//抑制するレベル
	//		D3D12_MESSAGE_SEVERITY serverities[] = { D3D12_MESSAGE_SEVERITY_INFO };
	//		D3D12_INFO_QUEUE_FILTER filter{};
	//		filter.DenyList.NumIDs = _countof(denyIds);
	//		filter.DenyList.pIDList = denyIds;
	//		filter.DenyList.NumSeverities = _countof(serverities);
	//		filter.DenyList.pSeverityList = serverities;
	//
	//		//指定したメッセージの表示を抑制する
	//		infoQueue->PushStorageFilter(&filter);
	//
	//	}
	//
	//#endif // _DEBUG
	//
	//
	//#pragma region コマンドキュー生成
	//
	//	//コマンドキュー生成
	//	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	//	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	//	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	//
	//	//コマンドキューが生成できないので起動できない
	//	assert(SUCCEEDED(hr));
	//
	//	//コマンドアロケータを生成する
	//	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>commandAllocator = nullptr;
	//	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	//	//コマンドアロケータを生成出来ないので起動できない
	//	assert(SUCCEEDED(hr));
	//
	//	//コマンドリストを生成する
	//	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	//	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	//	//コマンドリストetc...
	//	assert(SUCCEEDED(hr));
	//
	//#pragma endregion
	//
	//#pragma region SwapChainの設定
	//
	//	//SwapChain
	//	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
	//	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	//	swapChainDesc.Width = WinApp::kClientWidth;
	//	swapChainDesc.Height = WinApp::kClientHeight;//画面の高さ
	//	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//色の形式
	//	swapChainDesc.SampleDesc.Count = 1;//マルチサンプルしない
	//	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//描画のターゲットとして利用する
	//	swapChainDesc.BufferCount = 2;//ダブルバッファ
	//	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//モニターに移したら中身を破壊
	//
	//	hr = dxgiFactory->CreateSwapChainForHwnd(
	//		commandQueue.Get(), 
	//		winApp_->GetHwnd(),
	//		&swapChainDesc, 
	//		nullptr, 
	//		nullptr, 
	//		reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
	//	assert(SUCCEEDED(hr));
	//
	//	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	//
	//	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
	//	
	//	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
	//
	//	//swapchainからリソースを引っ張る
	//	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2] = { nullptr };
	//
	//	//うまくできなければ起動できない
	//	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	//	assert(SUCCEEDED(hr));
	//	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	//	assert(SUCCEEDED(hr));
	//
	//#pragma endregion
	//
	//#pragma region RTVの作成
	//
	//	//RTVの設定
	//	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	//	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	//
	//	//ディスクリプタの先頭を取得する
	//	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//	//RTVを二つ作るのでディスクリプタを二つ用意
	//	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	//	//まず一つ目を作る。一つ目は最初のところに作る。作る場所をこちらで指定する必要がある
	//	rtvHandles[0] = rtvStartHandle;
	//	device->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc, rtvHandles[0]);
	//	//２つ目のディスクリプタハンドルを得る（自力で）
	//	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//	//二つ目を作る
	//	device->CreateRenderTargetView(swapChainResources[1].Get(), &rtvDesc, rtvHandles[1]);
	//
	//#pragma endregion
	//
	//	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	//	uint64_t fenceValue = 0;
	//	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	//	assert(SUCCEEDED(hr));
	//
	//	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	//	assert(fenceEvent != nullptr);
	//
	//#pragma region DXCの初期化
	//
	//	//DXCの初期化
	//	IDxcUtils* dxcUtils = nullptr;
	//	IDxcCompiler3* dxcCompiler = nullptr;
	//
	//	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	//	assert(SUCCEEDED(hr));
	//
	//	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	//	assert(SUCCEEDED(hr));
	//
	//	//include対応のため設定しておく
	//	IDxcIncludeHandler* includeHandler = nullptr;
	//	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	//	assert(SUCCEEDED(hr));
	//#pragma endregion
	//
	//
	#pragma region  descriptionRootSignature
	
		D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
		descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	
		D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
		descriptorRange[0].BaseShaderRegister = 0;
		descriptorRange[0].NumDescriptors = 1;
		descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	
		//RootParameter作成。複数設定できるので配列。今回は結果は1つだけなので長さ１の配列
		D3D12_ROOT_PARAMETER rootParameters[4] = {};
		rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParameters[0].Descriptor.ShaderRegister = 0;
	
		rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		rootParameters[1].Descriptor.ShaderRegister = 0;
	
		rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
		rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
	
		rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParameters[3].Descriptor.ShaderRegister = 1;
	
		descriptionRootSignature.pParameters = rootParameters;
		descriptionRootSignature.NumParameters = _countof(rootParameters);
	
	
		D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
		staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
		staticSamplers[0].ShaderRegister = 0;
		staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	
		descriptionRootSignature.pStaticSamplers = staticSamplers;
		descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);
	
	
		Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	
		if (FAILED(hr))
		{
			Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
			assert(false);
	
		}
		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
		hr = dxCommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
		assert(SUCCEEDED(hr));
	#pragma endregion
	
	
	#pragma region inputElementDescとgraphicsPinpelineStateDesc
	
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
		inputElementDescs[0].SemanticName = "POSITION";
		inputElementDescs[0].SemanticIndex = 0;
		inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	
		inputElementDescs[1].SemanticName = "TEXCOORD";
		inputElementDescs[1].SemanticIndex = 0;
		inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	
		inputElementDescs[2].SemanticName = "NORMAL";
		inputElementDescs[2].SemanticIndex = 0;
		inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	
		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
		inputLayoutDesc.pInputElementDescs = inputElementDescs;
		inputLayoutDesc.NumElements = _countof(inputElementDescs);
	
		D3D12_BLEND_DESC blendDesc{};
		blendDesc.RenderTarget[0].RenderTargetWriteMask =
			D3D12_COLOR_WRITE_ENABLE_ALL;
	
		D3D12_RASTERIZER_DESC rasterizerDesc{};
	
		//裏面表示しない
		rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
		//三角形の中を塗りつぶす
		rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	
		Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlobComPtr = dxCommon->CompileShader(L"resources/shaders/Object3D.VS.hlsl", L"vs_6_0");
		IDxcBlob* vertexShaderBlob = vertexShaderBlobComPtr.Get();
		assert(vertexShaderBlob != nullptr);
	
		Microsoft::WRL::ComPtr<IDxcBlob> PixelShaderBlobComPtr = dxCommon->CompileShader(L"resources/shaders/Object3D.PS.hlsl", L"ps_6_0");
		IDxcBlob* pixelShaderBlob = PixelShaderBlobComPtr.Get();
		assert(pixelShaderBlob != nullptr);

		//ここから03_01
		D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
		depthStencilDesc.DepthEnable = true;
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		//ここから03_01
	
		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPinpelineStateDesc{};
		graphicsPinpelineStateDesc.pRootSignature = rootSignature.Get();
		graphicsPinpelineStateDesc.InputLayout = inputLayoutDesc;
		graphicsPinpelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
			vertexShaderBlob->GetBufferSize() };
		graphicsPinpelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),pixelShaderBlob->GetBufferSize() };
		graphicsPinpelineStateDesc.BlendState = blendDesc;
		graphicsPinpelineStateDesc.RasterizerState = rasterizerDesc;
	
		graphicsPinpelineStateDesc.NumRenderTargets = 1;
		graphicsPinpelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	
		graphicsPinpelineStateDesc.PrimitiveTopologyType =
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	
		graphicsPinpelineStateDesc.SampleDesc.Count = 1;
		graphicsPinpelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	
		graphicsPinpelineStateDesc.DepthStencilState = depthStencilDesc;
		graphicsPinpelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	
		Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResouce = dxCommon->CreateDepthStencilTextureResource(dxCommon->GetDevice(), WinApp::kClientWidth, WinApp::kClientHeight);
	
		//ここから03_01
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dxCommon->GetDevice()->CreateDepthStencilView(depthStencilResouce.Get(), &dsvDesc, dxCommon->GetDSVDescriptorHeap()->GetCPUDescriptorHandleForHeapStart());
		//ここから03_01
	
		Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
		hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPinpelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
		assert(SUCCEEDED(hr));
	
	#pragma endregion
	
		//Material用のResourceを作る
		Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = dxCommon->CreateBufferResource(sizeof(Material));
		Material* materialData = nullptr;
		materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
		//ここで色かえられるよ
		materialData->color = { 1.0f,1.0f,1.0f,1.0f };
		materialData->endleLighting = true;
		materialData->uvTransform = MakeIdentity4x4();
	
		bool useMonsterBall = false;
	
		TransformVector3 transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	
		//Resourcef
		const uint32_t kSubdivision = 36;
	
		//VertexResourceを生成
		//ID3D12Resource* vertexResource = CreateBufferResource(device, sizeof(VertexData) * kSubdivision * kSubdivision * 6);
	
		//モデル読み込み
		ModelData modeldata = LoaObjFile("resources", "plane.obj");
	
		//頂点リソースを作る
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexReComPtr = dxCommon->CreateBufferResource(sizeof(VertexData) * modeldata.vertices.size());
		ID3D12Resource* vertexResource = vertexReComPtr.Get();

		//頂点バッファビューを作成する
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
		vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();// リソースの先頭のアドレスから使う
		vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modeldata.vertices.size());// 使用するリソースのサイズは頂点のサイズ
		vertexBufferView.StrideInBytes = sizeof(VertexData);// 1頂点あたりのサイズ

		// 頂点リソースにデータを書き込む
		VertexData* vertexData = nullptr;
		vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));// 書き込むためのアドレスを取得
		std::memcpy(vertexData, modeldata.vertices.data(), sizeof(VertexData)* modeldata.vertices.size());// 頂点データをリソースにコピー

	//DepthStencilTextureを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = dxCommon->CreateDepthStencilTextureResource(dxCommon->GetDevice(), WinApp::kClientWidth, WinApp::kClientHeight);
	
	//VertexBufferResourceを生成
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite = dxCommon->CreateBufferResource(sizeof(VertexData) * 6);
	
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = dxCommon->CreateBufferResource(sizeof(uint32_t) * 6);
	
	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	
	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;
	
	uint32_t* indexDataSprite = nullptr;
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	indexDataSprite[0] = 0;
	indexDataSprite[1] = 1;
	indexDataSprite[2] = 2;
	indexDataSprite[3] = 1;
	indexDataSprite[4] = 3;
	indexDataSprite[5] = 2;
	
	//Sprite用のマテリアルリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite = dxCommon->CreateBufferResource(sizeof(Material));
	Material* materialDataSprite = nullptr;
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	materialDataSprite->color = { 1.0f,1.0f,1.0f,1.0f };
	materialDataSprite->endleLighting = false;
	materialDataSprite->uvTransform = MakeIdentity4x4();
	
	//ライティング
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResorce = dxCommon->CreateBufferResource(sizeof(DirectionaLight));
	DirectionaLight* directionalLightData = nullptr;
	directionalLightResorce->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f,-1.0f,0.0f };
	directionalLightData->intensity = 1.0f;

	//TransformationMatrixResource
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	TransformationMatrix* transformationMatrixDataSprite = nullptr;
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	transformationMatrixDataSprite->WVP = MakeIdentity4x4();
	transformationMatrixDataSprite->world = MakeIdentity4x4();

	// Sprite用のTransfomationMatrix用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	// データを書き込む
	TransformationMatrix* wvpDeta = nullptr;
	// 書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpDeta));
	// 単位行列を書き込んでおく
	wvpDeta->world = MakeIdentity4x4();
	wvpDeta->WVP = MakeIdentity4x4();

	////vetexResourceSprite頂点バッファーを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{ };
	//リソースの先頭のアドレスから使う
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 6;
	//1頂点当たりのサイズ
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);
	//頂点リソースにデータを書き込む
	VertexData* vertexDataSprite = nullptr;
	//書き込むためのアドレスを取得
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));

	// 1枚目の三角形
	vertexDataSprite[0].position = { 0.0f, 360.0f, 0.0f, 1.0f };// 左下
	vertexDataSprite[0].texcoord = { 0.0f, 1.0f };
	vertexDataSprite[0].nomal = { 0.0f, 0.0f, -1.0f };
	vertexDataSprite[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };// 左上
	vertexDataSprite[1].texcoord = { 0.0f,0.0f };
	vertexDataSprite[1].nomal = { 0.0f, 0.0f, -1.0f };
	vertexDataSprite[2].position = { 640.0f, 360.0f, 0.0f, 1.0f };// 右下
	vertexDataSprite[2].texcoord = { 1.0f, 1.0f };
	vertexDataSprite[2].nomal = { 0.0f, 0.0f, -1.0f };
	// 2枚目の三角形
	vertexDataSprite[3].position = { 640.0f, 0.0f, 0.0f, 1.0f };// 右上
	vertexDataSprite[3].texcoord = { 1.0f, 0.0f };
	vertexDataSprite[3].nomal = { 0.0f, 0.0f, -1.0f };

	D3D12_VIEWPORT viewport{};

	viewport.Width = WinApp::kClientWidth;
	viewport.Height = WinApp::kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	D3D12_RECT scissorRect{};

	scissorRect.left = 0;
	scissorRect.right = WinApp::kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = WinApp::kClientHeight;


	
	TransformVector3 cameraTransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} };
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	dxCommon->GetCPUDescriptorHandle(dxCommon->GetRTVDescriptorHeap(), dxCommon->GetDescriptorSizeRTV(), 0);


	DirectX::ScratchImage mipimage2 = dxCommon->LoadTexture("resources/monsterBall.png");
	const DirectX::TexMetadata& metadata2 = mipimage2.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = dxCommon->CreateTextureResource(dxCommon->GetDevice(), metadata2);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResources2 = dxCommon->UploadTextureData(textureResource2, mipimage2);

	//metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

	//SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = dxCommon->GetCPUDescriptorHandle(dxCommon->GetSRVDescriptorHeap(), dxCommon->GetDescriptorSizeSRV(), 2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = dxCommon->GetGPUDescriptorHandle(dxCommon->GetSRVDescriptorHeap(), dxCommon->GetDescriptorSizeSRV(), 2);
	//SRVの生成
	dxCommon->GetDevice()->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);

	//DirectX::ScratchImage mipImages = LoadTexture("resources/uvChecker.png");
	DirectX::ScratchImage mipImages2 = dxCommon->LoadTexture(modeldata.material.textureFilePath);
	const DirectX::TexMetadata& metadata = mipImages2.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = dxCommon->CreateTextureResource(dxCommon->GetDevice(), metadata);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResources = dxCommon->UploadTextureData(textureResource, mipImages2);
	//metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
	//SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = dxCommon->GetSRVDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = dxCommon->GetSRVDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
	//戦闘はImGuiが使っているのでその次を使う
	textureSrvHandleCPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//SRVの生成
	dxCommon->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);
	
		MSG msg{};
	
		//dxCommon->InitializeImGui();

	while (true) {

		if (winApp_->ProcessMessage()) {

			break;
		}
		else {

			Callback callback = judge_result;

			callback(0);

			//GE3
			input->Update();

			//ゲーム処理

			//描画前処理
			dxCommon->PreDraw();
			transform.rotate.y += 0.0f;
			Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
			Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
			Matrix4x4 viewMatrix = Inverse(cameraMatrix);
			Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.0f);
			Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
			wvpDeta->world = worldMatrix;
			wvpDeta->WVP = worldViewProjectionMatrix;


			// Sprite用のWorldViewProjectionMatrixを作る
			Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
			Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
			Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);
			Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
			transformationMatrixDataSprite->world = worldMatrixSprite;
			transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;


			//スプライト
			Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
			materialDataSprite->uvTransform = uvTransformMatrix;

			////これから書き込むバッファのインデックスを取得
			//UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			

			// ImGuiウィンドウの作成
			ImGui::Begin("Ball Controls");
			ImGui::SliderFloat3("Position", &transform.translate.x, -5.0f, 5.0f);
			ImGui::SliderFloat3("Rotation", &transform.rotate.x, -180.0f, 180.0f);
			ImGui::SliderFloat3("Scale", &transform.scale.x, 0.1f, 2.0f);
			ImGui::End();

			ImGui::Render();
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());
			

			dxCommon->GetCommandList()->RSSetViewports(1, &viewport);
			dxCommon->GetCommandList()->RSSetScissorRects(1, &scissorRect);
			//RootSignatureを設定。PSOに設定しているけど別途設定が必要
			dxCommon->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
			dxCommon->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());

			//Sphere
			dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
			//形状を設定。PSOに設定しているものとはまた別、同じものを設定すると考えておけば良い
			dxCommon->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
			//wvp用のCBufferの場所を設定
			dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
			dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);
			dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResorce->GetGPUVirtualAddress());
			//描画

			dxCommon->GetCommandList()->DrawInstanced(UINT(modeldata.vertices.size()), 1, 0, 0);
			//commandList->DrawInstanced(kSubdivision* kSubdivision * 6, 1, 0, 0);

			// Spriteの描画。変更が必要なものだけ変更する
			dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSprite); // VBVを設定
			dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
			// TransformationMatrixCBufferの場所を設定
			dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
			dxCommon->GetCommandList()->IASetIndexBuffer(&indexBufferViewSprite);// IBVを設定//06_00
			dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
			// 描画！（DrawCall/ドローコール）6個のインデックスを使用し1つのインスタンスを描画。その他は当面0で良い
			//commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);//06_00


			//barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			////実際のcommandListのImGuiの描画コマンドを積む
			//barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

			//commandList->ResourceBarrier(1, &barrier);


			////コマンドリストの内容を確定させる。　すべてのコマンドを頼んでからclauseすること
			//hr = commandList->Close();
			//assert(SUCCEEDED(hr));


			////GPUにコマンドリストの実行を行わせる
			//Microsoft::WRL::ComPtr<ID3D12CommandList> commandLists[] = { commandList };
			//commandQueue->ExecuteCommandLists(1, commandLists->GetAddressOf());
			////　GPUとOSに画面の交換を行うように通知する
			//swapChain->Present(1, 0);


			//fenceValue++;
			//commandQueue->Signal(fence.Get(), fenceValue);

			//if (fence->GetCompletedValue() < fenceValue) {

			//	fence->SetEventOnCompletion(fenceValue, fenceEvent);

			//	WaitForSingleObject(fenceEvent, INFINITE);

			//}


			//次のフレーム用のコマンドリストを準備
			//hr = commandAllocator->Reset();
			//assert(SUCCEEDED(hr));
			//hr = commandList->Reset(commandAllocator.Get(), nullptr);
			//assert(SUCCEEDED(hr));

			dxCommon->PostDraw();
		}
	}

	
	//Windows終了
	winApp_->Finalize();

	delete input;
	delete winApp_;
	delete dxCommon;
	//winApp_ = nullptr;


	//CloseHandle(fenceEvent);

	//CoUninitialize();
	return 0;
}