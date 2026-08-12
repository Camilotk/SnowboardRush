#include "Player.h"

#include <algorithm>
#include <cmath>

namespace sb {

Player::Player() {
    Reset();
}

void Player::Reset() {
    position = { 0.0f, PLAYER_REST_Y, PLAYER_Z };
    vx = 0.0f;
    vy = 0.0f;
    onGround = true;
    crashed = false;
    lean = 0.0f;
    pitch = 0.0f;
    crashSpin = 0.0f;
}

void Player::Jump() {
    if (onGround) {
        vy = JUMP_SPEED;
        onGround = false;
    }
}

void Player::LaunchFromRamp(float power) {
    vy = power;
    onGround = false;
}

void Player::Crash() {
    if (crashed) return;
    crashed = true;
    vy = 6.0f;   // small pop so the rider tumbles
    onGround = false;
}

void Player::Update(float dt, float steerInput, float speed) {
    landingImpact = 0.0f;

    if (crashed) {
        crashSpin += 360.0f * dt;
        vy += GRAVITY * 0.6f * dt;
        position.y += vy * dt;
        if (position.y <= PLAYER_REST_Y) {
            position.y = PLAYER_REST_Y;
            vy = 0.0f;
        }
        return;
    }

    // Horizontal steering with acceleration + damping. Carving bites harder
    // as downhill speed builds, so lateral authority scales with speed.
    const float speedT = std::clamp(speed / MAX_SPEED, 0.0f, 1.0f);
    const float authority = 0.5f + 0.5f * speedT;
    if (steerInput != 0.0f) {
        vx += steerInput * STEER_ACCEL * authority * dt;
    } else {
        vx -= vx * STEER_DAMP * dt;
        if (std::fabs(vx) < 0.05f) vx = 0.0f;
    }
    vx = std::clamp(vx, -MAX_STEER_SPEED, MAX_STEER_SPEED);

    position.x += vx * dt;
    const float limit = COURSE_HALF_WIDTH - 0.5f;
    if (position.x > limit) { position.x = limit; vx = 0.0f; }
    if (position.x < -limit) { position.x = -limit; vx = 0.0f; }

    // Steering lean (roll): lean into the direction of travel.
    const float targetLean = std::clamp(vx / MAX_STEER_SPEED, -1.0f, 1.0f) * MAX_LEAN;
    lean += (targetLean - lean) * std::min(1.0f, 14.0f * dt);

    // Vertical motion (gravity / jump / landing).
    if (!onGround) {
        vy += GRAVITY * dt;
        position.y += vy * dt;
        if (position.y <= PLAYER_REST_Y) {
            position.y = PLAYER_REST_Y;
            landingImpact = -vy;
            vy = 0.0f;
            onGround = true;
            // Brief nose-down dip on touchdown, recovering via the ground decay.
            pitch = std::clamp(-landingImpact * 0.02f, -0.4f, 0.3f);
        } else {
            pitch = std::clamp(-vy * 0.03f, -0.5f, 0.65f);
        }
    } else {
        pitch *= std::max(0.0f, 1.0f - 8.0f * dt);
        position.y = PLAYER_REST_Y;
    }
}

} // namespace sb
