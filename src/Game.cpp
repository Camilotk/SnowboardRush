// Snowboard Rush - a downhill snowboarding endless runner.
// Copyright (C) 2026 Camilo Cunha de Azevedo
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "Game.h"

#include "raymath.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace sb {

namespace {
constexpr Color kSkyBlue = { 132, 190, 235, 255 };
constexpr Color kSnowWhite = { 240, 246, 252, 255 };
constexpr Color kTextColor = { 20, 30, 50, 255 };

// Resolve an asset path relative to the executable when possible (so a
// packaged binary finds its assets no matter where it is launched from),
// falling back to the compile-time ASSETS_PATH.
std::string AssetsPath(const char* relative) {
    static std::string base;
    if (base.empty()) {
        const char* exeDir = GetApplicationDirectory();
        const std::string candidate = std::string(exeDir) + "assets/";
        base = FileExists((candidate + "models/snowboarder.obj").c_str())
                   ? candidate
                   : ASSETS_PATH;
    }
    return base + relative;
}
} // namespace

void Game::Run() {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "Snowboard Rush");
    SetTargetFPS(60);

    LoadAssets();
    sounds_.Init(AssetsPath("audio/wind.ogg").c_str());
#ifdef WEBCAM_CONTROL_ENABLED
    webcam_.Init();
#endif

    while (!WindowShouldClose()) {
        Update();
        Draw();
    }

    sounds_.Shutdown();
#ifdef WEBCAM_CONTROL_ENABLED
    webcam_.Shutdown();
#endif
    UnloadAssets();
    CloseWindow();
}

void Game::LoadAssets() {
    riderModel_ = LoadModel(AssetsPath("models/snowboarder.obj").c_str());
    boardModel_ = LoadModel(AssetsPath("models/snowboard.obj").c_str());
    treeModel_ = LoadModel(AssetsPath("models/pine_tree.obj").c_str());
    rockModel_ = LoadModel(AssetsPath("models/rock.obj").c_str());
    rockModel_.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = { 45, 55, 70, 255 };
    rampModel_ = LoadModel(AssetsPath("models/ramp.obj").c_str());
    coinModel_ = LoadModel(AssetsPath("models/coin.obj").c_str());

    barrierModel_ = LoadModel(AssetsPath("models/barrier.obj").c_str());

    moundModel_ = LoadModelFromMesh(GenMeshSphere(0.5f, 8, 8));
    moundModel_.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = kSnowWhite;

    markerModel_ = LoadModel(AssetsPath("models/marker.obj").c_str());

    mountainModel_ = LoadModelFromMesh(GenMeshCone(55.0f, 120.0f, 6));
    mountainModel_.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = { 226, 238, 250, 255 };

    groundModel_ = LoadModelFromMesh(GenMeshPlane(2000.0f, 2000.0f, 1, 1));
    Image snow = GenImagePerlinNoise(512, 512, 0, 0, 40.0f);
    ImageColorBrightness(&snow, 60);
    ImageColorTint(&snow, { 248, 252, 255, 255 });
    groundTexture_ = LoadTextureFromImage(snow);
    UnloadImage(snow);
    SetTextureWrap(groundTexture_, TEXTURE_WRAP_REPEAT);
    groundModel_.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = groundTexture_;

    // Advanced procedural snow shader, shared by the snow-covered surfaces.
    if (snowShader_.Load(AssetsPath("shaders/snow.vs").c_str(),
                         AssetsPath("shaders/snow.fs").c_str())) {
        snowShader_.ApplySettings(MakePowderPreset());
        groundModel_.materials[0].shader = snowShader_.shader;
        moundModel_.materials[0].shader = snowShader_.shader;
        mountainModel_.materials[0].shader = snowShader_.shader;
    }

    // Tilt the ground into a downhill grade (drops off toward -z / ahead).
    groundModel_.transform = MatrixRotateX(-SLOPE_ANGLE);

    camera_.Reset();
}

void Game::UnloadAssets() {
    UnloadModel(riderModel_);
    UnloadModel(boardModel_);
    UnloadModel(treeModel_);
    UnloadModel(rockModel_);
    UnloadModel(rampModel_);
    UnloadModel(coinModel_);
    UnloadModel(barrierModel_);
    UnloadModel(groundModel_);
    UnloadModel(mountainModel_);
    UnloadModel(moundModel_);
    UnloadModel(markerModel_);
    UnloadTexture(groundTexture_);
    snowShader_.Unload();
#ifdef WEBCAM_CONTROL_ENABLED
    if (webcamPreviewLoaded_) UnloadTexture(webcamPreviewTexture_);
#endif
}

void Game::StartRun() {
    player_.Reset();
    world_.Reset();
    camera_.Reset();
    speed_ = BASE_SPEED;
    runTime_ = 0.0f;
    distance_ = 0.0f;
    coins_ = 0;
    rampBonus_ = 0;
    score_ = 0;
    crashTimer_ = 0.0f;
    state_ = GameState::Playing;
}

void Game::OnCrash() {
    state_ = GameState::Crashed;
    crashTimer_ = 0.0f;
    if (score_ > highScore_) highScore_ = score_;
    player_.Crash();
    sounds_.PlayCrash();
}

void Game::HandleCollisions() {
    const Vector3& p = player_.position;

    // Obstacles.
    for (const auto& o : world_.Obstacles()) {
        if (!o.active) continue;
        if (std::fabs(o.position.z) > PLAYER_RADIUS + o.radius) continue;
        if (std::fabs(o.position.x - p.x) > PLAYER_RADIUS + o.radius) continue;
        if (p.y < o.clearHeight) {
            OnCrash();
            return;
        }
    }

    // Collectibles.
    for (auto& c : world_.Collectibles()) {
        if (!c.active) continue;
        if (std::fabs(c.position.z) > 1.1f) continue;
        if (std::fabs(c.position.x - p.x) > 1.3f) continue;
        if (std::fabs(c.position.y - p.y) > 1.7f) continue;
        c.active = false;
        ++coins_;
        sounds_.PlayCoin();
    }

    // Ramps.
    for (auto& r : world_.Ramps()) {
        if (!r.active || r.used) continue;
        if (std::fabs(r.position.z) > 2.6f) continue;
        if (std::fabs(r.position.x - p.x) > r.halfWidth) continue;
        if (!player_.onGround) continue;
        r.used = true;
        player_.LaunchFromRamp(RAMP_LAUNCH_FACTOR * speed_);
        rampBonus_ += RAMP_SCORE;
        sounds_.PlayRamp();
    }
}

#ifdef WEBCAM_CONTROL_ENABLED
void Game::UpdateWebcamPreview() {
    if (!webcamEnabled_ || !webcam_.IsAvailable()) return;

    std::vector<unsigned char> rgb;
    int w = 0, h = 0;
    if (!webcam_.GetPreviewFrame(rgb, w, h)) return;

    if (!webcamPreviewLoaded_ || webcamPreviewTexture_.width != w || webcamPreviewTexture_.height != h) {
        if (webcamPreviewLoaded_) UnloadTexture(webcamPreviewTexture_);
        Image img = {};
        img.data = rgb.data();
        img.width = w;
        img.height = h;
        img.mipmaps = 1;
        img.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
        webcamPreviewTexture_ = LoadTextureFromImage(img);
        webcamPreviewLoaded_ = true;
    } else {
        UpdateTexture(webcamPreviewTexture_, rgb.data());
    }
}
#endif

void Game::Update() {
    float dt = GetFrameTime();
    if (dt > 0.05f) dt = 0.05f;

    menuTime_ += dt;
#ifdef WEBCAM_CONTROL_ENABLED
    UpdateWebcamPreview();
#endif

    // Development-only shader debug controls.
    if (IsKeyPressed(KEY_F5)) snowShader_.CycleDebugMode(-1);
    if (IsKeyPressed(KEY_F6)) snowShader_.CycleDebugMode(1);

    switch (state_) {
        case GameState::Menu: {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                StartRun();
            }
            particles_.Update(dt);
            sounds_.UpdateWind(0.0f);
            break;
        }
        case GameState::Playing: {
            float steer = 0.0f;
            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) steer -= 1.0f;
            if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) steer += 1.0f;
            bool jumpPressed = IsKeyPressed(KEY_SPACE);

#ifdef WEBCAM_CONTROL_ENABLED
            if (IsKeyPressed(KEY_C)) webcamEnabled_ = !webcamEnabled_;
            if (webcamEnabled_ && webcam_.IsAvailable()) {
                steer = std::clamp(steer + webcam_.GetSteer(), -1.0f, 1.0f);
                if (webcam_.ConsumeJump()) jumpPressed = true;
            }
#endif

            if (jumpPressed) {
                player_.Jump();
                sounds_.PlayJump();
            }
            if (IsKeyPressed(KEY_R)) {
                StartRun();
                break;
            }

            runTime_ += dt;
            // Downhill speed: gravity pull along the slope minus snow friction,
            // quadratic air drag, and carving scrub. Reaches terminal ~50 u/s.
            const float downhill = -GRAVITY * SLOPE_SIN;
            const float friction = SNOW_MU * -GRAVITY * SLOPE_COS;
            const float carveScrub = CARVE_SCRUB * std::fabs(steer) * speed_;
            speed_ += (downhill - friction - DRAG_K * speed_ * speed_ - carveScrub) * dt;

            // Plowing through snow mounds scrubs speed (soft, no crash).
            float snow = 0.0f;
            for (const auto& m : world_.Mounds()) {
                if (!m.active) continue;
                const float dx = m.position.x - player_.position.x;
                const float dz = m.position.z; // player rides at z = 0
                const float reach = m.radius + PLAYER_RADIUS;
                const float d = std::sqrt(dx * dx + dz * dz);
                if (d < reach) snow += 1.0f - d / reach;
            }
            snow = std::min(1.0f, snow);
            if (snow > 0.0f) {
                speed_ *= std::max(0.0f, 1.0f - SNOW_SCRUB * snow * dt);
            }
            speed_ = std::clamp(speed_, 0.0f, MAX_SPEED);

            player_.Update(dt, steer, speed_);
            if (player_.landingImpact > 0.0f) {
                speed_ *= std::max(0.0f, 1.0f - LANDING_SCRUB * player_.landingImpact);
            }
            world_.Update(dt, speed_);
            distance_ = world_.Distance();
            score_ = coins_ * COIN_SCORE + static_cast<int>(distance_) * POINTS_PER_METER + rampBonus_;

            HandleCollisions();
            camera_.Update(dt, player_.position, speed_);
            particles_.Update(dt);
            sounds_.UpdateWind(std::min(1.0f, speed_ / MAX_SPEED));
            break;
        }
        case GameState::Crashed: {
            crashTimer_ += dt;
            player_.Update(dt, 0.0f, 0.0f);
            particles_.Update(dt);
            sounds_.UpdateWind(0.0f);
            if (IsKeyPressed(KEY_R) || IsKeyPressed(KEY_ENTER)) {
                StartRun();
            }
            break;
        }
    }
}

void Game::DrawWorld() {
    // Ground.
    DrawModel(groundModel_, { 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);

    // Distant mountains (static backdrop, sitting on the slope).
    const Vector3 mountains[] = {
        { -140.0f, 0.0f, -360.0f },
        { -60.0f, 0.0f, -430.0f },
        { 40.0f, 0.0f, -400.0f },
        { 140.0f, 0.0f, -350.0f },
        { 220.0f, 0.0f, -470.0f },
        { -230.0f, 0.0f, -480.0f },
    };
    for (const auto& m : mountains) {
        DrawModel(mountainModel_, { m.x, SLOPE * m.z, m.z }, 1.0f, WHITE);
    }

    // Sun.
    DrawSphereEx({ 60.0f, 70.0f, -220.0f }, 14.0f, 10, 10, { 255, 245, 200, 255 });

    // Obstacles.
    for (const auto& o : world_.Obstacles()) {
        if (!o.active) continue;
        switch (o.type) {
            case ObstacleType::Tree:
                DrawModelEx(treeModel_, o.position, { 0.0f, 1.0f, 0.0f }, o.yaw * DEG2RAD, { 1.0f, 1.0f, 1.0f }, WHITE);
                break;
            case ObstacleType::Rock:
                DrawModelEx(rockModel_, o.position, { 0.0f, 1.0f, 0.0f }, o.yaw * DEG2RAD, { 1.0f, 1.0f, 1.0f }, WHITE);
                break;
            case ObstacleType::Barrier:
                DrawModel(barrierModel_, o.position, 1.0f, WHITE);
                break;
        }
    }

    // Ramps.
    for (const auto& r : world_.Ramps()) {
        if (!r.active) continue;
        DrawModel(rampModel_, r.position, 1.0f, WHITE);
    }

    // Coins.
    const float t = static_cast<float>(GetTime());
    for (const auto& c : world_.Collectibles()) {
        if (!c.active) continue;
        const Vector3 pos = {
            c.position.x,
            c.position.y + std::sin(c.bobPhase + t * 4.0f) * 0.15f,
            c.position.z
        };
        DrawModelEx(coinModel_, pos, { 0.0f, 1.0f, 0.0f }, c.spin * DEG2RAD, { 1.0f, 1.0f, 1.0f }, WHITE);
    }

    // Snow mounds.
    for (const auto& m : world_.Mounds()) {
        if (!m.active) continue;
        const Vector3 pos = { m.position.x, m.position.y + m.radius * 0.4f, m.position.z };
        const Vector3 scale = { m.radius * 2.0f, m.radius * 1.2f, m.radius * 2.0f };
        DrawModelEx(moundModel_, pos, { 0.0f, 1.0f, 0.0f }, 0.0f, scale, WHITE);
    }

    // Boundary flag poles.
    for (const auto& k : world_.Markers()) {
        if (!k.active) continue;
        const float yaw = (k.position.x > 0.0f) ? PI : 0.0f;
        DrawModelEx(markerModel_, k.position, { 0.0f, 1.0f, 0.0f }, yaw, { 1.0f, 1.0f, 1.0f }, WHITE);
    }

    // Player: snowboard + rider.
    {
        const Vector3 ppos = { player_.position.x, player_.position.y, PLAYER_Z };
        Matrix rot;
        if (player_.crashed) {
            rot = MatrixMultiply(MatrixRotateX(-1.35f), MatrixRotateZ(player_.crashSpin * DEG2RAD));
        } else {
            // Negative roll so the rider banks INTO the direction of travel.
            rot = MatrixMultiply(MatrixRotateX(player_.pitch - PLAYER_FORWARD_PITCH), MatrixRotateZ(-player_.lean));
        }
        const Matrix transform = MatrixMultiply(MatrixTranslate(ppos.x, ppos.y, ppos.z), rot);

        boardModel_.transform = transform;
        DrawModel(boardModel_, { 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);

        riderModel_.transform = transform;
        DrawModel(riderModel_, { 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);
    }

    // Falling snow.
    particles_.Draw();
}

void Game::DrawUI() {
    const int sw = GetScreenWidth();
    const int sh = GetScreenHeight();

    switch (state_) {
        case GameState::Menu: {
            DrawRectangle(0, 0, sw, sh, Fade(WHITE, 0.10f));
            const char* title = "SNOWBOARD RUSH";
            const int titleSize = 90;
            const int titleW = MeasureText(title, titleSize);
            DrawText(title, (sw - titleW) / 2, 150, titleSize, kTextColor);

            const int subSize = 26;
            const char* lines[] = {
                "Ride downhill, dodge obstacles, collect coins.",
                "Hit a ramp to launch into the air!",
                "",
                "A / D  or  ARROW KEYS   -  steer",
                "SPACE                    -  jump",
                "R                        -  restart",
                "ESC                      -  quit",
            };
            const int n = static_cast<int>(sizeof(lines) / sizeof(lines[0]));
            int y = 320;
            for (int i = 0; i < n; ++i) {
                const int w = MeasureText(lines[i], subSize);
                DrawText(lines[i], (sw - w) / 2, y, subSize, kTextColor);
                y += 40;
            }

            const char* prompt = "PRESS ENTER TO START";
            const int promptSize = 34;
            const int pw = MeasureText(prompt, promptSize);
            const float pulse = 0.6f + 0.4f * std::sin(menuTime_ * 5.0f);
            DrawText(prompt, (sw - pw) / 2, 640, promptSize,
                     { (unsigned char)(kTextColor.r * pulse + 255 * (1.0f - pulse)),
                       (unsigned char)(kTextColor.g * pulse + 255 * (1.0f - pulse)),
                       (unsigned char)(kTextColor.b * pulse + 255 * (1.0f - pulse)), 255 });
            break;
        }

        case GameState::Playing: {
            const int pad = 16;
            const int fs = 26;

            DrawRectangle(0, 0, 300, 120, Fade(BLACK, 0.35f));
            DrawText(TextFormat("SCORE  %d", score_), pad, pad, fs, RAYWHITE);
            DrawText(TextFormat("COINS  %d", coins_), pad, pad + 34, fs, GOLD);
            DrawText(TextFormat("DIST   %d m", static_cast<int>(distance_)), pad, pad + 68, fs, RAYWHITE);

            // Speed indicator.
            const int barW = 220;
            const int barH = 14;
            const int bx = sw - barW - pad;
            const int by = pad + 6;
            DrawText("SPEED", bx, by - 24, 20, RAYWHITE);
            DrawRectangle(bx, by, barW, barH, Fade(WHITE, 0.25f));
            const float speedT = std::min(1.0f, (speed_ - BASE_SPEED) / (MAX_SPEED - BASE_SPEED));
            DrawRectangle(bx, by, static_cast<int>(barW * speedT), barH, { 60, 200, 120, 255 });

            if (runTime_ < 6.0f) {
                const char* hint = "A / D  steer   -   SPACE  jump";
                const int hw = MeasureText(hint, 22);
                DrawText(hint, (sw - hw) / 2, sh - 60, 22, Fade(RAYWHITE, 0.9f));
            }

            if (snowShader_.DebugMode() != 0) {
                DrawText(TextFormat("SHADER DEBUG %d  (F5/F6)", snowShader_.DebugMode()),
                         16, sh - 44, 20, YELLOW);
            }
            break;
        }

        case GameState::Crashed: {
            DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.45f));
            const char* over = "GAME OVER";
            const int overSize = 84;
            DrawText(over, (sw - MeasureText(over, overSize)) / 2, 180, overSize, { 220, 60, 50, 255 });

            const int fs = 30;
            const char* scoreLine = TextFormat("Score  %d", score_);
            const char* coinLine = TextFormat("Coins  %d", coins_);
            const char* distLine = TextFormat("Distance  %d m", static_cast<int>(distance_));
            const char* highLine = TextFormat("High Score  %d", highScore_);

            DrawText(scoreLine, (sw - MeasureText(scoreLine, fs)) / 2, 340, fs, RAYWHITE);
            DrawText(coinLine, (sw - MeasureText(coinLine, fs)) / 2, 380, fs, GOLD);
            DrawText(distLine, (sw - MeasureText(distLine, fs)) / 2, 420, fs, RAYWHITE);
            DrawText(highLine, (sw - MeasureText(highLine, fs)) / 2, 460, fs, RAYWHITE);

            const char* restart = "PRESS R TO RESTART";
            const int rs = 32;
            DrawText(restart, (sw - MeasureText(restart, rs)) / 2, 560, rs, RAYWHITE);
            break;
        }
    }

#ifdef WEBCAM_CONTROL_ENABLED
    if (webcamEnabled_ && webcamPreviewLoaded_) {
        const int thumbW = 220;
        const int thumbH = webcamPreviewTexture_.height * thumbW / webcamPreviewTexture_.width;
        const int tx = sw - thumbW - 16;
        const int ty = sh - thumbH - 16;
        DrawRectangle(tx - 4, ty - 4, thumbW + 8, thumbH + 8, Fade(BLACK, 0.5f));
        DrawTexturePro(webcamPreviewTexture_,
                        { 0, 0, (float)webcamPreviewTexture_.width, (float)webcamPreviewTexture_.height },
                        { (float)tx, (float)ty, (float)thumbW, (float)thumbH },
                        { 0, 0 }, 0.0f, WHITE);
        DrawRectangleLines(tx, ty, thumbW, thumbH, { 60, 200, 120, 255 });
        DrawText("CAM (C)", tx, ty - 20, 16, Fade(RAYWHITE, 0.8f));
    }
#endif
}

void Game::Draw() {
    BeginDrawing();
    ClearBackground(kSkyBlue);

    // Feed the snow shader the real camera position every frame.
    snowShader_.UpdatePerFrame(camera_.camera.position, static_cast<float>(GetTime()));

    BeginMode3D(camera_.camera);
    DrawWorld();
    EndMode3D();

    DrawUI();
    EndDrawing();
}

} // namespace sb
