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

#pragma once

#include "raylib.h"

#include "Constants.h"
#include "Player.h"
#include "World.h"
#include "CameraController.h"
#include "SnowParticles.h"
#include "SoundManager.h"
#include "SnowShader.h"

#ifdef WEBCAM_CONTROL_ENABLED
#include "WebcamInput.h"
#endif

namespace sb {

class Game {
public:
    Game() = default;
    ~Game() = default;

    void Run();

private:
    void LoadAssets();
    void UnloadAssets();

    void Update();
    void Draw();
    void DrawWorld();
    void DrawUI();
#ifdef WEBCAM_CONTROL_ENABLED
    void UpdateWebcamPreview();
#endif

    void StartRun();
    void HandleCollisions();
    void OnCrash();

    // state
    GameState state_ = GameState::Menu;
    Player player_;
    World world_;
    CameraController camera_;
    SnowParticles particles_{ 260 };
    SoundManager sounds_;
#ifdef WEBCAM_CONTROL_ENABLED
    WebcamInput webcam_;
    bool webcamEnabled_ = true;
    Texture2D webcamPreviewTexture_ = {};
    bool webcamPreviewLoaded_ = false;
#endif

    // scoring
    float distance_ = 0.0f;
    int coins_ = 0;
    int rampBonus_ = 0;
    int score_ = 0;
    int highScore_ = 0;

    // timing
    float runTime_ = 0.0f;
    float speed_ = 0.0f;
    float crashTimer_ = 0.0f;
    float menuTime_ = 0.0f;

    // assets
    Model riderModel_ = {};
    Model boardModel_ = {};
    Model treeModel_ = {};
    Model rockModel_ = {};
    Model rampModel_ = {};
    Model coinModel_ = {};
    Model barrierModel_ = {};
    Model groundModel_ = {};
    Model mountainModel_ = {};
    Model moundModel_ = {};
    Model markerModel_ = {};
    Texture2D groundTexture_ = {};
    SnowShader snowShader_;
};

} // namespace sb
