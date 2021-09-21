// include the basic windows header files and the Direct3D header files
#include <windows.h>
#include <windowsx.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
// include the Direct3D Library file
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

// global declarations
IDXGISwapChain* swapchain;             // the pointer to the swap chain interface
ID3D11Device5* dev;                     // the pointer to our Direct3D device interface
ID3D11DeviceContext* devcon;           // the pointer to our Direct3D device context
ID3D11RenderTargetView* backbuffer;    // the pointer to our back buffer
ID3D11VertexShader* lpVS;
ID3D11PixelShader* lpPS;

// function prototypes
void InitD3D(HWND hWnd);    // sets up and initializes Direct3D
void RenderFrame(void);     // renders a single frame
void CleanD3D(void);        // closes Direct3D and releases memory

struct Vec3 { float x, y, z; };
struct Vec4 { float x, y, z, w; };
typedef Vec4 Color;

struct VertexAttrib
{
    Vec3 Position;
    Color Color;
};

ID3D11Buffer* lpVBuffer;
ID3D11Buffer* lpIndexBuffer;
ID3D11InputLayout* lpLayout;

// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{

    HWND hWnd;
    WNDCLASSEX wc;

    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
//    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = L"WindowClass";

    RegisterClassEx(&wc);

    RECT wr = { 0, 0, 800, 600 };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    hWnd = CreateWindowEx(NULL,
        L"WindowClass",
        L"Our First Direct3D Program",
        WS_OVERLAPPEDWINDOW,
        100,
        100,
        1300,
        700,
        NULL,
        NULL,
        hInstance,
        NULL);

    ShowWindow(hWnd, nCmdShow);

    // set up and initialize Direct3D
    InitD3D(hWnd);

    // enter the main loop:

    MSG msg;

    while (TRUE)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
                break;
        }

        RenderFrame();
    }

    // clean up DirectX and COM    CleanD3D();

    return msg.wParam;
}


// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    } break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}


void InitPipline()
{
    ID3D10Blob* VS = nullptr;
    ID3D10Blob* PS = nullptr;

    D3DReadFileToBlob(L"./Debug/VertexShader.cso", &VS);
    D3DReadFileToBlob(L"./Debug/PixelShader.cso", &PS);
    //LoadShader
    //D3DCompileFromFile(L"VertexShader.hlsl", NULL, NULL, "main", "vs_5_0", NULL, NULL, &VS, NULL);
    //D3DCompileFromFile(L"PixelShader.hlsl", NULL, NULL, "main", "ps_5_0", NULL, NULL, &PS, NULL);

    dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &lpVS);
    dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &lpPS);

    devcon->VSSetShader(lpVS, NULL, NULL);
    devcon->PSSetShader(lpPS, NULL, NULL);

    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    dev->CreateInputLayout(ied, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &lpLayout);
    devcon->IASetInputLayout(lpLayout);


}



void InitGraphics()
{
    VertexAttrib vertex[] =
    {
       {-0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f},
       {-0.5f,  0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f},
       {0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f},
       {0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f},
    };


    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VertexAttrib) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    // Create indices.
    unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };

    // Fill in a buffer description.
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(unsigned int) * 6;
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;

    // Define the resource data.
    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = indices;
    InitData.SysMemPitch = 0;
    InitData.SysMemSlicePitch = 0;


    D3D11_SUBRESOURCE_DATA srd;
    ZeroMemory(&srd, sizeof(srd));
    srd.pSysMem = vertex;


    dev->CreateBuffer(&bd, &srd, &lpVBuffer);
 if (FAILED(dev->CreateBuffer(&bufferDesc, &InitData, &lpIndexBuffer)))
     int i = 1;
    
    D3D11_MAPPED_SUBRESOURCE ms;
    devcon->Map(lpVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
    memcpy(ms.pData, vertex, sizeof(vertex));
    devcon->Unmap(lpVBuffer, NULL);



    UINT stride = sizeof(VertexAttrib);
    UINT offset = 0;

    devcon->IASetIndexBuffer(lpIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    devcon->IASetVertexBuffers(0, 1, &lpVBuffer, &stride, &offset);
    devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// this function initializes and prepares Direct3D for use
void InitD3D(HWND hWnd)
{

    // create a struct to hold information about the swap chain
    DXGI_SWAP_CHAIN_DESC scd;

    // clear out the struct for use
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

    // fill the swap chain description struct
    scd.BufferCount = 2;                                    // one back buffer
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    scd.OutputWindow = hWnd;                                // the window to be used
    scd.SampleDesc.Count = 1;                               // how many multisamples
    scd.SampleDesc.Quality = 0;                             // multisample quality level
    scd.Windowed = TRUE;                                    // windowed/full-screen mode
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    // create a device, device context and swap chain using the information in the scd struct
    
    D3D11CreateDevice(nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr,
        0, D3D11_SDK_VERSION,
        (ID3D11Device**)&dev, nullptr, &devcon);
 

    IDXGIDevice1* dxgiDevice;
    IDXGIAdapter1* adapter = NULL;
    dev->QueryInterface(__uuidof(IDXGIDevice1),(void**) &dxgiDevice);
    dxgiDevice->GetAdapter((IDXGIAdapter**)&adapter);

    IDXGIFactory1* factory = NULL;

    
    adapter->GetParent(__uuidof(IDXGIFactory1), (void**)&factory);
    

    factory->CreateSwapChain((IUnknown*)dev, &scd, &swapchain);
    // get the address of the back buffer
    ID3D11Texture2D* pBackBuffer = nullptr;
    swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    // use the back buffer address to create the render target
    dev->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer);
    pBackBuffer->Release();

    // set the render target as the back buffer
    devcon->OMSetRenderTargets(1, &backbuffer, NULL);


    // Set the viewport
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = 1300;
    viewport.Height = 700;

    devcon->RSSetViewports(1, &viewport);
    InitPipline();
    InitGraphics();
}


// this is the function used to render a single frame
void RenderFrame(void)
{
    devcon->OMSetRenderTargets(1, &backbuffer, NULL);


    // clear the back buffer to a deep blue
    float color[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
    devcon->ClearRenderTargetView(backbuffer, color);


    // do 3D rendering on the back buffer here
    //devcon->Draw(6, 0);
    devcon->DrawIndexed(6, 0, 0);
    // switch the back buffer and the front buffer
    swapchain->Present(0, 0);

    DXGI_FRAME_STATISTICS stats;
    swapchain->GetFrameStatistics(&stats);
}


// this is the function that cleans up Direct3D and COM
void CleanD3D(void)
{
    // close and release all existing COM objects
    lpVS->Release();
    lpPS->Release();
    lpVBuffer->Release();
    lpIndexBuffer->Release();
    lpLayout->Release();
    swapchain->SetFullscreenState(FALSE, NULL);
    swapchain->Release();
    backbuffer->Release();
    dev->Release();
    devcon->Release();
}