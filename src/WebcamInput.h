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

// Tracks the player in front of a webcam and turns it into the same two
// signals the keyboard already provides: a steering axis and a one-shot jump
// trigger. Primary tracking is a Haar face detector, which yields an absolute
// body position that persists while the player stands still; a motion-blob
// detector takes over when no face is visible. Capture and analysis run on a
// background thread so the frame rate of the (comparatively slow) camera never
// stalls the game loop; all public getters are safe to call from the main thread.
class WebcamInput {
public:
    ~WebcamInput();

    // Starts the capture thread, which owns the camera: it opens it, and
    // reopens it by itself if the device disappears mid-session. Returns
    // false only if the thread could not be started; a missing camera is
    // non-fatal and simply leaves IsAvailable() false, so the caller keeps
    // running on keyboard input. Safe to call twice.
    bool Init();
    void Shutdown();

    // -1 (lean/step left) .. +1 (lean/step right).
    float GetSteer() const;

    // Returns true once, the first time it's called after a jump motion was
    // detected, then clears itself.
    bool ConsumeJump();

    // True once the camera opened and at least one frame has been analyzed.
    bool IsAvailable() const { return available_.load(std::memory_order_relaxed); }

    // Hands over the latest annotated preview frame (RGB, 3 bytes/pixel) for
    // an on-screen thumbnail, by swapping it into the caller's buffer - no
    // copy, and the caller's old buffer gets recycled for the next frame.
    // Returns false when no *new* frame has arrived since the last call, so
    // the caller can skip a redundant texture upload.
    bool GetPreviewFrame(std::vector<unsigned char>& outRgb, int& width, int& height);

    // How far the player has to lean for full steer; higher is twitchier.
    void SetSensitivity(float gain) { sensitivity_.store(gain, std::memory_order_relaxed); }
    float GetSensitivity() const { return sensitivity_.load(std::memory_order_relaxed); }

private:
    void CaptureLoop();
    bool OpenCamera();
    void PublishSteer(float steer);

    // Owned exclusively by the capture thread once it starts.
    cv::VideoCapture capture_;
    cv::CascadeClassifier faceCascade_;

    std::thread thread_;
    std::atomic<bool> running_{ false };
    std::atomic<bool> available_{ false };
    std::atomic<float> sensitivity_{ 2.2f };

    mutable std::mutex mutex_;
    float steer_ = 0.0f;
    bool jumpPending_ = false;

    // Double-buffered preview: the thread fills previewBack_, then swaps it
    // into previewFront_ under the lock, so the game thread never blocks on
    // a copy.
    std::vector<unsigned char> previewFront_;
    std::vector<unsigned char> previewBack_;
    int previewWidth_ = 0;
    int previewHeight_ = 0;
    bool hasNewPreview_ = false;
};

} // namespace sb
