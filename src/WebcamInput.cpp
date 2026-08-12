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

#include "WebcamInput.h"

#include <algorithm>
#include <chrono>
#include <deque>

#include "raylib.h"

namespace sb {

namespace {
constexpr double kSteerDeadZone = 0.05;
constexpr double kSteerSmoothing = 0.5;    // EMA weight for new samples.
constexpr double kSteerGain = 1.7;         // Amplifies a modest lean to full steer.
constexpr double kMinMotionArea = 500.0;   // Ignore tiny/noisy contours.
constexpr double kDiffThreshold = 18.0;    // Lower = more sensitive to motion.
constexpr auto kJumpWindow = std::chrono::milliseconds(250);
constexpr auto kJumpCooldown = std::chrono::milliseconds(400);
constexpr double kJumpRiseFraction = 0.035; // Fraction of frame height.
} // namespace

WebcamInput::~WebcamInput() { Shutdown(); }

bool WebcamInput::Init() {
    capture_.open(0);
    if (!capture_.isOpened()) {
        TraceLog(LOG_WARNING, "WebcamInput: could not open a camera; falling back to keyboard-only input.");
        return false;
    }
    running_.store(true, std::memory_order_relaxed);
    thread_ = std::thread(&WebcamInput::CaptureLoop, this);
    TraceLog(LOG_INFO, "WebcamInput: camera opened, motion tracking started.");
    return true;
}

void WebcamInput::Shutdown() {
    running_.store(false, std::memory_order_relaxed);
    if (thread_.joinable()) thread_.join();
    if (capture_.isOpened()) capture_.release();
    available_.store(false, std::memory_order_relaxed);
}

float WebcamInput::GetSteer() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return steer_;
}

bool WebcamInput::ConsumeJump() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!jumpPending_) return false;
    jumpPending_ = false;
    return true;
}

bool WebcamInput::GetPreviewFrame(std::vector<unsigned char>& outRgb, int& width, int& height) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (previewWidth_ == 0 || previewHeight_ == 0) return false;
    outRgb = previewRgb_;
    width = previewWidth_;
    height = previewHeight_;
    return true;
}

void WebcamInput::CaptureLoop() {
    using Clock = std::chrono::steady_clock;

    cv::Mat frame, gray, background, diff, mask;
    std::deque<std::pair<Clock::time_point, double>> topHistory;
    Clock::time_point lastJump{};
    double smoothedSteer = 0.0;

    while (running_.load(std::memory_order_relaxed)) {
        if (!capture_.read(frame) || frame.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        // Mirror the feed so it behaves like a mirror: stepping to your own
        // left moves the tracked blob left on screen (and in the preview).
        cv::flip(frame, frame, 1);

        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv::GaussianBlur(gray, gray, cv::Size(9, 9), 0);

        if (background.empty()) {
            gray.convertTo(background, CV_32F);
            available_.store(true, std::memory_order_relaxed);
            continue;
        }

        cv::Mat backgroundU8;
        background.convertTo(backgroundU8, CV_8U);
        cv::absdiff(gray, backgroundU8, diff);
        cv::threshold(diff, mask, kDiffThreshold, 255, cv::THRESH_BINARY);
        cv::dilate(mask, mask, cv::Mat(), cv::Point(-1, -1), 2);

        // Slowly adapt the background so lighting drift doesn't accumulate,
        // but a standing player doesn't get absorbed into the background.
        cv::accumulateWeighted(gray, background, 0.02);

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        double bestArea = 0.0;
        cv::Rect bestBox;
        for (const auto& c : contours) {
            const double area = cv::contourArea(c);
            if (area > bestArea) {
                bestArea = area;
                bestBox = cv::boundingRect(c);
            }
        }

        const auto now = Clock::now();
        if (bestArea >= kMinMotionArea) {
            const double cx = bestBox.x + bestBox.width * 0.5;
            double target = (cx - frame.cols * 0.5) / (frame.cols * 0.5) * kSteerGain;
            target = std::clamp(target, -1.0, 1.0);
            if (std::fabs(target) < kSteerDeadZone) target = 0.0;
            smoothedSteer += (target - smoothedSteer) * kSteerSmoothing;

            topHistory.emplace_back(now, static_cast<double>(bestBox.y));
            while (!topHistory.empty() && now - topHistory.front().first > kJumpWindow) {
                topHistory.pop_front();
            }
            if (!topHistory.empty() && now - lastJump > kJumpCooldown) {
                const double rise = topHistory.front().second - bestBox.y; // positive = moved up.
                if (rise > kJumpRiseFraction * frame.rows) {
                    std::lock_guard<std::mutex> lock(mutex_);
                    jumpPending_ = true;
                    lastJump = now;
                }
            }
        } else {
            smoothedSteer *= (1.0 - kSteerSmoothing);
            topHistory.clear();
        }

        // Annotate a preview frame with the detection box so the player can
        // see and calibrate what's being tracked.
        cv::Mat preview = frame.clone();
        if (bestArea >= kMinMotionArea) {
            cv::rectangle(preview, bestBox, cv::Scalar(0, 255, 0), 2);
        }
        cv::cvtColor(preview, preview, cv::COLOR_BGR2RGB);
        if (!preview.isContinuous()) preview = preview.clone();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            steer_ = static_cast<float>(smoothedSteer);
            previewWidth_ = preview.cols;
            previewHeight_ = preview.rows;
            previewRgb_.assign(preview.datastart, preview.dataend);
        }
        available_.store(true, std::memory_order_relaxed);
    }
}

} // namespace sb
