#pragma once

#include "raylib.h"

#include "Constants.h"
#include "Player.h"
#include "World.h"
#include "CameraController.h"
#include "SnowParticles.h"
#include "SoundManager.h"

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
    Shader groundShader_ = {};
    int groundOffsetLoc_ = -1;
};

} // namespace sb
