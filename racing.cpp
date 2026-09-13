// =====================================================================
//  RACING 3D v5  -  5 sirkuit + menu pemilihan
//  Compile:
//    g++ -O2 -std=c++17 racing5.cpp -o racing5 \
//        $(pkg-config --cflags --libs raylib) -lm
// =====================================================================
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstring>

#ifndef RAD2DEG
#define RAD2DEG (180.0f / PI)
#endif

// ----------------------------- Konstanta -----------------------------
static const int    NSEG        = 800;
static const float  ROAD_HALF   = 15.0f;
static const float  KERB_W      = 1.6f;
static const float  WALL_OFF    = ROAD_HALF + KERB_W + 2.5f;
static const int    TOTAL_LAPS  = 3;
static const int    NUM_AI      = 5;

// --------------------------- State game ------------------------------
enum GameState { STATE_MENU = 0, STATE_COUNTDOWN, STATE_RACING, STATE_FINISHED };

// ----------------------------- Data trek -----------------------------
static std::vector<Vector3> gCenter;
static std::vector<Vector3> gNormal;
static std::vector<Vector3> gRacing;
static std::vector<float>   gCurv;
static std::vector<bool>    gIsCorner;
static float                gTrackLen = 0.0f;

// --------------------------- Daftar layout ---------------------------
struct Layout {
    const char*          name;
    const char*          desc;
    std::vector<Vector3> cps;
};

static std::vector<Layout> gLayouts;

static void InitLayouts()
{
    gLayouts.clear();

    // 0. Oval klasik dengan chicane
    gLayouts.push_back({
        "Sunset Oval",
        "Oval 1200x800 dengan chicane di sisi bawah",
        {
            {   0, 0,  400 }, { 300, 0,  400 }, { 500, 0,  300 },
            { 600, 0,    0 }, { 500, 0, -300 }, { 300, 0, -400 },
            { 150, 0, -400 }, {  75, 0, -370 }, {   0, 0, -400 },
            { -75, 0, -370 }, {-150, 0, -400 }, {-300, 0, -400 },
            {-500, 0, -300 }, {-600, 0,    0 }, {-500, 0,  300 },
            {-300, 0,  400 },
        }
    });

    // 1. Tri-oval speedway (cepat, sudut lebar)
    gLayouts.push_back({
        "Tri-Oval Speedway",
        "Sirkuit cepat dengan 3 sudut lebar, cocok untuk balapan ketat",
        {
            {   0, 0,  550 }, { 500, 0,  350 }, { 700, 0,    0 },
            { 500, 0, -350 }, {   0, 0, -550 }, {-500, 0, -350 },
            {-700, 0,    0 }, {-500, 0,  350 },
        }
    });

    // 2. Sirkuit teknis dengan banyak tikungan
    gLayouts.push_back({
        "Monako Mini",
        "Sirkuit teknis dengan banyak tikungan tajam dan bump",
        {
            {   0, 0,  400 }, { 300, 0,  400 }, { 500, 0,  250 },
            { 400, 0,  100 }, { 550, 0,  -50 }, { 450, 0, -250 },
            { 250, 0, -400 }, {   0, 0, -450 }, {-250, 0, -400 },
            {-450, 0, -250 }, {-550, 0,    0 }, {-450, 0,  250 },
            {-250, 0,  400 },
        }
    });

    // 3. Grand Prix (lurus panjang, hairpin, esses)
    gLayouts.push_back({
        "Grand Prix Circuit",
        "Layout panjang dengan lurus utama, hairpin, dan esses",
        {
            {   0, 0,  500 }, { 400, 0,  480 }, { 700, 0,  380 },
            { 850, 0,  150 }, { 800, 0, -150 }, { 900, 0, -350 },
            { 700, 0, -550 }, { 350, 0, -600 }, {   0, 0, -550 },
            {-350, 0, -600 }, {-700, 0, -450 }, {-850, 0, -150 },
            {-750, 0,  200 }, {-550, 0,  350 }, {-300, 0,  480 },
        }
    });

    // 4. Sirkuit kart kompak dengan S-curve
    gLayouts.push_back({
        "Karting Sirkuit",
        "Sirkuit kecil dengan S-curve cepat, banyak manuver",
        {
            {   0, 0,  280 }, { 180, 0,  250 }, { 260, 0,  100 },
            { 180, 0,   50 }, { 260, 0,  -50 }, { 180, 0, -150 },
            {   0, 0, -260 }, {-180, 0, -180 }, {-260, 0,    0 },
            {-180, 0,  180 },
        }
    });
}

// -------------------------------- Utils ------------------------------
static float WrapAngle(float a)
{
    while (a >  PI) a -= 2.0f * PI;
    while (a < -PI) a += 2.0f * PI;
    return a;
}

static Vector3 CatmullRom3(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, float t)
{
    float t2 = t * t, t3 = t2 * t;
    Vector3 r;
    r.x = 0.5f * ((2*p1.x) + (-p0.x + p2.x)*t
        + (2*p0.x - 5*p1.x + 4*p2.x - p3.x)*t2
        + (-p0.x + 3*p1.x - 3*p2.x + p3.x)*t3);
    r.y = 0.0f;
    r.z = 0.5f * ((2*p1.z) + (-p0.z + p2.z)*t
        + (2*p0.z - 5*p1.z + 4*p2.z - p3.z)*t2
        + (-p0.z + 3*p1.z - 3*p2.z + p3.z)*t3);
    return r;
}

// ------------------------ Bangun sirkuit -----------------------------
static void InitTrack(int layoutId)
{
    if (layoutId < 0 || layoutId >= (int)gLayouts.size()) layoutId = 0;
    const std::vector<Vector3>& cps = gLayouts[layoutId].cps;
    int NCP = (int)cps.size();

    // Sample padat via Catmull-Rom
    std::vector<Vector3> dense;
    const int SPC = 80;
    for (int i = 0; i < NCP; i++) {
        Vector3 p0 = cps[(i - 1 + NCP) % NCP];
        Vector3 p1 = cps[i];
        Vector3 p2 = cps[(i + 1) % NCP];
        Vector3 p3 = cps[(i + 2) % NCP];
        for (int k = 0; k < SPC; k++) {
            float t = (float)k / SPC;
            dense.push_back(CatmullRom3(p0, p1, p2, p3, t));
        }
    }

    // Resample by arc-length
    int DN = (int)dense.size();
    std::vector<float> cum(DN + 1, 0.0f);
    for (int i = 0; i < DN; i++) {
        Vector3 a = dense[i];
        Vector3 b = dense[(i + 1) % DN];
        float dx = b.x - a.x, dz = b.z - a.z;
        cum[i + 1] = cum[i] + sqrtf(dx*dx + dz*dz);
    }
    gTrackLen = cum[DN];

    gCenter.resize(NSEG);
    float stepLen = gTrackLen / NSEG;
    int seg = 0;
    for (int i = 0; i < NSEG; i++) {
        float target = i * stepLen;
        while (seg < DN - 1 && cum[seg + 1] < target) seg++;
        float local = (cum[seg + 1] > cum[seg])
            ? (target - cum[seg]) / (cum[seg + 1] - cum[seg]) : 0.0f;
        gCenter[i] = Vector3Lerp(dense[seg], dense[(seg + 1) % DN], local);
    }

    gNormal.resize(NSEG);
    for (int i = 0; i < NSEG; i++) {
        Vector3 a = gCenter[(i - 1 + NSEG) % NSEG];
        Vector3 b = gCenter[(i + 1) % NSEG];
        Vector3 t = Vector3Normalize(Vector3Subtract(b, a));
        gNormal[i] = Vector3{ t.z, 0.0f, -t.x };
    }

    gCurv.resize(NSEG);
    for (int i = 0; i < NSEG; i++) {
        Vector3 a = gCenter[(i - 1 + NSEG) % NSEG];
        Vector3 b = gCenter[i];
        Vector3 c = gCenter[(i + 1) % NSEG];
        Vector3 t1 = Vector3Normalize(Vector3Subtract(b, a));
        Vector3 t2 = Vector3Normalize(Vector3Subtract(c, b));
        gCurv[i] = t1.x * t2.z - t1.z * t2.x;
    }

    gIsCorner.resize(NSEG);
    for (int i = 0; i < NSEG; i++) {
        float s = 0.0f;
        for (int k = -10; k <= 10; k++) s += fabsf(gCurv[(i + k + NSEG) % NSEG]);
        gIsCorner[i] = (s > 0.30f);
    }

    // Racing line
    std::vector<float> off(NSEG, 0.0f);
    for (int i = 0; i < NSEG; i++) {
        float s = 0.0f;
        for (int k = -12; k <= 12; k++) s += gCurv[(i + k + NSEG) % NSEG];
        float avg = s / 25.0f;
        off[i] = Clamp(-avg * ROAD_HALF * 5.0f,
                       -ROAD_HALF * 0.55f, ROAD_HALF * 0.55f);
    }
    for (int pass = 0; pass < 12; pass++) {
        std::vector<float> tmp = off;
        for (int i = 0; i < NSEG; i++)
            off[i] = (tmp[(i - 1 + NSEG) % NSEG] + 2.0f * tmp[i]
                      + tmp[(i + 1) % NSEG]) * 0.25f;
    }
    gRacing.resize(NSEG);
    for (int i = 0; i < NSEG; i++)
        gRacing[i] = Vector3Add(gCenter[i], Vector3Scale(gNormal[i], off[i]));
}

// ------------------- Pencarian LOKAL ---------------------------------
struct TrackHit { int index; float dist; float lateral; };

static TrackHit NearestLocal(Vector3 p, const std::vector<Vector3>& path,
                             int centerIdx, int radius = 60)
{
    TrackHit best{ centerIdx, 1e9f, 0.0f };
    for (int k = -radius; k <= radius; k++) {
        int i = (centerIdx + k + NSEG * 4) % NSEG;
        Vector3 a = path[i];
        Vector3 b = path[(i + 1) % NSEG];
        float abx = b.x - a.x, abz = b.z - a.z;
        float apx = p.x - a.x, apz = p.z - a.z;
        float len2 = abx*abx + abz*abz;
        float t = (len2 > 1e-4f) ? ((apx*abx + apz*abz) / len2) : 0.0f;
        t = Clamp(t, 0.0f, 1.0f);
        float qx = a.x + abx * t;
        float qz = a.z + abz * t;
        float dx = p.x - qx, dz = p.z - qz;
        float d = dx*dx + dz*dz;
        if (d < best.dist) { best.dist = d; best.index = i; }
    }
    best.dist = sqrtf(best.dist);
    Vector3 c0 = path[best.index];
    Vector3 n  = gNormal[best.index];
    best.lateral = (p.x - c0.x) * n.x + (p.z - c0.z) * n.z;
    return best;
}

// ----------------------------- Mobil ---------------------------------
struct Car {
    Vector3 pos = { 0, 0, 0 };
    float   yaw = 0.0f;
    float   speed = 0.0f;
    float   steer = 0.0f;
    int     lastIdx = 0;
    int     lap = 0;
    float   lapTime = 0.0f;
    float   bestLap = 0.0f;
    bool    onRoad = true;
    bool    isPlayer = false;
    int     rank = 1;
    Color   color = RED;
    float   skill = 1.0f;
    float   aggression = 1.0f;
    float   offsetBias = 0.0f;
    float   stuckTimer = 0.0f;
    float   reverseTimer = 0.0f;
};

// ------------------------- Gambar mobil 3D ---------------------------
static void DrawCar3D(const Car& c)
{
    rlPushMatrix();
    rlTranslatef(c.pos.x, 0.02f, c.pos.z);
    rlRotatef(c.yaw * RAD2DEG, 0.0f, 1.0f, 0.0f);

    Color tyre     = { 20, 20, 24, 255 };
    Color rim      = { 190, 190, 195, 255 };
    Color body     = c.color;
    Color darkBody = { (unsigned char)(body.r * 0.6f),
                       (unsigned char)(body.g * 0.6f),
                       (unsigned char)(body.b * 0.6f), 255 };
    Color glass    = { 28, 42, 62, 255 };

    float wr = 0.42f, ww = 0.36f;
    Vector3 wpos[4] = {
        {  1.05f, wr,  1.45f }, { -1.05f, wr,  1.45f },
        {  1.05f, wr, -1.45f }, { -1.05f, wr, -1.45f }
    };
    for (int i = 0; i < 4; i++) {
        DrawCube(wpos[i], ww, wr*2, wr*2, tyre);
        DrawCube(Vector3{ wpos[i].x * 1.06f, wr, wpos[i].z },
                 0.10f, wr*1.4f, wr*1.4f, rim);
    }

    DrawCube(Vector3{ 0.0f, 0.75f, 0.0f }, 2.20f, 0.55f, 4.10f, body);
    DrawCubeWires(Vector3{ 0.0f, 0.75f, 0.0f }, 2.20f, 0.55f, 4.10f, BLACK);
    DrawCube(Vector3{ 0.0f, 1.20f, -0.15f }, 1.40f, 0.50f, 1.65f, glass);
    DrawCube(Vector3{ 0.0f, 1.05f, -1.65f }, 1.70f, 0.35f, 0.80f, body);
    DrawCube(Vector3{ 0.0f, 0.72f, 2.20f }, 1.30f, 0.42f, 0.60f, body);

    DrawCube(Vector3{ 0.0f, 0.55f, 2.55f }, 2.20f, 0.10f, 0.55f, darkBody);
    DrawCube(Vector3{  0.85f, 0.65f, 2.30f }, 0.12f, 0.30f, 0.55f, darkBody);
    DrawCube(Vector3{ -0.85f, 0.65f, 2.30f }, 0.12f, 0.30f, 0.55f, darkBody);

    DrawCube(Vector3{ 0.0f, 1.40f, -2.15f }, 2.05f, 0.10f, 0.50f, darkBody);
    DrawCube(Vector3{  0.75f, 1.15f, -2.15f }, 0.15f, 0.55f, 0.30f, darkBody);
    DrawCube(Vector3{ -0.75f, 1.15f, -2.15f }, 0.15f, 0.55f, 0.30f, darkBody);

    DrawCube(Vector3{  1.00f, 0.75f, -0.30f }, 0.42f, 0.42f, 1.90f, darkBody);
    DrawCube(Vector3{ -1.00f, 0.75f, -0.30f }, 0.42f, 0.42f, 1.90f, darkBody);

    DrawCube(Vector3{  0.55f, 0.72f, 2.55f }, 0.32f, 0.16f, 0.06f, WHITE);
    DrawCube(Vector3{ -0.55f, 0.72f, 2.55f }, 0.32f, 0.16f, 0.06f, WHITE);
    DrawCube(Vector3{  0.55f, 0.75f, -2.42f }, 0.32f, 0.16f, 0.06f, RED);
    DrawCube(Vector3{ -0.55f, 0.75f, -2.42f }, 0.32f, 0.16f, 0.06f, RED);

    rlPopMatrix();
}

// ============== RENDER JALAN: KUBUS TUMPANG-TINDIH ===================
static void DrawRoad()
{
    Color asphalt = { 78, 78, 86, 255 };
    for (int i = 0; i < NSEG; i++) {
        int j = (i + 1) % NSEG;
        Vector3 a = gCenter[i];
        Vector3 b = gCenter[j];
        Vector3 d = Vector3Subtract(b, a);
        float segLen = sqrtf(d.x*d.x + d.z*d.z) * 1.15f;
        float yaw    = atan2f(d.x, d.z);

        rlPushMatrix();
        rlTranslatef(a.x, 0.0f, a.z);
        rlRotatef(yaw * RAD2DEG, 0.0f, 1.0f, 0.0f);
        DrawCube(Vector3{ 0.0f, -0.05f, segLen * 0.5f },
                 ROAD_HALF * 2.0f, 0.10f, segLen, asphalt);
        rlPopMatrix();
    }
}

static void DrawEdgeLines()
{
    Color white = { 240, 240, 240, 255 };
    for (int i = 0; i < NSEG; i++) {
        int j = (i + 1) % NSEG;
        Vector3 a = gCenter[i];
        Vector3 b = gCenter[j];
        Vector3 d = Vector3Subtract(b, a);
        float segLen = sqrtf(d.x*d.x + d.z*d.z) * 1.15f;
        float yaw    = atan2f(d.x, d.z);

        rlPushMatrix();
        rlTranslatef(a.x, 0.0f, a.z);
        rlRotatef(yaw * RAD2DEG, 0.0f, 1.0f, 0.0f);
        DrawCube(Vector3{ -ROAD_HALF + 0.35f, 0.0f, segLen * 0.5f },
                 0.55f, 0.06f, segLen, white);
        DrawCube(Vector3{  ROAD_HALF - 0.35f, 0.0f, segLen * 0.5f },
                 0.55f, 0.06f, segLen, white);
        rlPopMatrix();
    }
}

static void DrawCenterDashes()
{
    Color white = { 230, 230, 230, 255 };
    for (int i = 0; i < NSEG; i += 8) {
        int j = (i + 1) % NSEG;
        Vector3 d = Vector3Subtract(gCenter[j], gCenter[i]);
        float segLen = sqrtf(d.x*d.x + d.z*d.z);
        float yaw    = atan2f(d.x, d.z);
        rlPushMatrix();
        rlTranslatef(gCenter[i].x, 0.02f, gCenter[i].z);
        rlRotatef(yaw * RAD2DEG, 0.0f, 1.0f, 0.0f);
        DrawCube(Vector3{ 0, 0, segLen * 0.5f }, 0.35f, 0.04f, segLen * 0.9f, white);
        rlPopMatrix();
    }
}

static void DrawKerbs()
{
    for (int i = 0; i < NSEG; i++) {
        if (!gIsCorner[i]) continue;
        int j = (i + 1) % NSEG;
        Vector3 d = Vector3Subtract(gCenter[j], gCenter[i]);
        float segLen = sqrtf(d.x*d.x + d.z*d.z) * 1.15f;
        float yaw    = atan2f(d.x, d.z);
        Color kc = ((i / 3) % 2 == 0) ? Color{ 220, 40, 40, 255 }
                                      : Color{ 245, 245, 245, 255 };
        rlPushMatrix();
        rlTranslatef(gCenter[i].x, 0.0f, gCenter[i].z);
        rlRotatef(yaw * RAD2DEG, 0.0f, 1.0f, 0.0f);
        DrawCube(Vector3{  ROAD_HALF + KERB_W * 0.5f, 0.005f, segLen * 0.5f },
                 KERB_W, 0.06f, segLen, kc);
        DrawCube(Vector3{ -ROAD_HALF - KERB_W * 0.5f, 0.005f, segLen * 0.5f },
                 KERB_W, 0.06f, segLen, kc);
        rlPopMatrix();
    }
}

static void DrawStartLine()
{
    Vector3 a = gCenter[0];
    Vector3 n = gNormal[0];
    Vector3 t = Vector3Normalize(Vector3Subtract(gCenter[1], gCenter[0]));
    const int tiles = 14;
    float w = (2.0f * ROAD_HALF) / (float)tiles;
    for (int k = 0; k < tiles; k++) {
        float o = -ROAD_HALF + w * (float)k;
        Vector3 c0 = Vector3Add(a, Vector3Scale(n, o));
        c0.y = 0.08f;
        Color col = (k % 2 == 0) ? Color{ 245, 245, 245, 255 }
                                 : Color{ 25, 25, 25, 255 };
        rlPushMatrix();
        rlTranslatef(c0.x, 0.0f, c0.z);
        rlRotatef(atan2f(t.x, t.z) * RAD2DEG, 0, 1, 0);
        DrawCube(Vector3{ 0, 0.08f, 0.9f }, w, 0.02f, 1.8f, col);
        rlPopMatrix();
    }
}

static void DrawWallCubes(float offset, float h, Color col)
{
    for (int i = 0; i < NSEG; i++) {
        int j = (i + 1) % NSEG;
        Vector3 a = Vector3Add(gCenter[i], Vector3Scale(gNormal[i], offset));
        Vector3 aj = Vector3Add(gCenter[j], Vector3Scale(gNormal[j], offset));
        Vector3 dj = Vector3Subtract(aj, a);
        float segLen = sqrtf(dj.x*dj.x + dj.z*dj.z) * 1.15f;
        float yaw = atan2f(dj.x, dj.z);
        rlPushMatrix();
        rlTranslatef(a.x, 0.0f, a.z);
        rlRotatef(yaw * RAD2DEG, 0.0f, 1.0f, 0.0f);
        DrawCube(Vector3{ 0, h * 0.5f, segLen * 0.5f }, 0.30f, h, segLen, col);
        rlPopMatrix();
    }
}

static void DrawBarriers()
{
    Color wall   = { 175, 178, 185, 255 };
    Color stripe = { 220, 40, 40, 255 };
    DrawWallCubes( WALL_OFF, 0.95f, wall);
    DrawWallCubes(-WALL_OFF, 0.95f, wall);
    DrawWallCubes( WALL_OFF + 0.18f, 0.18f, stripe);
    DrawWallCubes(-WALL_OFF - 0.18f, 0.18f, stripe);
    for (int i = 0; i < NSEG; i += 20) {
        for (int side = -1; side <= 1; side += 2) {
            Vector3 p = Vector3Add(gCenter[i],
                        Vector3Scale(gNormal[i], side * (WALL_OFF + 0.30f)));
            DrawCube(Vector3{ p.x, 0.55f, p.z }, 0.30f, 1.1f, 0.30f,
                     Color{ 90, 92, 98, 255 });
        }
    }
}

static void DrawSponsorBanners()
{
    Color cols[] = {
        { 220, 60, 60, 255 }, { 60, 130, 220, 255 }, { 240, 190, 40, 255 },
        { 60, 200, 90, 255 }, { 200, 80, 200, 255 },
    };
    for (int i = 0; i < NSEG; i += 40) {
        Color c = cols[(i / 40) % 5];
        for (int side = -1; side <= 1; side += 2) {
            Vector3 p = Vector3Add(gCenter[i],
                        Vector3Scale(gNormal[i], side * (WALL_OFF - 0.16f)));
            Vector3 d = Vector3Subtract(gCenter[(i + 1) % NSEG], gCenter[i]);
            float yaw = atan2f(d.x, d.z);
            rlPushMatrix();
            rlTranslatef(p.x, 0.0f, p.z);
            rlRotatef(yaw * RAD2DEG, 0, 1, 0);
            DrawCube(Vector3{ 0, 0.55f, 1.6f }, 0.06f, 0.45f, 3.0f, c);
            rlPopMatrix();
        }
    }
}

static void DrawStartGantry()
{
    Vector3 n  = gNormal[0];
    Vector3 pL = Vector3Add(gCenter[0], Vector3Scale(n,  ROAD_HALF + 2.0f));
    Vector3 pR = Vector3Add(gCenter[0], Vector3Scale(n, -ROAD_HALF - 2.0f));

    DrawCube(Vector3{ pL.x, 5.0f, pL.z }, 0.80f, 10.0f, 0.80f, Color{ 45, 45, 58, 255 });
    DrawCube(Vector3{ pR.x, 5.0f, pR.z }, 0.80f, 10.0f, 0.80f, Color{ 45, 45, 58, 255 });

    Vector3 mid = Vector3Lerp(pL, pR, 0.5f);
    Vector3 tan = Vector3Subtract(gCenter[1], gCenter[0]);
    float yaw   = atan2f(tan.x, tan.z);
    rlPushMatrix();
    rlTranslatef(mid.x, 10.2f, mid.z);
    rlRotatef(yaw * RAD2DEG, 0.0f, 1.0f, 0.0f);
    DrawCube(Vector3{ 0, 0, 0 }, ROAD_HALF * 2.0f + 4.5f, 2.0f, 1.3f,
             Color{ 30, 30, 42, 255 });
    int tiles = 18;
    float tw = (ROAD_HALF * 2.0f + 4.0f) / tiles;
    for (int k = 0; k < tiles; k++) {
        Color cc = (k % 2 == 0) ? WHITE : BLACK;
        DrawCube(Vector3{ -ROAD_HALF - 2.0f + tw * (k + 0.5f), 0.0f, 0.68f },
                 tw * 0.92f, 1.7f, 0.06f, cc);
    }
    for (int k = 0; k < 5; k++) {
        float x = -1.4f + k * 0.7f;
        Color lc = (k < 2) ? RED : (k < 4 ? YELLOW : GREEN);
        DrawSphere(Vector3{ x, 0.35f, 0.75f }, 0.20f, lc);
    }
    rlPopMatrix();
}

static void DrawTrees()
{
    for (int i = 0; i < NSEG; i += 14) {
        float r1 = 0.5f + 0.5f * sinf((float)i * 12.9898f);
        float r2 = 0.5f + 0.5f * sinf((float)i * 78.233f);
        for (int side = -1; side <= 1; side += 2) {
            if (r2 < 0.40f) continue;
            float off = WALL_OFF + 14.0f + r1 * 60.0f;
            Vector3 p = Vector3Add(gCenter[i], Vector3Scale(gNormal[i], side * off));
            float h = 5.0f + r1 * 4.0f;
            float cr = 2.8f + r2 * 2.0f;
            DrawCylinder(Vector3{ p.x, 0, p.z }, 0.55f, 0.7f, h, 6,
                         Color{ 92, 62, 38, 255 });
            Color leaf1 = { (unsigned char)(28 + 30 * r1),
                            (unsigned char)(90 + 50 * r2),
                            (unsigned char)(40 + 30 * r1), 255 };
            DrawSphere(Vector3{ p.x, h + 1.2f, p.z }, cr, leaf1);
            DrawSphere(Vector3{ p.x, h + 2.0f, p.z }, cr * 0.72f,
                       Color{ (unsigned char)(leaf1.r + 20),
                              (unsigned char)(leaf1.g + 20),
                              (unsigned char)(leaf1.b + 15), 255 });
        }
    }
}

static void DrawGrandstand(int midIdx, float side)
{
    Vector3 base = Vector3Add(gCenter[midIdx],
                              Vector3Scale(gNormal[midIdx], side * (WALL_OFF + 10.0f)));
    Vector3 tan  = Vector3Subtract(gCenter[(midIdx + 1) % NSEG],
                                   gCenter[(midIdx - 1 + NSEG) % NSEG]);
    float yaw = atan2f(tan.x, tan.z);
    rlPushMatrix();
    rlTranslatef(base.x, 0.0f, base.z);
    rlRotatef(yaw * RAD2DEG, 0.0f, 1.0f, 0.0f);
    DrawCube(Vector3{ 0, 2.0f, 0 }, 50.0f, 4.0f, 14.0f, Color{ 210, 210, 218, 255 });
    DrawCubeWires(Vector3{ 0, 2.0f, 0 }, 50.0f, 4.0f, 14.0f, BLACK);
    DrawCube(Vector3{ 0, 4.6f, 0 }, 52.0f, 0.6f, 16.0f, Color{ 65, 65, 82, 255 });
    for (int k = 0; k < 200; k++) {
        float cx = -23.0f + (k % 32) * 1.45f;
        float cz = -5.6f + ((k / 32) % 5) * 2.8f;
        float cy = 2.6f + ((k / 160) % 2) * 0.9f;
        Color cc = { (unsigned char)(80 + (k * 37) % 175),
                     (unsigned char)(80 + (k * 73) % 175),
                     (unsigned char)(80 + (k * 113) % 175), 255 };
        DrawCube(Vector3{ cx, cy, cz }, 0.6f, 0.9f, 0.6f, cc);
        DrawSphere(Vector3{ cx, cy + 0.6f, cz }, 0.25f,
                   Color{ (unsigned char)(200 - (k*13)%40),
                          (unsigned char)(160 - (k*17)%40),
                          (unsigned char)(130 - (k*11)%40), 255 });
    }
    rlPopMatrix();
}

static void DrawPitBuilding()
{
    int midIdx = NSEG - 20;
    Vector3 base = Vector3Add(gCenter[midIdx],
                              Vector3Scale(gNormal[midIdx], -(WALL_OFF + 9.0f)));
    Vector3 tan = Vector3Subtract(gCenter[(midIdx + 1) % NSEG],
                                  gCenter[(midIdx - 1 + NSEG) % NSEG]);
    float yaw = atan2f(tan.x, tan.z);
    rlPushMatrix();
    rlTranslatef(base.x, 0.0f, base.z);
    rlRotatef(yaw * RAD2DEG, 0.0f, 1.0f, 0.0f);
    DrawCube(Vector3{ 0, 3.0f, 0 }, 55.0f, 6.0f, 12.0f, Color{ 220, 222, 228, 255 });
    DrawCubeWires(Vector3{ 0, 3.0f, 0 }, 55.0f, 6.0f, 12.0f, BLACK);
    for (int k = 0; k < 11; k++) {
        float cx = -22.5f + k * 4.5f;
        DrawCube(Vector3{ cx, 3.5f, 6.02f }, 3.4f, 2.2f, 0.1f,
                 Color{ 50, 80, 120, 255 });
    }
    DrawCube(Vector3{ 0, 6.3f, 0 }, 57.0f, 0.6f, 14.0f, Color{ 70, 70, 85, 255 });
    DrawCube(Vector3{ -18.0f, 7.5f, 0 }, 9.0f, 2.0f, 0.5f, WHITE);
    DrawCube(Vector3{  18.0f, 7.5f, 0 }, 9.0f, 2.0f, 0.5f, RED);
    rlPopMatrix();
}

static void DrawMarshalPosts()
{
    for (int i = 0; i < NSEG; i += 100) {
        for (int side = -1; side <= 1; side += 2) {
            Vector3 p = Vector3Add(gCenter[i],
                        Vector3Scale(gNormal[i], side * (WALL_OFF + 6.0f)));
            DrawCube(Vector3{ p.x, 1.2f, p.z }, 2.0f, 2.4f, 2.0f,
                     Color{ 235, 235, 240, 255 });
            DrawCubeWires(Vector3{ p.x, 1.2f, p.z }, 2.0f, 2.4f, 2.0f, BLACK);
            DrawCube(Vector3{ p.x, 2.6f, p.z }, 2.2f, 0.15f, 2.2f,
                     Color{ 60, 60, 70, 255 });
            DrawCube(Vector3{ p.x + 0.8f, 3.3f, p.z }, 0.05f, 1.2f, 0.05f,
                     Color{ 90, 60, 40, 255 });
            DrawCube(Vector3{ p.x + 1.1f, 3.7f, p.z }, 0.55f, 0.35f, 0.04f,
                     (i % 200 == 0) ? YELLOW : WHITE);
        }
    }
}

static void DrawEnvironment()
{
    for (int i = 0; i < 20; i++) {
        float ang = (float)i / 20.0f * 2.0f * PI;
        float r   = 1400.0f + 250.0f * sinf(ang * 3.7f);
        float x   = r * sinf(ang);
        float z   = r * cosf(ang);
        float h   = 80.0f + 50.0f * sinf(ang * 5.1f);
        DrawSphere(Vector3{ x, -30.0f, z }, 100.0f + h * 0.5f,
                   Color{ 70, 100, 70, 255 });
    }
    for (int i = 0; i < 15; i++) {
        float ang = (float)i * 0.7f;
        float r   = 600.0f + 400.0f * sinf(ang * 1.3f);
        float x   = r * sinf(ang);
        float z   = r * cosf(ang);
        float y   = 130.0f + 30.0f * sinf(ang * 2.1f);
        Color wc  = { 240, 240, 245, 220 };
        DrawSphere(Vector3{ x,     y,     z     }, 15.0f, wc);
        DrawSphere(Vector3{ x + 14, y,     z + 6 }, 12.0f, wc);
        DrawSphere(Vector3{ x - 12, y,     z + 9 }, 11.0f, wc);
        DrawSphere(Vector3{ x + 6,  y - 2, z - 9 }, 13.0f, wc);
    }
}

// ------------------------------ Minimap ------------------------------
static void DrawMiniMap(const std::vector<Car>& cars, int playerIdx)
{
    int w = 220, h = 170;
    int x = GetScreenWidth()  - w - 15;
    int y = GetScreenHeight() - h - 15;

    float minX = 1e9f, maxX = -1e9f, minZ = 1e9f, maxZ = -1e9f;
    for (auto& c : gCenter) {
        minX = fminf(minX, c.x); maxX = fmaxf(maxX, c.x);
        minZ = fminf(minZ, c.z); maxZ = fmaxf(maxZ, c.z);
    }
    float pad = 30.0f;
    minX -= pad; maxX += pad; minZ -= pad; maxZ += pad;
    float rangeX = maxX - minX, rangeZ = maxZ - minZ;
    float scale  = fminf((float)w / rangeX, (float)h / rangeZ) * 0.92f;
    float offX   = x + w * 0.5f - (minX + maxX) * 0.5f * scale;
    float offY   = y + h * 0.5f - (minZ + maxZ) * 0.5f * scale;

    DrawRectangle(x, y, w, h, Fade(BLACK, 0.6f));
    DrawRectangleLines(x, y, w, h, WHITE);

    for (int i = 0; i < NSEG; i++) {
        int j = (i + 1) % NSEG;
        Vector2 a = { offX + gCenter[i].x * scale, offY + gCenter[i].z * scale };
        Vector2 b = { offX + gCenter[j].x * scale, offY + gCenter[j].z * scale };
        DrawLineEx(a, b, 3.2f, GRAY);
    }
    {
        Vector2 p = { offX + gCenter[0].x * scale, offY + gCenter[0].z * scale };
        DrawCircleV(p, 4, WHITE);
    }
    for (size_t i = 0; i < cars.size(); i++) {
        Vector2 p = { offX + cars[i].pos.x * scale, offY + cars[i].pos.z * scale };
        Color c = cars[i].color;
        if ((int)i == playerIdx) {
            DrawCircleV(p, 5, WHITE);
            DrawCircleV(p, 3.6f, c);
        } else {
            DrawCircleV(p, 3.4f, c);
        }
    }
}

// ---------------------------- MENU -----------------------------------
static void DrawMenu(int selected)
{
    ClearBackground(Color{ 15, 17, 26, 255 });

    // Strip dekoratif atas
    DrawRectangle(0, 0, GetScreenWidth(), 8, RED);
    DrawRectangle(0, 8, GetScreenWidth(), 4, WHITE);

    // Judul
    DrawText("RACING 3D", 60, 30, 52, WHITE);
    DrawText("PILIH SIRKUIT", 60, 90, 26, LIGHTGRAY);

    // Garis pemisah
    DrawLine(60, 130, 420, 130, DARKGRAY);

    // Daftar sirkuit
    int listX = 60;
    int listY = 160;
    int itemH = 62;
    for (int i = 0; i < (int)gLayouts.size(); i++) {
        bool sel = (i == selected);
        Color bg = sel ? Color{ 60, 80, 130, 255 } : Color{ 32, 36, 48, 255 };
        Color fg = sel ? WHITE : Color{ 180, 185, 195, 255 };
        Color br = sel ? SKYBLUE : Color{ 60, 65, 80, 255 };

        int y = listY + i * itemH;
        DrawRectangle(listX, y, 360, itemH - 8, bg);
        DrawRectangleLinesEx(Rectangle{ (float)listX, (float)y,
                                        360.0f, (float)(itemH - 8) },
                             2.0f, br);

        if (sel) {
            // Indikator
            DrawRectangle(listX, y, 6, itemH - 8, SKYBLUE);
        }

        DrawText(TextFormat("%d. %s", i + 1, gLayouts[i].name),
                 listX + 20, y + 10, 22, fg);
        DrawText(gLayouts[i].desc,
                 listX + 20, y + 36, 14,
                 sel ? Color{ 200, 210, 230, 255 } : Color{ 130, 135, 145, 255 });
    }

    // Preview box
    int px = 460, py = 150, pw = 760, ph = 480;
    DrawRectangle(px, py, pw, ph, Color{ 25, 30, 42, 255 });
    DrawRectangleLinesEx(Rectangle{ (float)px, (float)py, (float)pw, (float)ph },
                         2.0f, Color{ 80, 90, 110, 255 });

    // Hitung bounding box trek
    float minX = 1e9f, maxX = -1e9f, minZ = 1e9f, maxZ = -1e9f;
    for (auto& c : gCenter) {
        if (c.x < minX) minX = c.x; if (c.x > maxX) maxX = c.x;
        if (c.z < minZ) minZ = c.z; if (c.z > maxZ) maxZ = c.z;
    }
    float pad = 50.0f;
    minX -= pad; maxX += pad; minZ -= pad; maxZ += pad;
    float rangeX = maxX - minX, rangeZ = maxZ - minZ;
    float scale = fminf((float)pw / rangeX, (float)ph / rangeZ) * 0.82f;
    float offX = px + pw * 0.5f - (minX + maxX) * 0.5f * scale;
    float offY = py + ph * 0.5f - (minZ + maxZ) * 0.5f * scale;

    // Grid dekoratif
    for (int gx = 0; gx < pw; gx += 40)
        DrawLine(px + gx, py, px + gx, py + ph, Color{ 40, 45, 60, 100 });
    for (int gy = 0; gy < ph; gy += 40)
        DrawLine(px, py + gy, px + pw, py + gy, Color{ 40, 45, 60, 100 });

    // Shadow trek
    for (int i = 0; i < NSEG; i++) {
        int j = (i + 1) % NSEG;
        Vector2 a = { offX + gCenter[i].x * scale + 2,
                      offY + gCenter[i].z * scale + 2 };
        Vector2 b = { offX + gCenter[j].x * scale + 2,
                      offY + gCenter[j].z * scale + 2 };
        DrawLineEx(a, b, 10.0f, Color{ 0, 0, 0, 120 });
    }
    // Trek utama
    for (int i = 0; i < NSEG; i++) {
        int j = (i + 1) % NSEG;
        Vector2 a = { offX + gCenter[i].x * scale, offY + gCenter[i].z * scale };
        Vector2 b = { offX + gCenter[j].x * scale, offY + gCenter[j].z * scale };
        DrawLineEx(a, b, 8.0f, Color{ 100, 100, 110, 255 });
    }
    // Racing line
    for (int i = 0; i < NSEG; i += 3) {
        int j = (i + 3) % NSEG;
        Vector2 a = { offX + gRacing[i].x * scale, offY + gRacing[i].z * scale };
        Vector2 b = { offX + gRacing[j].x * scale, offY + gRacing[j].z * scale };
        DrawLineEx(a, b, 1.5f, Color{ 255, 200, 60, 160 });
    }

    // Titik start
    Vector2 sp = { offX + gCenter[0].x * scale, offY + gCenter[0].z * scale };
    DrawCircleV(Vector2{ sp.x, sp.y }, 11, WHITE);
    DrawCircleV(sp, 8, GREEN);
    DrawText("START", sp.x + 15, sp.y - 8, 14, GREEN);

    // Info sirkuit
    DrawRectangle(px + 15, py + 15, 400, 60, Fade(BLACK, 0.6f));
    DrawText(gLayouts[selected].name, px + 25, py + 22, 22, YELLOW);
    DrawText(TextFormat("Panjang: %.0f m    |    %.1f km",
             gTrackLen, gTrackLen / 1000.0f),
             px + 25, py + 48, 16, LIGHTGRAY);

    // Footer
    DrawText("Panah atas/bawah: pilih sirkuit   ENTER: mulai balapan",
             60, 600, 18, LIGHTGRAY);
    DrawText("ESC: keluar", 60, 628, 18, GRAY);

    DrawFPS(GetScreenWidth() - 90, 12);
}

// ---------------------- Update mobil pemain --------------------------
static void UpdatePlayer(Car& p, float dt, bool canDrive)
{
    if (!canDrive) { p.speed *= 0.95f; return; }
    float throttle = 0.0f, brake = 0.0f;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    throttle = 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  brake    = 1.0f;
    float steerIn = 0.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  steerIn += 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) steerIn -= 1.0f;
    p.steer = Lerp(p.steer, steerIn, 1.0f - expf(-10.0f * dt));

    float maxSpd = p.onRoad ? 82.0f : 24.0f;
    if (throttle > 0.0f) p.speed += 36.0f * dt;
    if (brake > 0.0f) {
        if (p.speed > 0.5f) p.speed -= 65.0f * dt;
        else                p.speed -= 20.0f * dt;
    }
    float drag = p.onRoad ? 0.35f : 3.2f;
    p.speed -= p.speed * drag * dt;
    if (p.speed > maxSpd) p.speed -= (p.speed - maxSpd) * 4.0f * dt;
    p.speed = Clamp(p.speed, -15.0f, 92.0f);

    float grip = Clamp(fabsf(p.speed) / 10.0f, 0.0f, 1.0f);
    p.yaw += p.steer * 2.5f * grip * dt * (p.speed >= 0.0f ? 1.0f : -1.0f);

    Vector3 fwd = { sinf(p.yaw), 0.0f, cosf(p.yaw) };
    p.pos = Vector3Add(p.pos, Vector3Scale(fwd, p.speed * dt));
    p.pos.y = 0.0f;
}

// ------------------------------ Update AI ----------------------------
static void UpdateAI(Car& c, std::vector<Car>& all, float dt, bool canDrive)
{
    if (!canDrive) { c.speed = 0.0f; return; }

    float segLen = gTrackLen / NSEG;
    TrackHit h   = NearestLocal(c.pos, gCenter, c.lastIdx, 60);
    int curIdx   = h.index;

    Vector3 localN = gNormal[curIdx];
    float lateral  = h.lateral;

    float lookDist = 14.0f + c.speed * 0.55f;
    int aheadIdx   = (curIdx + (int)(lookDist / segLen)) % NSEG;

    Vector3 target = Vector3Add(gRacing[aheadIdx],
                                Vector3Scale(gNormal[aheadIdx], c.offsetBias));

    Vector3 myFwd = { sinf(c.yaw), 0.0f, cosf(c.yaw) };
    float avoid = 0.0f;
    for (auto& o : all) {
        if (&o == &c) continue;
        Vector3 d = Vector3Subtract(o.pos, c.pos);
        float dist = sqrtf(d.x*d.x + d.z*d.z);
        if (dist > 18.0f || dist < 0.5f) continue;
        float fdot = (d.x * myFwd.x + d.z * myFwd.z) / dist;
        if (fdot < 0.5f) continue;
        float sideways = d.x * localN.x + d.z * localN.z;
        float oSide    = (sideways >= 0.0f) ? 1.0f : -1.0f;
        float w        = 1.0f - dist / 18.0f;
        avoid += -oSide * w * 6.0f;
    }
    avoid = Clamp(avoid, -8.0f, 8.0f);

    Vector3 targetC = gCenter[aheadIdx];
    Vector3 targetN = gNormal[aheadIdx];
    float tOff = (target.x - targetC.x) * targetN.x
               + (target.z - targetC.z) * targetN.z;
    tOff += avoid;
    tOff = Clamp(tOff, -ROAD_HALF * 0.80f, ROAD_HALF * 0.80f);
    target = Vector3Add(targetC, Vector3Scale(targetN, tOff));

    if (fabsf(lateral) > ROAD_HALF * 0.75f) {
        float over = (fabsf(lateral) - ROAD_HALF * 0.75f)
                   / (ROAD_HALF * 0.25f);
        float dir  = (lateral > 0.0f) ? -1.0f : 1.0f;
        target = Vector3Add(target,
                            Vector3Scale(localN, dir * over * 15.0f));
    }

    float desiredYaw = atan2f(target.x - c.pos.x, target.z - c.pos.z);
    float diff       = WrapAngle(desiredYaw - c.yaw);
    c.steer = Clamp(diff * 3.5f, -1.0f, 1.0f);

    float maxC = 0.0f;
    for (int k = 4; k < 60; k += 3) {
        int ii = (curIdx + k) % NSEG;
        maxC = fmaxf(maxC, fabsf(gCurv[ii]));
    }
    float targetSpd = 92.0f * c.skill / (1.0f + maxC * 12.0f);
    if (fabsf(lateral) > ROAD_HALF) targetSpd *= 0.55f;
    targetSpd = Clamp(targetSpd, 14.0f, 90.0f);

    if (c.speed < 2.0f && canDrive) {
        c.stuckTimer += dt;
        if (c.stuckTimer > 1.8f) { c.reverseTimer = 0.9f; c.stuckTimer = 0.0f; }
    } else c.stuckTimer = 0.0f;

    if (c.reverseTimer > 0.0f) {
        c.reverseTimer -= dt;
        c.speed = -14.0f;
        c.yaw  -= c.steer * 0.5f * dt;
    } else {
        if (c.speed < targetSpd) c.speed += 34.0f * c.aggression * dt;
        else                     c.speed -= 60.0f * dt;
        c.speed = Clamp(c.speed, 0.0f, 92.0f);
    }

    float grip = Clamp(c.speed / 12.0f, 0.0f, 1.0f);
    c.yaw += c.steer * 2.6f * grip * dt;

    Vector3 fwd = { sinf(c.yaw), 0.0f, cosf(c.yaw) };
    c.pos = Vector3Add(c.pos, Vector3Scale(fwd, c.speed * dt));
    c.pos.y = 0.0f;

    TrackHit h2 = NearestLocal(c.pos, gCenter, c.lastIdx, 60);
    c.lastIdx = h2.index;
}

// ---------------------- Update progres & lap -------------------------
static void UpdateLap(Car& c, bool countLap)
{
    TrackHit h = NearestLocal(c.pos, gCenter, c.lastIdx, 60);
    c.onRoad = (fabsf(h.lateral) < ROAD_HALF);
    int idx = h.index;
    if (countLap) {
        if (c.lastIdx > NSEG * 3 / 4 && idx < NSEG / 4) {
            c.lap++;
            if (c.bestLap <= 0.0f || c.lapTime < c.bestLap)
                c.bestLap = c.lapTime;
            c.lapTime = 0.0f;
        } else if (c.lastIdx < NSEG / 4 && idx > NSEG * 3 / 4) {
            if (c.lap > 0) c.lap--;
        }
    }
    c.lastIdx = idx;
}

static void ComputeRank(std::vector<Car>& cars)
{
    std::vector<std::pair<long long, int>> prog;
    for (size_t i = 0; i < cars.size(); i++) {
        long long p = (long long)cars[i].lap * NSEG + cars[i].lastIdx;
        prog.push_back({ p, (int)i });
    }
    std::sort(prog.begin(), prog.end(),
              [](auto& a, auto& b){ return a.first > b.first; });
    for (size_t r = 0; r < prog.size(); r++)
        cars[prog[r].second].rank = (int)r + 1;
}

static void ResetCars(std::vector<Car>& cars)
{
    for (size_t i = 0; i < cars.size(); i++) {
        Car& c = cars[i];
        c.speed = 0.0f; c.steer = 0.0f; c.lap = 0; c.lapTime = 0.0f;
        c.onRoad = true; c.stuckTimer = 0.0f; c.reverseTimer = 0.0f;

        int idx = (NSEG - 6 - (int)i * 5 + NSEG) % NSEG;
        float off = (i % 2 == 0) ? -4.0f : 4.0f;
        c.pos = Vector3Add(gCenter[idx], Vector3Scale(gNormal[idx], off));
        Vector3 tan = Vector3Subtract(gCenter[(idx + 1) % NSEG], gCenter[idx]);
        c.yaw = atan2f(tan.x, tan.z);
        c.lastIdx = idx;
        if (c.isPlayer) c.bestLap = 0.0f;
    }
}

// =============================== MAIN ================================
int main(void)
{
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "Balap Mobil 3D v5 - 5 Sirkuit");
    SetTargetFPS(60);

    InitLayouts();
    InitTrack(0);

    std::vector<Car> cars;
    Car player{}; player.isPlayer = true; player.color = RED;
    cars.push_back(player);

    const Color aiColors[NUM_AI] = { BLUE, GREEN, ORANGE, PURPLE, YELLOW };
    const float aiSkill[NUM_AI]  = { 0.99f, 0.95f, 0.91f, 0.87f, 0.83f };
    const float aiAggr[NUM_AI]   = { 1.05f, 1.00f, 0.96f, 0.92f, 0.88f };
    const float aiBias[NUM_AI]   = {  1.5f, -1.5f,  0.8f, -0.8f,  0.0f };

    for (int i = 0; i < NUM_AI; i++) {
        Car a{};
        a.isPlayer = false; a.color = aiColors[i];
        a.skill = aiSkill[i]; a.aggression = aiAggr[i]; a.offsetBias = aiBias[i];
        cars.push_back(a);
    }
    ResetCars(cars);

    Camera3D cam = { 0 };
    cam.up = { 0, 1, 0 };
    cam.fovy = 65.0f;
    cam.projection = CAMERA_PERSPECTIVE;
    {
        Vector3 fwd = { sinf(cars[0].yaw), 0, cosf(cars[0].yaw) };
        cam.position = Vector3Add(cars[0].pos,
                        Vector3Add(Vector3Scale(fwd, -14.0f), Vector3{ 0, 7.0f, 0 }));
        cam.target   = Vector3Add(cars[0].pos,
                        Vector3Add(Vector3Scale(fwd, 12.0f), Vector3{ 0, 1.5f, 0 }));
    }

    int   state         = STATE_MENU;
    int   menuSelected  = 0;
    int   menuPrev      = -1;
    int   currentLayout = 0;
    float countdown     = 3.99f;
    float raceTime      = 0.0f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (dt > 0.1f) dt = 0.1f;

        // =================== MENU ===================
        if (state == STATE_MENU) {
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
                menuSelected = (menuSelected + 1) % (int)gLayouts.size();
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
                menuSelected = (menuSelected - 1 + (int)gLayouts.size())
                             % (int)gLayouts.size();

            if (menuSelected != menuPrev) {
                InitTrack(menuSelected);
                menuPrev = menuSelected;
            }

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
                currentLayout = menuSelected;
                InitTrack(currentLayout);
                ResetCars(cars);
                raceTime  = 0.0f;
                countdown = 3.99f;
                state     = STATE_COUNTDOWN;

                Vector3 fwd = { sinf(cars[0].yaw), 0, cosf(cars[0].yaw) };
                cam.position = Vector3Add(cars[0].pos,
                                Vector3Add(Vector3Scale(fwd, -14.0f),
                                           Vector3{ 0, 7.0f, 0 }));
                cam.target   = Vector3Add(cars[0].pos,
                                Vector3Add(Vector3Scale(fwd, 12.0f),
                                           Vector3{ 0, 1.5f, 0 }));
            }

            BeginDrawing();
            DrawMenu(menuSelected);
            EndDrawing();
            continue;
        }

        // ================ COUNTDOWN / RACING ================
        Car& player = cars[0];

        if (state == STATE_COUNTDOWN) {
            countdown -= dt;
            if (countdown <= 0.0f) {
                state = STATE_RACING;
                raceTime = 0.0f;
                for (auto& c : cars) c.lapTime = 0.0f;
            }
        }
        if (state == STATE_RACING) {
            raceTime += dt;
            for (auto& c : cars) c.lapTime += dt;
        }
        bool canDrive = (state == STATE_RACING);

        UpdatePlayer(player, dt, canDrive);
        for (size_t i = 1; i < cars.size(); i++)
            UpdateAI(cars[i], cars, dt, canDrive);

        // Tabrakan antar mobil
        for (size_t i = 0; i < cars.size(); i++) {
            for (size_t j = i + 1; j < cars.size(); j++) {
                Vector3 d = Vector3Subtract(cars[j].pos, cars[i].pos);
                float dist = sqrtf(d.x*d.x + d.z*d.z);
                if (dist < 3.8f && dist > 0.001f) {
                    Vector3 push = Vector3Scale(d, ((3.8f - dist) / dist) * 0.5f);
                    cars[i].pos = Vector3Subtract(cars[i].pos, push);
                    cars[j].pos = Vector3Add(cars[j].pos, push);
                    cars[i].speed *= 0.98f;
                    cars[j].speed *= 0.98f;
                }
            }
        }

        for (auto& c : cars) UpdateLap(c, state == STATE_RACING);
        ComputeRank(cars);

        if (state == STATE_RACING && player.lap >= TOTAL_LAPS)
            state = STATE_FINISHED;

        // Kamera
        {
            Vector3 fwd = { sinf(player.yaw), 0, cosf(player.yaw) };
            float spd = Clamp(fabsf(player.speed) / 85.0f, 0.0f, 1.0f);
            Vector3 desiredPos = Vector3Add(player.pos,
                Vector3Add(Vector3Scale(fwd, -13.0f - spd * 4.0f),
                           Vector3{ 0.0f, 6.5f + spd * 1.8f, 0.0f }));
            Vector3 desiredTarget = Vector3Add(player.pos,
                Vector3Add(Vector3Scale(fwd, 13.0f + spd * 10.0f),
                           Vector3{ 0.0f, 1.6f, 0.0f }));
            float k1 = 1.0f - expf(-7.5f * dt);
            float k2 = 1.0f - expf(-13.0f * dt);
            cam.position = Vector3Lerp(cam.position, desiredPos,    k1);
            cam.target   = Vector3Lerp(cam.target,   desiredTarget, k2);
            cam.fovy     = 65.0f + spd * 14.0f;
        }

        // =========================== RENDER ===========================
        BeginDrawing();
        ClearBackground(Color{ 130, 180, 235, 255 });

        BeginMode3D(cam);
            DrawEnvironment();
            DrawPlane(Vector3{ 0.0f, -0.5f, 0.0f },
                      Vector2{ 6000.0f, 6000.0f }, Color{ 55, 130, 70, 255 });
            DrawRoad();
            DrawEdgeLines();
            DrawKerbs();
            DrawCenterDashes();
            DrawStartLine();
            DrawBarriers();
            DrawSponsorBanners();
            DrawStartGantry();
            DrawPitBuilding();
            DrawGrandstand(80,  -1.0f);
            DrawGrandstand(300,  1.0f);
            DrawGrandstand(560, -1.0f);
            DrawMarshalPosts();
            DrawTrees();
            for (auto& c : cars) DrawCar3D(c);
        EndMode3D();

        // HUD
        DrawRectangle(10, 10, 360, 158, Fade(BLACK, 0.5f));
        DrawText(TextFormat("POSISI    : %d / %d", player.rank, NUM_AI + 1),
                 22, 18, 20, WHITE);
        DrawText(TextFormat("LAP       : %d / %d", player.lap + 1, TOTAL_LAPS),
                 22, 42, 20, WHITE);
        DrawText(TextFormat("WAKTU     : %.2f s", raceTime),
                 22, 66, 20, WHITE);
        DrawText(TextFormat("LAP SEKARANG : %.2f s", player.lapTime),
                 22, 90, 20, LIGHTGRAY);
        DrawText(TextFormat("LAP TERBAIK  : %.2f s", player.bestLap),
                 22, 114, 20, YELLOW);
        DrawText(TextFormat("KECEPATAN : %.0f km/h", fabsf(player.speed) * 3.6f),
                 22, 138, 20, player.onRoad ? SKYBLUE : ORANGE);

        // Nama sirkuit di tengah atas
        const char* tn = gLayouts[currentLayout].name;
        int tnw = MeasureText(tn, 20);
        DrawRectangle(GetScreenWidth()/2 - tnw/2 - 12, 12, tnw + 24, 30,
                      Fade(BLACK, 0.5f));
        DrawText(tn, GetScreenWidth()/2 - tnw/2, 18, 20, WHITE);

        if (!player.onRoad && state == STATE_RACING)
            DrawText("DI LUAR TREK!", 380, 22, 22, ORANGE);

        DrawFPS(GetScreenWidth() - 90, 12);
        DrawMiniMap(cars, 0);

        if (state == STATE_COUNTDOWN) {
            int n = (int)ceilf(countdown);
            if (n > 3) n = 3;
            int lit = 4 - n;
            for (int k = 0; k < 5; k++) {
                int cx = GetScreenWidth() / 2 - 100 + k * 50;
                Color c = (k <= lit) ? RED : Color{ 60, 20, 20, 255 };
                DrawCircle(cx, 130, 22, c);
                DrawCircleLines(cx, 130, 22, BLACK);
            }
            const char* txt = TextFormat("%d", n);
            int w = MeasureText(txt, 140);
            DrawText(txt, GetScreenWidth()/2 - w/2, 200, 140, WHITE);
            DrawText("BERSIAP...", GetScreenWidth()/2 - 95, 370, 32, WHITE);
        }

        if (state == STATE_FINISHED) {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                          Fade(BLACK, 0.72f));
            const char* title = (player.rank == 1) ? "MENANG!" : "SELESAI!";
            Color titleCol    = (player.rank == 1) ? GOLD : WHITE;
            int tw = MeasureText(title, 100);
            DrawText(title, GetScreenWidth()/2 - tw/2, 100, 100, titleCol);
            DrawText(TextFormat("Sirkuit        : %s", gLayouts[currentLayout].name),
                     GetScreenWidth()/2 - 220, 230, 26, LIGHTGRAY);
            DrawText(TextFormat("Posisi akhir   : P%d dari %d",
                     player.rank, NUM_AI + 1),
                     GetScreenWidth()/2 - 220, 270, 28, WHITE);
            DrawText(TextFormat("Total waktu    : %.2f s", raceTime),
                     GetScreenWidth()/2 - 220, 310, 28, WHITE);
            DrawText(TextFormat("Lap terbaik    : %.2f s", player.bestLap),
                     GetScreenWidth()/2 - 220, 350, 28, YELLOW);

            DrawText("[R] Balapan lagi    [M] Pilih sirkuit    [ESC] Keluar",
                     GetScreenWidth()/2 - 300, 460, 24, LIGHTGRAY);

            if (IsKeyPressed(KEY_R)) {
                ResetCars(cars);
                raceTime = 0.0f; countdown = 3.99f;
                state = STATE_COUNTDOWN;
                Vector3 fwd = { sinf(cars[0].yaw), 0, cosf(cars[0].yaw) };
                cam.position = Vector3Add(cars[0].pos,
                                Vector3Add(Vector3Scale(fwd, -14.0f),
                                           Vector3{ 0, 7.0f, 0 }));
                cam.target   = Vector3Add(cars[0].pos,
                                Vector3Add(Vector3Scale(fwd, 12.0f),
                                           Vector3{ 0, 1.5f, 0 }));
            }
            if (IsKeyPressed(KEY_M)) {
                state = STATE_MENU;
                menuPrev = -1;  // trigger re-init
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}