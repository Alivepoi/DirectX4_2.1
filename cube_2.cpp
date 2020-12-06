//////////////////////////////////////////////////////////////////////////////////////////////////
// 
// File: cube.cpp
// 
// Author: Frank Luna (C) All Rights Reserved
//
// System: AMD Athlon 1800+ XP, 512 DDR, Geforce 3, Windows XP, MSVC++ 7.0 
//
// Desc: Renders a spinning cube in wireframe mode.  Demonstrates Vertex_ and 
//       index buffers, world and view transformations, render states and
//       drawing commands.
//          
//////////////////////////////////////////////////////////////////////////////////////////////////

#include "d3dUtility.h"
#include "Class_cube.h"
#include "Vertex.h"
//
// Globals
//

IDirect3DDevice9* Device = 0; 

const int Width  = 640;
const int Height = 480;

IDirect3DVertexBuffer9* VB = 0;
IDirect3DIndexBuffer9*  IB = 0;
Cube* Box = 0;

D3DMATERIAL9 Mtrl_; //定义材质
IDirect3DTexture9* Tex = 0;//定义纹理

//
// Classes and Structures
//

struct Vertex_
{
	Vertex_(){}
	Vertex_(float x, float y, float z)
	{
		_x = x;  _y = y;  _z = z;
	}
	float _x, _y, _z; //坐标
	static const DWORD FVF;
};
const DWORD Vertex_::FVF = D3DFVF_XYZ | D3DFVF_NORMAL;

//
// Framework Functions
//
bool Setup()
{

	//
	// Create Vertex_ and index buffers.
	//
	Box = new Cube(Device);

	Device->CreateVertexBuffer(
		8 * sizeof(Vertex_), 
		D3DUSAGE_WRITEONLY,
		Vertex_::FVF,
		D3DPOOL_MANAGED,
		&VB,
		0);

	Device->CreateIndexBuffer(
		36 * sizeof(WORD),
		D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,
		D3DPOOL_MANAGED,
		&IB,
		0);

	//
	// Fill the buffers with the cube data.
	//

	// define unique vertices:
	Vertex_* vertices;
	VB->Lock(0, 0, (void**)&vertices, 0);

	// vertices of a unit cube
	vertices[0] = Vertex_(-1.0f, -1.0f, -1.0f);
	vertices[1] = Vertex_(-1.0f, 1.0f, -1.0f);
	vertices[2] = Vertex_(1.0f, 1.0f, -1.0f);
	vertices[3] = Vertex_(1.0f, -1.0f, -1.0f);
	vertices[4] = Vertex_(-1.0f, -1.0f, 1.0f);
	vertices[5] = Vertex_(-1.0f, 1.0f, 1.0f);
	vertices[6] = Vertex_(1.0f, 1.0f, 1.0f);
	vertices[7] = Vertex_(1.0f, -1.0f, 1.0f);


	VB->Unlock();
	// define the triangles of the cube:
	WORD* indices = 0;
	IB->Lock(0, 0, (void**)&indices, 0);

	// front side
	indices[0] = 0; indices[1] = 1; indices[2] = 2;
	indices[3] = 0; indices[4] = 2; indices[5] = 3;

	// back side
	indices[6] = 4; indices[7] = 6; indices[8] = 5;
	indices[9] = 4; indices[10] = 7; indices[11] = 6;

	// left side
	indices[12] = 4; indices[13] = 5; indices[14] = 1;
	indices[15] = 4; indices[16] = 1; indices[17] = 0;

	// right side
	indices[18] = 3; indices[19] = 2; indices[20] = 6;
	indices[21] = 3; indices[22] = 6; indices[23] = 7;

	// top
	indices[24] = 1; indices[25] = 5; indices[26] = 6;
	indices[27] = 1; indices[28] = 6; indices[29] = 2;

	// bottom
	indices[30] = 4; indices[31] = 0; indices[32] = 3;
	indices[33] = 4; indices[34] = 3; indices[35] = 7;

	//indices[36] = 6; indices[37] = 7; indices[38] = 8;

	IB->Unlock();

	//设置方向光源
	D3DXVECTOR3 dir(1.0f, -0.0f, 0.25f);
	D3DXCOLOR   c = d3d::WHITE;
	D3DLIGHT9 dirLight = d3d::InitDirectionalLight(&dir, &c);

	//设置点光源
	D3DXVECTOR3 point_(1.0f, 1.0f, 1.0f);
	D3DXCOLOR  p = d3d::RED;
	D3DLIGHT9 pointLight = d3d::InitPointLight(&point_, &p);

	////设置并启用灯光 
	Device->SetLight(0, &dirLight);
	Device->SetLight(1, &pointLight);
	Device->LightEnable(0, true);
	Device->LightEnable(1, true);

	////设置渲染状态
	Device->SetRenderState(D3DRS_NORMALIZENORMALS, true);
	Device->SetRenderState(D3DRS_SPECULARENABLE, false);

	D3DXCreateTextureFromFile(
		Device,
		"TreasureChest.jpg",
		&Tex);
	//使用各向异性纹理过滤器
	Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	Device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	//
	// Position and aim the camera.
	//
	Mtrl_ = d3d::YELLOW_MTRL;



	//
	// Set the projection matrix.
	//

	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovLH(
			&proj,
			D3DX_PI * 0.5f, // 90 - degree
			(float)Width / (float)Height,
			1.0f,     
			1000.0f);
	Device->SetTransform(D3DTS_PROJECTION, &proj);

	//
	// Switch to wireframe mode.
	//

	Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	return true;
}

void Cleanup()
{
	d3d::Delete<Cube*>(Box);

	d3d::Release<IDirect3DVertexBuffer9*>(VB);
	d3d::Release<IDirect3DIndexBuffer9*>(IB);
	d3d::Release<IDirect3DTexture9*>(Tex);
}

bool Display(float timeDelta)
{
	if( Device )
	{


		// 
	// Update the scene: update camera position.
	//

		static float angle = (3.0f * D3DX_PI) / 2.0f;
		static float height = 2.0f;

		if (::GetAsyncKeyState(VK_LEFT) & 0x8000f)
			angle -= 0.5f * timeDelta;

		if (::GetAsyncKeyState(VK_RIGHT) & 0x8000f)
			angle += 0.5f * timeDelta;

		if (::GetAsyncKeyState(VK_UP) & 0x8000f)
			height += 5.0f * timeDelta;

		if (::GetAsyncKeyState(VK_DOWN) & 0x8000f)
			height -= 5.0f * timeDelta;

		D3DXVECTOR3 position(cosf(angle) * 3.0f, height, sinf(angle) * 3.0f);
		D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
		D3DXMATRIX V;
		D3DXMatrixLookAtLH(&V, &position, &target, &up);

		Device->SetTransform(D3DTS_VIEW, &V);

		//
		// draw the scene:
		//

		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);
		Device->BeginScene();

		Device->SetTexture(0, Tex);
		Box->draw(0, 0, 0);

		Device->SetStreamSource(0, VB, 0, sizeof(Vertex_));
		Device->SetIndices(IB);
		Device->SetFVF(Vertex_::FVF);
		Device->SetMaterial(&Mtrl_);
	
		//Device->SetTexture(0, Tex);
		//Draw cube.
		//Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 16, 0, 24);
		

		//D3DXMATRIX V, V_;
		//D3DXMatrixTranslation(&V, 4, 0, 0);
		//Device->SetTransform(D3DTS_WORLD, &V);
		//Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 16, 0, 24);


		
	


		Device->EndScene();
		Device->Present(0, 0, 0, 0);
	}
	return true;
}

//
// WndProc
//
LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;
		
	case WM_KEYDOWN:
		if( wParam == VK_ESCAPE )
			::DestroyWindow(hwnd);
		break;
	}
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

//
// WinMain
//
int WINAPI WinMain(HINSTANCE hinstance,
				   HINSTANCE prevInstance, 
				   PSTR cmdLine,
				   int showCmd)
{
	if(!d3d::InitD3D(hinstance,
		Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}
		
	if(!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}

	d3d::EnterMsgLoop( Display );

	Cleanup();

	Device->Release();

	return 0;
}
