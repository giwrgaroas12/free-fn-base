#pragma once
#include <thread>
#include <unordered_map>
#include <string>
#include <d3d9.h>
#include <vector>
#include "comm/driver.hpp"
#include "settings.hpp"
#include "offsets.hpp"

#define M_PI 3.14159265358979323846f

class Vector2 {
public:
	Vector2() : x(0.f), y(0.f) {}
	Vector2(double _x, double _y) : x(_x), y(_y) {}
	~Vector2() {}

	double x, y, z;
};


class Vector3
{
public:
	Vector3() : x(0.f), y(0.f), z(0.f)
	{

	}

	Vector3(double _x, double _y, double _z) : x(_x), y(_y), z(_z)
	{

	}
	~Vector3()
	{

	}

	double x;
	double y;
	double z;

	inline double dot(Vector3 v)
	{
		return x * v.x + y * v.y + z * v.z;
	}

	inline bool isValid() const {
		return isfinite(x) && isfinite(y) && isfinite(z);
	}

	inline double distance(Vector3 v)
	{
		return double(sqrtf(powf(v.x - x, 2.0) + powf(v.y - y, 2.0) + powf(v.z - z, 2.0)));
	}

	inline double length() {
		return sqrt(x * x + y * y + z * z);
	}

	Vector3 operator+(const Vector3& other) const { return { this->x + other.x, this->y + other.y, this->z + other.z }; }
	Vector3 operator-(const Vector3& other) const { return { this->x - other.x, this->y - other.y, this->z - other.z }; }
	Vector3 operator*(float offset) const { return { this->x * offset, this->y * offset, this->z * offset }; }
	Vector3 operator/(float offset) const { return { this->x / offset, this->y / offset, this->z / offset }; }

	Vector3& operator*=(const double other) { this->x *= other; this->y *= other; this->z *= other; return *this; }
	Vector3& operator/=(const double other) { this->x /= other; this->y /= other; this->z /= other; return *this; }

	Vector3& operator=(const Vector3& other) { this->x = other.x; this->y = other.y; this->z = other.z; return *this; }
	Vector3& operator+=(const Vector3& other) { this->x += other.x; this->y += other.y; this->z += other.z; return *this; }
	Vector3& operator-=(const Vector3& other) { this->x -= other.x; this->y -= other.y; this->z -= other.z; return *this; }
	Vector3& operator*=(const Vector3& other) { this->x *= other.x; this->y *= other.y; this->z *= other.z; return *this; }


	operator bool() { return bool(this->x || this->y || this->z); }
	friend bool operator==(const Vector3& a, const Vector3& b) { return a.x == b.x && a.y == b.y && a.z == b.z; }

};

struct fquat { double x, y, z, w; };
struct FTransform
{
	fquat rot;
	Vector3 translation;
	uint8_t pad1c[0x8];
	Vector3 scale;
	uint8_t pad2c[0x8];

	D3DMATRIX to_matrix_with_scale() const
	{
		D3DMATRIX m{};

		Vector3 AdjustedScale
		(
			(scale.x == 0.0) ? 1.0 : scale.x,
			(scale.y == 0.0) ? 1.0 : scale.y,
			(scale.z == 0.0) ? 1.0 : scale.z
		);

		m._41 = translation.x;
		m._42 = translation.y;
		m._43 = translation.z;

		float x2 = rot.x + rot.x;
		float y2 = rot.y + rot.y;
		float z2 = rot.z + rot.z;
		float xx2 = rot.x * x2;
		float yy2 = rot.y * y2;
		float zz2 = rot.z * z2;

		m._11 = (1.0f - (yy2 + zz2)) * AdjustedScale.x;
		m._22 = (1.0f - (xx2 + zz2)) * AdjustedScale.y;
		m._33 = (1.0f - (xx2 + yy2)) * AdjustedScale.z;

		float yz2 = rot.y * z2;
		float wx2 = rot.w * x2;
		m._32 = (yz2 - wx2) * AdjustedScale.z;
		m._23 = (yz2 + wx2) * AdjustedScale.y;

		float xy2 = rot.x * y2;
		float wz2 = rot.w * z2;
		m._21 = (xy2 - wz2) * AdjustedScale.y;
		m._12 = (xy2 + wz2) * AdjustedScale.x;

		float xz2 = rot.x * z2;
		float wy2 = rot.w * y2;
		m._31 = (xz2 + wy2) * AdjustedScale.z;
		m._13 = (xz2 - wy2) * AdjustedScale.x;

		m._14 = 0.0f;
		m._24 = 0.0f;
		m._34 = 0.0f;
		m._44 = 1.0f;

		return m;
	}
};

D3DMATRIX matrix_multiplication(const D3DMATRIX& pm1, const D3DMATRIX& pm2)
{
	D3DMATRIX pout{};

	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			pout.m[row][col] = pm1.m[row][0] * pm2.m[0][col] +
				pm1.m[row][1] * pm2.m[1][col] +
				pm1.m[row][2] * pm2.m[2][col] +
				pm1.m[row][3] * pm2.m[3][col];
		}
	}

	return pout;
}

D3DMATRIX to_matrix(Vector3 rot, Vector3 origin = Vector3(0, 0, 0))
{
	float radpitch = (rot.x * M_PI / 180);
	float radyaw = (rot.y * M_PI / 180);
	float radroll = (rot.z * M_PI / 180);
	float sp = sinf(radpitch);
	float cp = cosf(radpitch);
	float sy = sinf(radyaw);
	float cy = cosf(radyaw);
	float sr = sinf(radroll);
	float cr = cosf(radroll);
	D3DMATRIX matrix{};
	matrix.m[0][0] = cp * cy;
	matrix.m[0][1] = cp * sy;
	matrix.m[0][2] = sp;
	matrix.m[0][3] = 0.f;
	matrix.m[1][0] = sr * sp * cy - cr * sy;
	matrix.m[1][1] = sr * sp * sy + cr * cy;
	matrix.m[1][2] = -sr * cp;
	matrix.m[1][3] = 0.f;
	matrix.m[2][0] = -(cr * sp * cy + sr * sy);
	matrix.m[2][1] = cy * sr - cr * sp * sy;
	matrix.m[2][2] = cr * cp;
	matrix.m[2][3] = 0.f;
	matrix.m[3][0] = origin.x;
	matrix.m[3][1] = origin.y;
	matrix.m[3][2] = origin.z;
	matrix.m[3][3] = 1.f;
	return matrix;
}

struct camera {
	Vector3 location;
	Vector3 rotation;
	float fov;
};

namespace cache {
	inline uintptr_t uworld;
	inline uintptr_t mesh;
	inline uintptr_t game_instance;
	inline uintptr_t local_players;
	inline uintptr_t player_controller;
	inline uintptr_t local_pawn;
	inline uintptr_t root_component;
	inline uintptr_t player_state;
	inline uintptr_t game_state;
	inline uintptr_t player_array;
	inline uintptr_t closest_entity;
	inline uintptr_t closest_mesh;
	inline uintptr_t current_weapon;
	inline uintptr_t Current_Vehicle;
	inline uintptr_t player_id;
	inline DWORD ScreenCenterX;
	inline DWORD ScreenCenterY;
	inline camera local_camera;
	inline uintptr_t my_team_id;
	inline Vector3 relative_location;
	inline int player_count;
	inline float closest_distance;
	std::vector<ImVec2> radarPoints;
}



struct FPlane : Vector3
{
	double W;

	FPlane() : W(0) {}
	FPlane(double W) : W(W) {}
};

class FMatrix
{
public:
	double m[4][4];
	FPlane XPlane, YPlane, ZPlane, WPlane;

	FMatrix() : XPlane(), YPlane(), ZPlane(), WPlane() {}
	FMatrix(FPlane XPlane, FPlane YPlane, FPlane ZPlane, FPlane WPlane)
		: XPlane(XPlane), YPlane(YPlane), ZPlane(ZPlane), WPlane(WPlane) {
	}
};

inline double RadiansToDegrees(double dRadians)
{
	return dRadians * (180.0 / M_PI);
}

template< typename t >
class TArray
{
public:

	TArray() : tData(), iCount(), iMaxCount() {}
	TArray(t* data, int count, int max_count) :
		tData(tData), iCount(iCount), iMaxCount(iMaxCount) {
	}

public:

	auto Get(int idx) -> t
	{
		return read< t >(reinterpret_cast<__int64>(this->tData) + (idx * sizeof(t)));
	}

	auto Size() -> std::uint32_t
	{
		return this->iCount;
	}

	bool IsValid()
	{
		return this->iCount != 0;
	}

	t* tData;
	int iCount;
	int iMaxCount;
};

Vector3 calculateNewRotation(Vector3& zaz, Vector3& daz)
{
	Vector3 dalte = zaz - daz;
	Vector3 ongle;
	float hpm = sqrtf(dalte.x * dalte.x + dalte.y * dalte.y);
	ongle.y = atan(dalte.y / dalte.x) * 57.295779513082f;
	ongle.x = (atan(dalte.z / hpm) * 57.295779513082f) * -1.f;

	if (dalte.x >= 0.f) ongle.y += 180.f;

	return ongle;
}

Vector3 getLoc(Vector3 Loc)
{
	Vector3 Location = Vector3(Loc.x, Loc.y, Loc.z);

	return Location;
}


auto GetViewState() -> uintptr_t
{
	TArray<uintptr_t> ViewState = read<TArray<uintptr_t>>(cache::local_players + 0xD0);
	return ViewState.Get(1);
}




camera get_view_point()
{
	auto mProjection = read<FMatrix>(GetViewState() + 0x940);
	cache::local_camera.rotation.x = RadiansToDegrees(std::asin(mProjection.ZPlane.W));
	cache::local_camera.rotation.y = RadiansToDegrees(std::atan2(mProjection.YPlane.W, mProjection.XPlane.W));
	cache::local_camera.rotation.z = 0.0;
	cache::local_camera.location.x = mProjection.m[3][0];
	cache::local_camera.location.y = mProjection.m[3][1];
	cache::local_camera.location.z = mProjection.m[3][2];
	float FieldOfView = atanf(1 / read<double>(GetViewState() + 0x740)) * 2;
	cache::local_camera.fov = (FieldOfView) * (180.f / M_PI); return cache::local_camera;
}


Vector2 project_world_to_screen(Vector3 world_location)
{
	cache::local_camera = get_view_point();
	D3DMATRIX temp_matrix = to_matrix(cache::local_camera.rotation);
	Vector3 vaxisx = Vector3(temp_matrix.m[0][0], temp_matrix.m[0][1], temp_matrix.m[0][2]);
	Vector3 vaxisy = Vector3(temp_matrix.m[1][0], temp_matrix.m[1][1], temp_matrix.m[1][2]);
	Vector3 vaxisz = Vector3(temp_matrix.m[2][0], temp_matrix.m[2][1], temp_matrix.m[2][2]);
	Vector3 vdelta = world_location - cache::local_camera.location;
	Vector3 vtransformed = Vector3(vdelta.dot(vaxisy), vdelta.dot(vaxisz), vdelta.dot(vaxisx));
	if (vtransformed.z < 1) vtransformed.z = 1;

	return Vector2(options::screen_center_x + vtransformed.x * ((options::screen_center_x / tanf(cache::local_camera.fov * M_PI / 360))) / vtransformed.z, options::screen_center_y - vtransformed.y * ((options::screen_center_x / tanf(cache::local_camera.fov * M_PI / 360))) / vtransformed.z);
}





//static auto get_entity_bone(uintptr_t skeletal_mesh, int bone_index) -> Vector3 {
//	static thread_local std::unordered_map<uintptr_t, uintptr_t> bone_array_cache;
//	static thread_local std::chrono::steady_clock::time_point last_cache_clear = std::chrono::steady_clock::now();
//
//	auto now = std::chrono::steady_clock::now();
//	if (std::chrono::duration_cast<std::chrono::seconds>(now - last_cache_clear).count() > 5) {
//		bone_array_cache.clear();
//		last_cache_clear = now;
//	}
//
//	uintptr_t bone_array;
//	auto cached = bone_array_cache.find(skeletal_mesh);
//
//	if (cached != bone_array_cache.end()) {
//		bone_array = cached->second;
//	}
//	else {
//
//		bone_array = read<uintptr_t>(skeletal_mesh + BONE_ARRAY);
//		if (!bone_array) {
//			bone_array = read<uintptr_t>(skeletal_mesh + BONE_ARRAY + 0x10);
//		}
//		bone_array_cache[skeletal_mesh] = bone_array;
//	}
//
//	if (!bone_array) return Vector3(0, 0, 0);
//
//	__m128 bone_matrix[4];
//	__m128 component_matrix[4];
//
//	FTransform bone = read<FTransform>(bone_array + (bone_index * 0x60));
//
//	FTransform component_to_world = read<FTransform>(skeletal_mesh + COMPONENT_TO_WORLD);
//
//	D3DMATRIX bone_m = bone.to_matrix_with_scale();
//	D3DMATRIX comp_m = component_to_world.to_matrix_with_scale();
//
//	// Load matrices into SIMD registers
//	for (int i = 0; i < 4; i++) {
//		bone_matrix[i] = _mm_loadu_ps(&bone_m.m[i][0]);
//		component_matrix[i] = _mm_loadu_ps(&comp_m.m[i][0]);
//	}
//
//	__m128 result[4];
//	for (int i = 0; i < 4; i++) {
//		__m128 row = bone_matrix[i];
//
//		__m128 e0 = _mm_shuffle_ps(row, row, _MM_SHUFFLE(0, 0, 0, 0));
//		__m128 e1 = _mm_shuffle_ps(row, row, _MM_SHUFFLE(1, 1, 1, 1));
//		__m128 e2 = _mm_shuffle_ps(row, row, _MM_SHUFFLE(2, 2, 2, 2));
//		__m128 e3 = _mm_shuffle_ps(row, row, _MM_SHUFFLE(3, 3, 3, 3));
//
//		__m128 m0 = _mm_mul_ps(e0, component_matrix[0]);
//		__m128 m1 = _mm_mul_ps(e1, component_matrix[1]);
//		__m128 m2 = _mm_mul_ps(e2, component_matrix[2]);
//		__m128 m3 = _mm_mul_ps(e3, component_matrix[3]);
//
//		result[i] = _mm_add_ps(_mm_add_ps(m0, m1), _mm_add_ps(m2, m3));
//	}
//
//	float final_matrix[4];
//	_mm_storeu_ps(final_matrix, result[3]);
//
//	return Vector3(final_matrix[0], final_matrix[1], final_matrix[2]);
//}



Vector3 get_entity_bone(uintptr_t mesh, int bone_id)
{
	uintptr_t bone_array = read<uintptr_t>(mesh + BONE_ARRAY);
	if (bone_array == 0) bone_array = read<uintptr_t>(mesh + BONE_ARRAY_CACHE); // 0x10
	FTransform bone = read<FTransform>(bone_array + (bone_id * 0x60));
	FTransform component_to_world = read<FTransform>(mesh + COMPONENT_TO_WORLD);
	D3DMATRIX matrix = matrix_multiplication(bone.to_matrix_with_scale(), component_to_world.to_matrix_with_scale());
	return Vector3(matrix._41, matrix._42, matrix._43);
}

bool IsVisible(uintptr_t mesh)
{
	auto Seconds = read<double>(cache::uworld + 0x160);
	auto LastRenderTime = read<float>(mesh + 0x32C);
	return Seconds - LastRenderTime <= 0.06f;
}


std::wstring string_to_wstring(const std::string& str) {
	if (str.empty()) return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstr(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], size_needed);
	return wstr;
}

std::string wstring_to_utf8(const std::wstring& wstr) {
	if (wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string str(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], size_needed, NULL, NULL);
	return str;
}

void radar_range(float* x, float* y, float range)
{
	if (fabs((*x)) > range || fabs((*y)) > range)
	{
		if ((*y) > (*x))
		{
			if ((*y) > -(*x))
			{
				(*x) = range * (*x) / (*y);
				(*y) = range;
			}
			else
			{
				(*y) = -range * (*y) / (*x);
				(*x) = -range;
			}
		}
		else
		{
			if ((*y) > -(*x))
			{
				(*y) = range * (*y) / (*x);
				(*x) = range;
			}
			else
			{
				(*x) = -range * (*x) / (*y);
				(*y) = -range;
			}
		}
	}
}




std::string get_player_platform(uintptr_t player_state, ImColor& resultColor)
{
	std::string result;

	__int64 FString = read<__int64>(player_state + PLATFORM);
	uintptr_t test_platform = read<uintptr_t>(player_state + PLATFORM);

	int length = static_cast<int>(read<__int64>(FString + 16));

	wchar_t platform[64] = { 0 };
	if (test_platform) {
		mem::read_memory((uintptr_t)test_platform, platform, sizeof(platform));
	}

	std::wstring platform_wstr(platform);
	std::string platform_str(platform_wstr.begin(), platform_wstr.end());

	resultColor = ImColor(255, 255, 255);

	if (platform_str == "XBL") {
		result = "XBOX ONE";
		resultColor = ImColor(0, 255, 0);
	}
	else if (platform_str == "PSN") {
		result = "PLAYSTATION 4";
		resultColor = ImColor(0, 0, 255);
	}
	else if (platform_str == "PS5") {
		result = "PLAYSTATION 5";
		resultColor = ImColor(0, 0, 200);
	}
	else if (platform_str == "XSX") {
		result = "XBOX SERIES S/X";
		resultColor = ImColor(0, 128, 0);
	}
	else if (platform_str == "SWT") {
		result = "NINTENDO";
		resultColor = ImColor(255, 0, 0);
	}
	else if (platform_str == "WIN") {
		result = "WINDOWS";
		resultColor = ImColor(255, 255, 255);
	}
	else if (platform_str == "MOBIL-A") {
		result = "ANDROID";
		resultColor = ImColor(0, 255, 0);
	}
	else if (platform_str == "MOBIL-I") {
		result = "IOS";
		resultColor = ImColor(0, 122, 255);
	}

	return result;
}


bool is_dead(uintptr_t pawn_private)
{
	return (read<char>(pawn_private + 0x718) >> 5) & 1;
}

std::string get_player_name(uintptr_t playerstate)
{
	__int64 FString = read<__int64>(playerstate + PLAYERNAME);
	int Length = read<int>(FString + 16);
	uintptr_t FText = read<__int64>(FString + 8);

	if (Length == 0) return std::string("BOT");

	wchar_t* NameBuffer = new wchar_t[Length];
	//mem::read_physical((void*)(FText), NameBuffer, Length * sizeof(wchar_t));

	char v21;
	int v22;
	int i;
	int v25;
	WORD* v23;

	v21 = Length - 1;
	if (!(DWORD)Length)
		v21 = 0;
	v22 = 0;
	v23 = (WORD*)NameBuffer;
	for (i = (v21) & 3; ; *v23++ += i & 7)
	{
		v25 = Length - 1;
		if (!(DWORD)Length)
			v25 = 0;
		if (v22 >= v25)
			break;
		i += 3;
		++v22;
	}

	std::wstring wbuffer{ NameBuffer };
	delete[] NameBuffer;
	return std::string(wbuffer.begin(), wbuffer.end());
}


inline std::wstring MBytesToWString(const char* lpcszString)
{
	int len = strlen(lpcszString);
	int unicodeLen = ::MultiByteToWideChar(CP_ACP, 0, lpcszString, -1, NULL, 0);
	wchar_t* pUnicode = new wchar_t[unicodeLen + 1];
	memset(pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
	::MultiByteToWideChar(CP_ACP, 0, lpcszString, -1, (LPWSTR)pUnicode, unicodeLen);
	std::wstring wString = (wchar_t*)pUnicode;
	delete[] pUnicode;
	return wString;
}
inline std::string WStringToUTF8(const wchar_t* lpwcszWString)
{
	char* pElementText;
	int iTextLen = ::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, NULL, 0, NULL, NULL);
	pElementText = new char[iTextLen + 1];
	memset((void*)pElementText, 0, (iTextLen + 1) * sizeof(char));
	::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, pElementText, iTextLen, NULL, NULL);
	std::string strReturn(pElementText);
	delete[] pElementText;
	return strReturn;
}

static ImFont* Verdana, * Verdana2, * DefaultFont;

bool is_in_screen(Vector2 screen_location)
{
	if (screen_location.x > 0 && screen_location.x < options::width && screen_location.y > 0 && screen_location.y < options::height)
	{
		return true;
	}
	else
	{
		return false;
	}
}

typedef struct items
{
	uintptr_t actor;
	std::string name;
	bool is_pickup;
	float distance;
} items;
std::vector<items> inline items_list;
std::vector<items> items_temp_list;

typedef struct actors
{
	uintptr_t player_state;
	uintptr_t pawn_private;
	uintptr_t mesh;
	uintptr_t root_component;
	Vector3 relative_location;
	std::string username;
	std::string weapon;
} actors;
std::vector<actors> inline actor_list;
std::vector<actors> actor_temp_list;


