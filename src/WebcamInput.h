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

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

#include <opencv2/opencv.hpp>

namespace sb {

// Tracks the player's motion in front of a webcam and turns it into the same
// two signals the keyboard already provides: a steering axis and a one-shot
// jump trigger. Capture and motion analysis run on a background thread so
// the frame rate of the (comparatively slow) camera never stalls the game
// loop; all public getters are safe to call from the main thread.
class WebcamInput {
public:
    ~WebcamInput();

    // Opens the default camera and starts the capture thread. Returns false
    // (non-fatal - the caller should just fall back to keyboard-only input)
    // if no camera is available.
    bool Init();
    void Shutdown();

    // -1 (lean/step left) .. +1 (lean/step right).
    float GetSteer() const;

    // Returns true once, the first time it's called after a jump motion was
    // detected, then clears itself.
    bool ConsumeJump();

    // True once the camera opened and at least one frame has been analyzed.
    bool IsAvailable() const { return available_.load(std::memory_order_relaxed); }

    // Copies out the latest mirrored preview frame (RGB, 3 bytes/pixel) with
    // the detected motion box drawn in green, for an on-screen thumbnail.
    // Returns false if no frame has been captured yet.
    bool GetPreviewFrame(std::vector<unsigned char>& outRgb, int& width, int& height) const;

private:
    void CaptureLoop();

    cv::VideoCapture capture_;
    std::thread thread_;
    std::atomic<bool> running_{ false };
    std::atomic<bool> available_{ false };

    mutable std::mutex mutex_;
    float steer_ = 0.0f;
    bool jumpPending_ = false;

    std::vector<unsigned char> previewRgb_;
    int previewWidth_ = 0;
    int previewHeight_ = 0;
};

} // namespace sb
