# 🏎️ Racing 3D

Game balap mobil 3D sederhana yang dibuat dengan **C++17** dan **raylib**. Dibuat sebagai proyek pembelajaran grafis 3D dan AI balapan, dengan fokus pada kode yang mudah dibaca dan dimodifikasi.

![Status](https://img.shields.io/badge/status-active-brightgreen)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows%20%7C%20macOS-blue)
![Language](https://img.shields.io/badge/language-C%2B%2B17-orange)
![Library](https://img.shields.io/badge/library-raylib%206.0-red)
![License](https://img.shields.io/badge/license-MIT-green)

---

## 📖 Daftar Isi

- [Tentang Proyek](#-tentang-proyek)
- [Fitur](#-fitur)
- [Screenshot](#-screenshot)
- [Persyaratan](#-persyaratan)
- [Instalasi](#-instalasi)
- [Cara Menjalankan](#-cara-menjalankan)
- [Kontrol](#-kontrol)
- [Daftar Sirkuit](#-daftar-sirkuit)
- [Struktur Proyek](#-struktur-proyek)
- [Menambah Sirkuit Baru](#-menambah-sirkuit-baru)
- [Arsitektur Kode](#-arsitektur-kode)
- [Troubleshooting](#-troubleshooting)
- [Roadmap](#-roadmap)
- [Kontribusi](#-kontribusi)
- [Lisensi](#-lisensi)
- [Kredit](#-kredit)

---

## 🎮 Tentang Proyek

**Racing 3D** adalah game balap mobil 3D yang dibangun sepenuhnya dengan geometri primitif raylib — tanpa aset eksternal, tanpa file model 3D, tanpa tekstur. Semua elemen visual (jalan, mobil, bangunan, penonton, pohon) dirakit dari kubus, bola, dan silinder lewat kode.

Proyek ini bertujuan sebagai:

- **Referensi belajar** grafis 3D dengan C++ dan raylib
- **Contoh AI balapan** dengan racing line, look-ahead, dan overtaking
- **Basis kode** yang mudah dikembangkan untuk eksperimen game balap

---

## ✨ Fitur

### Gameplay
- 🏁 **5 sirkuit berbeda** dengan karakteristik masing-masing
- 🤖 **5 AI pembalap** dengan kepribadian individu (skill, aggression, racing line bias)
- 🏎️ **Fisika balap sederhana**: akselerasi, pengereman, drag, grip
- 📊 **Sistem lap & timing**: lap counter, waktu total, best lap
- 🏆 **Ranking real-time** untuk semua 6 pembalap
- 🔄 **Hitung mundur 3-2-1** dengan lampu start gaya F1
- 🗺️ **Minimap** menampilkan posisi semua mobil

### Visual
- 🛣️ **Jalan aspal** dengan garis tepi putih dan marka tengah putus-putus
- 🔴 **Kerbs merah-putih** otomatis di tikungan
- 🧱 **Armco barrier** dengan post tegak dan stripe merah
- 🏟️ **Grandstand** dengan ratusan penonton warna-warni
- 🏢 **Gedung pit** dengan jendela kaca
- 🚩 **Marshal post** dengan bendera
- 🏔️ **Bukit & awan** di horizon
- 🌳 **Pohon** tersebar di sekitar trek
- 🎏 **Sponsor banners** warna-warni

### AI
- 📈 **Racing line** dihitung otomatis dari curvature
- 👁️ **Look-ahead dinamis** berbasis jarak (bukan indeks)
- 🛑 **Auto-braking** sebelum masuk tikungan
- 🚗 **Overtaking** — menghindari mobil di depan
- 🧲 **Anti-stuck** — mundur otomatis kalau terjebak
- 🎭 **Kepribadian** berbeda: agresif, konservatif, rapi

### Menu
- 🎨 **Menu utama** dengan preview 2D sirkuit
- 📐 **Preview layout** dengan racing line dan titik start
- 📏 **Info sirkuit**: nama, deskripsi, panjang (m / km)

---

## 📸 Screenshot

> 💡 **Untuk maintainer:** Tambahkan screenshot di folder `docs/screenshots/` dan ganti path di bawah.

```
docs/screenshots/
├── menu.png          — Menu pilih sirkuit
├── gameplay1.png     — Balapan di Sunset Oval
├── gameplay2.png     — Di Grand Prix Circuit
├── grandstand.png    — Detail grandstand + penonton
└── finish.png        — Layar finish
```

```markdown
![Menu](docs/screenshots/menu.png)
![Gameplay](docs/screenshots/gameplay1.png)
```

---

## 📋 Persyaratan

| Komponen | Versi Minimum | Keterangan |
|---|---|---|
| **Compiler** | GCC 9+ / Clang 10+ / MSVC 2019+ | Harus mendukung C++17 |
| **CMake** (opsional) | 3.15+ | Untuk build raylib dari source |
| **raylib** | 5.0+ | Sudah termasuk `rlgl.h` |
| **OpenGL** | 3.3+ | GPU apapun dengan driver modern |

### Dependensi Sistem

**Ubuntu / Debian / Linux Mint:**
```bash
sudo apt update
sudo apt install build-essential git cmake \
    libasound2-dev libx11-dev libxrandr-dev libxi-dev \
    libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev \
    libxinerama-dev libwayland-dev libxkbcommon-dev
```

**Fedora:**
```bash
sudo dnf install gcc-c++ make git cmake alsa-lib-devel \
    mesa-libGL-devel libX11-devel libXrandr-devel libXi-devel \
    libXcursor-devel libXinerama-devel wayland-devel libxkbcommon-devel
```

**Arch / Manjaro:**
```bash
sudo pacman -S base-devel git cmake mesa
```

**macOS:**
```bash
xcode-select --install
brew install cmake
```

**Windows:**
- Install [MSYS2](https://www.msys2.org/) atau Visual Studio 2019+
- Atau pakai WSL2 dengan Ubuntu

---

## 🔧 Instalasi

### 1. Install raylib

**Opsi A — Package manager (paling cepat):**

```bash
# Ubuntu 23.04+ / Debian 12+
sudo apt install libraylib-dev

# Fedora
sudo dnf install raylib-devel

# Arch
sudo pacman -S raylib

# macOS
brew install raylib
```

**Opsi B — Build dari source (untuk versi terbaru):**

```bash
git clone https://github.com/raysan5/raylib.git
cd raylib
mkdir build && cd build
cmake -DBUILD_SHARED_LIBS=ON ..
make -j$(nproc)
sudo make install
sudo ldconfig        # Linux only
```

### 2. Clone repository ini

```bash
git clone https://github.com/USERNAME/racing3d.git
cd racing3d
```

### 3. Compile

**Linux / macOS:**
```bash
g++ -O2 -std=c++17 racing5.cpp -o racing3d \
    $(pkg-config --cflags --libs raylib) -lm
```

**Linux (kalau `pkg-config` tidak menemukan raylib):**
```bash
g++ -O2 -std=c++17 racing5.cpp -o racing3d \
    -I/usr/local/include -L/usr/local/lib -lraylib \
    -lGL -lm -lpthread -ldl -lrt -lX11
```

**Windows (MSYS2 MinGW):**
```bash
g++ -O2 -std=c++17 racing5.cpp -o racing3d.exe \
    -lraylib -lopengl32 -lgdi32 -lwinmm
```

**Windows (Visual Studio Developer Command Prompt):**
```cmd
cl /O2 /std:c++17 racing5.cpp /I path\to\raylib\include ^
   /link path\to\raylib\lib\raylib.lib
```

### 4. (Opsional) Build dengan CMake

Buat `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.15)
project(racing3d CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(raylib REQUIRED)

add_executable(racing3d racing5.cpp)
target_link_libraries(racing3d PRIVATE raylib)
```

Lalu:

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
./racing3d
```

---

## 🚀 Cara Menjalankan

Setelah compile berhasil:

```bash
./racing3d
```

Jendela game 1280×720 akan terbuka dengan menu pemilihan sirkuit.

> ⚠️ **Catatan:** Jalankan binary hasil compile (`./racing3d`), **bukan** file source (`./racing5.cpp`).

---

## 🎮 Kontrol

### Menu
| Tombol | Fungsi |
|---|---|
| **↑** / **↓** atau **W** / **S** | Navigasi pilihan sirkuit |
| **ENTER** | Mulai balapan |
| **ESC** | Keluar dari game |

### Saat Balapan
| Tombol | Fungsi |
|---|---|
| **W** / **↑** | Gas |
| **S** / **↓** | Rem (tahan saat maju) / Mundur (saat berhenti) |
| **A** / **←** | Setir kiri |
| **D** / **→** | Setir kanan |
| **ESC** | Keluar dari game |

### Setelah Balapan Selesai
| Tombol | Fungsi |
|---|---|
| **R** | Balapan ulang di sirkuit yang sama |
| **M** | Kembali ke menu pilih sirkuit |
| **ESC** | Keluar dari game |

---

## 🏁 Daftar Sirkuit

| # | Nama | Karakteristik | Tingkat Kesulitan |
|---|------|---------------|:---:|
| 1 | **Sunset Oval** | Oval 1200×800 dengan chicane di sisi bawah | ⭐ |
| 2 | **Tri-Oval Speedway** | 3 sudut lebar, kecepatan tinggi, balapan ketat | ⭐⭐ |
| 3 | **Monako Mini** | Chicane berulang, banyak tikungan tajam | ⭐⭐⭐⭐ |
| 4 | **Grand Prix Circuit** | Lurus panjang + hairpin + esses, layout GP | ⭐⭐⭐⭐⭐ |
| 5 | **Karting Sirkuit** | Kompak, S-curve cepat, banyak manuver | ⭐⭐⭐ |

---

## 📁 Struktur Proyek

```
racing3d/
├── README.md              # Dokumen ini
├── LICENSE                # Lisensi MIT
├── CMakeLists.txt         # (opsional) Build config
├── .gitignore             # Ignore binary, build artifacts
├── racing5.cpp            # Source code utama (single file)
├── docs/
│   └── screenshots/       # Screenshot untuk README
│       ├── menu.png
│       ├── gameplay1.png
│       └── ...
└── build/                 # (gitignored) Output compile
    └── racing3d
```

### `.gitignore` yang disarankan

```gitignore
# Build artifacts
build/
*.o
*.obj
*.exe
racing3d
racing2
racing3

# IDE
.vscode/
.idea/
*.swp
*~

# OS
.DS_Store
Thumbs.db

# Raylib build files (kalau submodule)
raylib/build/
```

---

## 🗺️ Menambah Sirkuit Baru

Sirkuit didefinisikan sebagai **array of control points** yang membentuk loop tertutup. Sistem akan membuat spline Catmull-Rom melalui titik-titik tersebut, lalu me-resample berdasarkan arc-length untuk menghasilkan segmen yang rata.

### Langkah-langkah

Buka `racing5.cpp`, cari fungsi `InitLayouts()`, dan tambahkan entry baru:

```cpp
gLayouts.push_back({
    "Nama Sirkuit Anda",           // nama
    "Deskripsi singkat sirkuit",   // deskripsi (untuk menu)
    {
        {   0, 0,  400 },   // control point 1
        { 300, 0,  400 },   // control point 2
        { 500, 0,  200 },   // ...
        { 400, 0, -100 },
        { 200, 0, -400 },
        {   0, 0, -450 },
        {-200, 0, -400 },
        {-400, 0, -100 },
        {-500, 0,  200 },
        {-300, 0,  400 },
    }
});
```

### Tips Desain Sirkuit

| Aturan | Alasan |
|---|---|
| **Minimal 6 control point** | Loop tertutup butuh minimal 6 titik agar spline tidak aneh |
| **Jarak antar CP jangan beda jauh** | Kalau ada yang 50 unit, ada yang 500 unit, spline akan overshoot |
| **Sudut tajam** = 2-3 CP berdekatan | Menghasilkan hairpin |
| **Sudut lebar** = CP berjarak jauh | Menghasilkan fast sweeper |
| **CP tidak boleh menyilang** | Kecuali ingin figure-8 (belum didukung) |
| **Semakin banyak CP** | Semakin teknis dan panjang |

### Verifikasi Visual

Setelah menambahkan sirkuit, jalankan game → pilih sirkuit baru di menu → preview 2D di kanan akan langsung menampilkan layout. Kalau bentuknya aneh atau menyilang, berarti kontrol point-nya perlu diatur ulang.

---

## 🏗️ Arsitektur Kode

### Alur Program

```
main()
 │
 ├─ InitLayouts()        — definisi 5 sirkuit (control points)
 ├─ InitTrack(layoutId)  — build spline + normal + racing line
 │
 └─ Game Loop
     ├─ STATE_MENU         — pilih sirkuit
     ├─ STATE_COUNTDOWN    — 3..2..1 + lampu start
     ├─ STATE_RACING       — gameplay utama
     └─ STATE_FINISHED     — layar hasil + opsi ulang
```

### Data Struktur Utama

| Struct / Var | Fungsi |
|---|---|
| `Layout` | Definisi sirkuit (nama, deskripsi, control points) |
| `Car` | State mobil (posisi, kecepatan, yaw, lap, skill AI) |
| `TrackHit` | Hasil pencarian titik terdekat di trek |
| `gCenter[]` | Titik tengah jalan (NSEG = 800 titik) |
| `gNormal[]` | Vektor normal (kanan arah laju) |
| `gRacing[]` | Racing line hasil komputasi curvature |
| `gCurv[]` | Curvature bertanda tiap segmen |
| `gIsCorner[]` | Flag apakah segmen adalah tikungan |

### Fungsi Penting

| Fungsi | Deskripsi |
|---|---|
| `InitTrack(layoutId)` | Bangun trek dari control points via Catmull-Rom + arc-length resample |
| `NearestLocal(pos, path, centerIdx, radius)` | Cari titik terdekat **secara lokal** (bukan global) — mencegah ambigu saat dua bagian trek berdekatan |
| `UpdatePlayer(car, dt, canDrive)` | Handle input + fisika pemain |
| `UpdateAI(car, allCars, dt, canDrive)` | AI: racing line, look-ahead, overtaking, anti-stuck |
| `UpdateLap(car, countLap)` | Deteksi lap selesai + update status on-road |
| `ComputeRank(cars)` | Ranking real-time berdasarkan progress |
| `DrawRoad()`, `DrawKerbs()`, dst. | Rendering komponen visual |

### Konsep Kunci

1. **Catmull-Rom Spline** — interpolasi kurva mulus melalui control points
2. **Arc-Length Resampling** — memastikan jarak antar segmen seragam
3. **Racing Line dari Curvature** — offset ke dalam tikungan untuk meminimalkan jarak tempuh
4. **Local Nearest Search** — pencarian titik terdekat dengan radius terbatas dari `lastIdx`
5. **Lateral Offset** — deteksi off-road pakai dot product dengan normal (bukan jarak euclidean)

---

## 🔧 Troubleshooting

### `fatal error: raylib.h: No such file or directory`
raylib belum terinstall. Ulangi [langkah instalasi raylib](#1-install-raylib).

### `Package raylib was not found in the pkg-config search path`
Compile dengan flag manual:
```bash
g++ -O2 -std=c++17 racing5.cpp -o racing3d \
    -I/usr/local/include -L/usr/local/lib -lraylib \
    -lGL -lm -lpthread -ldl -lrt -lX11
```

### `error: 'rlPushMatrix' was not declared in this scope`
Versi raylib ≥6.0 memindahkan fungsi `rl*` ke `rlgl.h`. Pastikan file sumber **sudah include** `#include "rlgl.h"`. Ini sudah ada di kode — kalau masih muncul, cek apakah kode kamu versi lama.

### `error while loading shared libraries: libraylib.so`
```bash
sudo ldconfig
# atau:
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

### `zsh: permission denied: ./racing5.cpp`
File `.cpp` **bukan** binary — itu source code. Yang dijalankan adalah hasil compile (`./racing3d`).

### Game jalan tapi lambat / patah-patah
- Pastikan driver GPU terinstall
- Cek dengan `glxinfo | grep "OpenGL renderer"` — harus menampilkan GPU (bukan `llvmpipe`)
- Kalau pakai `llvmpipe` (software rendering), install driver GPU yang sesuai

### Jalan tidak terlihat / mobil tenggelam
- Cek kompatibilitas raylib: minimal versi 5.0
- Update driver GPU

### AI keluar lintasan terus
- Biasanya karena `ROAD_HALF` terlalu kecil untuk sirkuit dengan tikungan tajam
- Coba naikkan `ROAD_HALF` di baris konstanta

---

## 🚧 Roadmap

Ide pengembangan lanjutan (PR sangat welcome):

### Gameplay
- [ ] **Mode multiplayer** (split-screen lokal)
- [ ] **Pilih mobil** dengan statistik berbeda (kecepatan / handling / akselerasi)
- [ ] **Damage system** — mobil rusak kalau nabrak barrier
- [ ] **Pit stop** — ganti ban untuk grip lebih baik
- [ ] **Weather** — hujan, kabut, siang/malam
- [ ] **Ghost replay** — lawan waktu sendiri

### AI
- [ ] **Behavior tree** untuk AI yang lebih kompleks
- [ ] **Adaptive difficulty** — AI menyesuaikan skill dengan pemain
- [ ] **Rubber-banding** opsional (catch-up AI)
- [ ] **Multiple racing lines** per sirkuit
- [ ] **Pit strategy AI** (kapan masuk pit)

### Visual
- [ ] **Model mobil lebih detail** (atau load dari file `.obj` / `.glb`)
- [ ] **Bayangan dinamis** dengan shadow map
- [ ] **Partikel** (asap ban, percikan, debu)
- [ ] **Efek cuaca** (hujan partikel, riak di aspal)
- [ ] **Skybox** dengan tekstur langit
- [ ] **Bloom / HDR** untuk lampu start dan lampu mobil
- [ ] **Motion blur** saat kecepatan tinggi

### Audio
- [ ] **Sound engine** mobil (pitch naik dengan RPM)
- [ ] **SFX** tabrakan, ban berdecit, klakson
- [ ] **BGM** menu + balapan
- [ ] **Ambience** kerumunan penonton

### Teknis
- [ ] **Pisah ke beberapa file** (`.h` / `.cpp`) untuk maintainability
- [ ] **Modular sirkuit** via file eksternal (JSON / teks)
- [ ] **Unit test** untuk fungsi geometri (NearestLocal, InitTrack, dll.)
- [ ] **CI/CD** dengan GitHub Actions untuk build di 3 OS
- [ ] **Release binary** untuk Linux, Windows, macOS
- [ ] **Gamepad / controller** support
- [ ] **Rebindable keys**

### Platform
- [ ] **Web build** (via Emscripten) — playable di browser
- [ ] **Android** (raylib support)
- [ ] **Steam Deck** optimized (support gamepad + cloud save)

---

## 🤝 Kontribusi

Kontribusi dari siapa saja sangat terbuka! Baik untuk perbaikan bug, fitur baru, sirkuit baru, atau perbaikan dokumentasi.

### Cara Kontribusi

1. **Fork** repository ini
2. Buat **branch fitur**:
   ```bash
   git checkout -b fitur/nama-fitur-baru
   ```
3. **Commit** perubahan:
   ```bash
   git commit -m "feat: tambah sirkuit Alpen"
   ```
4. **Push** ke branch:
   ```bash
   git push origin fitur/nama-fitur-baru
   ```
5. Buat **Pull Request** di GitHub

### Konvensi Commit

Pakai format [Conventional Commits](https://www.conventionalcommits.org/):

| Prefix | Untuk |
|---|---|
| `feat:` | Fitur baru |
| `fix:` | Perbaikan bug |
| `docs:` | Perubahan dokumentasi |
| `style:` | Format kode (tanpa mengubah fungsi) |
| `refactor:` | Refactor kode |
| `perf:` | Optimasi performa |
| `test:` | Tambah / ubah test |
| `chore:` | Maintenance (build, deps, dll.) |

Contoh:
```
feat: tambah sirkuit gunung dengan elevasi
fix: AI stuck di chicane Monako Mini
docs: perbaiki typo di README
```

### Gaya Kode

- **C++17** — jangan pakai fitur yang belum tersedia
- **4 spasi** untuk indentasi (bukan tab)
- **Nama fungsi**: `CamelCase` (`UpdatePlayer`)
- **Nama variabel**: `camelCase` (`playerSpeed`)
- **Nama konstanta**: `UPPER_SNAKE_CASE` (`ROAD_HALF`)
- **Nama struct**: `PascalCase` (`TrackHit`)
- Komentar singkat untuk fungsi / blok yang kompleks
- Tetap **single file** untuk saat ini (kecuali PR refactor besar)

### Checklist PR

Sebelum submit PR, pastikan:

- [ ] Kode compile tanpa warning di 3 platform (kalau bisa)
- [ ] Sudah dites di Linux / Windows / macOS
- [ ] Tidak ada memory leak (jalankan dengan `valgrind` kalau bisa)
- [ ] Update README kalau ada fitur baru
- [ ] Tidak commit file binary / build artifacts

---

## 📜 Lisensi

Proyek ini dilisensikan di bawah **MIT License**. Lihat file [LICENSE](LICENSE) untuk detail.

```
MIT License

Copyright (c) 2024 [Nama Kamu]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

## 🙏 Kredit

### Library
- **[raylib](https://www.raylib.com/)** oleh Ramon Santamaria — library grafis 3D yang luar biasa sederhana
- **[raymath](https://github.com/raysan5/raylib/blob/master/src/raymath.h)** — utilitas matematika vektor & matriks
- **[rlgl](https://github.com/raysan5/raylib/blob/master/src/rlgl.h)** — layer OpenGL dari raylib

### Inspirasi
- **OutRun** (Sega, 1986) — untuk vibe arcade racing
- **Micro Machines** (Codemasters, 1991) — untuk top-down racing sederhana
- **Trackmania** — untuk bentuk sirkuit yang kreatif
- **F1 Series** (Codemasters) — untuk atmosfer balapan modern

### Teknik
- **Catmull-Rom Spline** — Barry & Goldman, 1988
- **Arc-Length Parameterization** — untuk gerakan yang konsisten
- **Racing Line Generation** — berdasarkan analisis curvature

---

## 📞 Kontak & Komunitas

- **Issues**: [GitHub Issues](https://github.com/USERNAME/racing3d/issues)
- **Discussions**: [GitHub Discussions](https://github.com/USERNAME/racing3d/discussions)
- **Discord**: *(opsional)* [Server Discord](https://discord.gg/xxxxx)

---

## ⭐ Dukung Proyek

Kalau proyek ini bermanfaat untuk kamu:

- ⭐ **Beri bintang** di GitHub
- 🍴 **Fork** dan buat versi kamu sendiri
- 🐛 **Laporkan bug** yang kamu temukan
- 💡 **Usulkan fitur** di Discussions
- 📢 **Bagikan** ke teman yang suka game development

---

**Selamat balapan! 🏁🏎️💨**

*Made with ❤️ using C++ and raylib*
