#pragma once
#include "../Patchables/Offsets.h"
#include "../Utils/Utils.h"
#include "D3DRenderer.h"
#include <d3d9.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <vector>
#include <map>
#include <string>
#include <d3dx9.h>
#include <experimental/filesystem>
#include "../Console/Console.h"
#include "../Geometry/Geometry.h"

inline LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class ImRender {
public:
	//IMainMenu* Menus;
	enum class DrawType {
		Normal,
		Filled
	};
	void Init(LPDIRECT3DDEVICE9 Device);
	void Free();
	void begin_draw();
	void end_draw();
	void draw_text(Vector2 screen_pos, const char* text, bool outlined = false, ImColor color = ImColor(255,255,255,255) , ImFont* font = nullptr);
	void draw_line(Vector2 start_pos, Vector2 end_pos, ImColor color = ImColor(255, 255, 255, 255), float thickness = 1.0f);
	void draw_line(Vector3 start_pos, Vector3 end_pos, ImColor color = ImColor(255, 255, 255, 255), float thickness = 1.0f);
	void draw_line3D(Vector3 start_pos, Vector3 end_pos, ImColor color = ImColor(255, 255, 255, 255), float thickness = 1.0f);
	void rect(Vector3 start, Vector3 end, float radius, float width, ImColor color);
	void polygon(Geometry::Polygon poly, ImColor color, float tick = 1);
	//void draw_rect(Vector4 screen_pos, ImColor color = ImColor(255, 255, 255, 255), DrawType type = DrawType::Normal, float rounding = 0.0f, unsigned short points = 12, float thickness = 1.0f);
	void draw_triangle(Vector2 pos_one, Vector2 pos_two, Vector2 pos_three, ImColor color = ImColor(255,255,255,255), DrawType type = DrawType::Normal, float thickness = 1.0f);
	void draw_circle(Vector2 screen_pos, float radius, ImColor color = ImColor(255, 255, 255, 255), DrawType type = DrawType::Normal, unsigned short points = 12, float thickness= 1.0f);
	void draw_circle(Vector3 screen_pos, float radius, ImColor color = ImColor(255, 255, 255, 255), DrawType type = DrawType::Normal, float thickness= 1.0f);
	void draw_image(const char* img, Vector3 pos, Vector2 body, ImColor color);
	void draw_image(const char* img, Vector2 pos, Vector2 body, ImColor color);
	std::map<std::string, PDIRECT3DTEXTURE9> Images = {};
private:
	D3DRenderer* riot_render;
	bool IsInitalized = false;
	float m_width = 0.0f;
	float m_height = 0.0f;
	bool LoadTextureFromFile(LPDIRECT3DDEVICE9 device, LPCSTR filename, PDIRECT3DTEXTURE9* out_texture);
	void LoadIcons(LPDIRECT3DDEVICE9 device, std::string& path);
};
inline ImRender render;