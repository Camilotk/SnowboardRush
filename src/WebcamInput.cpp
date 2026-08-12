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
#include <cmath>
#include <deque>

#include "raylib.h"

namespace sb {

namespace {

constexpr double kPi = 3.14159265358979323846;

// Capture is requested at a modest resolution and all analysis happens at
// kProcWidth, which keeps per-frame cost low enough to detect every frame.
constexpr int kCaptureWidth = 640;
constexpr int kCaptureHeight = 480;
constexpr int kCaptureFps = 30;
constexpr int kProcWidth = 320;

// One Euro filter: cuts jitter hard while the player is holding still, then
// opens up as they move so an intentional lean doesn't feel laggy.
constexpr double kMinCutoff = 0.8; // Hz, the resting smoothness.
constexpr double kBeta = 2.0;      // How fast the cutoff opens with speed.
constexpr double kDCutoff = 1.0;   // Hz, smoothing of the speed estimate.

constexpr double kSteerDeadZone = 0.05;   // Normalized, blended not clipped.
constexpr double kNeutralTrimBand = 0.22; // Only re-centre while near neutral.
constexpr double kNeutralTau = 6.0;       // Seconds; deliberately sluggish.
constexpr double kNeutralLimit = 0.5;     // Never trim past half a frame.

constexpr double kMinMotionArea = 120.0;  // In kProcWidth pixels.
constexpr double kDiffThreshold = 18.0;
constexpr auto kFaceHoldTime = std::chrono::milliseconds(500);
constexpr auto kRoiFreshTime = std::chrono::milliseconds(400);
constexpr auto kReacquireReset = std::chrono::milliseconds(2000);
constexpr auto kJumpWindow = std::chrono::milliseconds(220);
constexpr auto kJumpCooldown = std::chrono::milliseconds(450);
constexpr double kJumpRiseFaces = 0.35;     // Hop height, in face heights.
constexpr double kJumpRiseFraction = 0.045; // Fallback: fraction of frame.

// A camera that stops delivering is treated as unplugged: drop it, then keep
// trying to reopen so the player can plug it back in mid-session.
constexpr int kFailuresBeforeReopen = 30;
constexpr auto kReopenInterval = std::chrono::milliseconds(1000);

const char* const kCascadePaths[] = {
    "/usr/share/opencv4/haarcascades/haarcascade_frontalface_alt2.xml",
    "/usr/local/share/opencv4/haarcascades/haarcascade_frontalface_alt2.xml",
    "/usr/share/opencv/haarcascades/haarcascade_frontalface_alt2.xml",
    "C:/opencv/etc/haarcascades/haarcascade_frontalface_alt2.xml",
};

// Position filter from Casiez et al., "1 Euro Filter" (CHI 2012). The cutoff
// frequency rises with the signal's speed, which is exactly the jitter-versus-
// lag trade-off this input needs.
class OneEuroFilter {
public:
    void Reset() { initialized_ = false; }

    double Filter(double x, double dt) {
        if (!initialized_ || dt <= 0.0) {
            initialized_ = true;
            x_ = x;
            dx_ = 0.0;
            return x_;
        }
        const double dx = (x - x_) / dt;
        dx_ += Alpha(kDCutoff, dt) * (dx - dx_);
        const double cutoff = kMinCutoff + kBeta * std::fabs(dx_);
        x_ += Alpha(cutoff, dt) * (x - x_);
        return x_;
    }

private:
    static double Alpha(double cutoff, double dt) {
        const double tau = 1.0 / (2.0 * kPi * cutoff);
        return 1.0 / (1.0 + tau / dt);
    }

    bool initialized_ = false;
    double x_ = 0.0;
    double dx_ = 0.0;
};

} // namespace

WebcamInput::~WebcamInput() { Shutdown(); }

bool WebcamInput::Init() {
    if (running_.load(std::memory_order_relaxed)) return true;

    for (const char* path : kCascadePaths) {
        if (faceCascade_.load(path)) break;
    }
    if (faceCascade_.empty()) {
        TraceLog(LOG_WARNING, "WebcamInput: face cascade not found; using motion tracking only.");
    }

    // The camera is opened on the capture thread so a slow or absent device
    // never stalls startup, and so reopening needs no cross-thread handoff.
    running_.store(true, std::memory_order_relaxed);
    thread_ = std::thread(&WebcamInput::CaptureLoop, this);
    return true;
}

void WebcamInput::Shutdown() {
    running_.store(false, std::memory_order_relaxed);
    if (thread_.joinable()) thread_.join();
    if (capture_.isOpened()) capture_.release();
    available_.store(false, std::memory_order_relaxed);
    PublishSteer(0.0f);
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

bool WebcamInput::GetPreviewFrame(std::vector<unsigned char>& outRgb, int& width, int& height) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!hasNewPreview_) return false;
    outRgb.swap(previewFront_);
    width = previewWidth_;
    height = previewHeight_;
    hasNewPreview_ = false;
    return true;
}

void WebcamInput::PublishSteer(float steer) {
    std::lock_guard<std::mutex> lock(mutex_);
    steer_ = steer;
}

bool WebcamInput::OpenCamera() {
#ifdef __linux__
    // V4L2 directly avoids the GStreamer wrapper, which adds latency and
    // logs spurious warnings for plain webcams.
    if (!capture_.open(0, cv::CAP_V4L2)) capture_.open(0, cv::CAP_ANY);
#else
    capture_.open(0, cv::CAP_ANY);
#endif
    if (!capture_.isOpened()) return false;

    // Best-effort; ignored by cameras that don't support them. BUFFERSIZE is
    // the important one: without it the driver queues frames and every read
    // hands back a stale one, which reads as input lag.
    capture_.set(cv::CAP_PROP_FRAME_WIDTH, kCaptureWidth);
    capture_.set(cv::CAP_PROP_FRAME_HEIGHT, kCaptureHeight);
    capture_.set(cv::CAP_PROP_FPS, kCaptureFps);
    capture_.set(cv::CAP_PROP_BUFFERSIZE, 1);
    return true;
}

void WebcamInput::CaptureLoop() {
    using Clock = std::chrono::steady_clock;

    cv::Mat frame, procColor, gray, blurred, background, backgroundU8, diff, mask;
    std::vector<cv::Rect> faces;
    std::vector<std::vector<cv::Point>> contours;
    std::deque<std::pair<Clock::time_point, double>> centerYHistory;

    OneEuroFilter positionFilter;
    Clock::time_point lastJump{};
    Clock::time_point lastFace{};
    Clock::time_point lastTracked{};
    Clock::time_point lastOpenAttempt{};
    Clock::time_point lastTick = Clock::now();
    cv::Rect lastFaceBox;
    double neutralX = 0.0;
    bool neutralSeeded = false;
    double steer = 0.0;
    int readFailures = 0;
    bool loggedOpen = false;

    // Everything derived from the video stream, dropped whenever the camera
    // goes away so a reconnect starts from a clean slate.
    auto resetTracking = [&]() {
        positionFilter.Reset();
        centerYHistory.clear();
        background.release();
        lastFaceBox = cv::Rect();
        neutralSeeded = false;
        steer = 0.0;
        PublishSteer(0.0f);
    };

    while (running_.load(std::memory_order_relaxed)) {
        // --- Camera lifecycle: open, and reopen after a failure. ---
        if (!capture_.isOpened()) {
            const auto now = Clock::now();
            if (now - lastOpenAttempt < kReopenInterval) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }
            lastOpenAttempt = now;
            if (!OpenCamera()) {
                if (!loggedOpen) {
                    TraceLog(LOG_WARNING,
                             "WebcamInput: no camera available; keyboard input still works.");
                    loggedOpen = true;
                }
                continue;
            }
            TraceLog(LOG_INFO, "WebcamInput: camera opened, tracking started.");
            loggedOpen = true;
            readFailures = 0;
            resetTracking();
            lastTick = Clock::now();
        }

        if (!capture_.read(frame) || frame.empty()) {
            if (++readFailures >= kFailuresBeforeReopen) {
                TraceLog(LOG_WARNING, "WebcamInput: camera stopped responding; will retry.");
                capture_.release();
                available_.store(false, std::memory_order_relaxed);
                resetTracking();
                readFailures = 0;
                loggedOpen = false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        readFailures = 0;

        const auto now = Clock::now();
        const double dt = std::chrono::duration<double>(now - lastTick).count();
        lastTick = now;

        // Mirror the feed so it behaves like a mirror: stepping to your own
        // left moves the tracked body left on screen (and in the preview).
        cv::flip(frame, frame, 1);

        // Downscale once, in colour, then work entirely at this size. The
        // preview reuses the same image, so there is no full-resolution
        // colour conversion or clone anywhere in the loop.
        const double procScale = static_cast<double>(kProcWidth) / frame.cols;
        cv::resize(frame, procColor, cv::Size(), procScale, procScale, cv::INTER_AREA);
        cv::cvtColor(procColor, gray, cv::COLOR_BGR2GRAY);

        // --- Primary: face detection (absolute position, holds when still). ---
        cv::Rect trackBox;
        bool tracked = false;
        bool usingFace = false;
        if (!faceCascade_.empty()) {
            cv::equalizeHist(gray, gray);

            bool found = false;
            // Searching a window around the last known face is several times
            // cheaper than a full scan, which raises the detection rate and
            // therefore how promptly the steer follows a real movement.
            if (lastFaceBox.area() > 0 && now - lastFace < kRoiFreshTime) {
                cv::Rect roi = lastFaceBox;
                const int padX = static_cast<int>(roi.width * 0.7);
                const int padY = static_cast<int>(roi.height * 0.7);
                roi.x -= padX;
                roi.y -= padY;
                roi.width += 2 * padX;
                roi.height += 2 * padY;
                roi &= cv::Rect(0, 0, gray.cols, gray.rows);

                if (roi.width > 20 && roi.height > 20) {
                    const int minSide = std::max(20, static_cast<int>(lastFaceBox.width * 0.6));
                    const int maxSide = static_cast<int>(lastFaceBox.width * 1.7);
                    faceCascade_.detectMultiScale(gray(roi), faces, 1.1, 4, 0,
                                                  cv::Size(minSide, minSide),
                                                  cv::Size(maxSide, maxSide));
                    if (!faces.empty()) {
                        for (auto& f : faces) f += roi.tl();
                        found = true;
                    }
                }
            }
            if (!found) {
                faceCascade_.detectMultiScale(gray, faces, 1.15, 4, 0, cv::Size(30, 30));
                found = !faces.empty();
            }

            if (found) {
                // Largest face = closest person, i.e. the one playing.
                lastFaceBox = *std::max_element(
                    faces.begin(), faces.end(),
                    [](const cv::Rect& a, const cv::Rect& b) { return a.area() < b.area(); });
                lastFace = now;
            }
            // Detection still drops the occasional frame; hold the last box
            // briefly so the steer doesn't flicker between detections.
            if (lastFaceBox.area() > 0 && now - lastFace < kFaceHoldTime) {
                trackBox = lastFaceBox;
                tracked = true;
                usingFace = true;
            }
        }

        // --- Fallback: motion blob, when no face is visible. ---
        if (!tracked) {
            cv::GaussianBlur(gray, blurred, cv::Size(7, 7), 0);
            if (background.empty()) {
                blurred.convertTo(background, CV_32F);
            } else {
                background.convertTo(backgroundU8, CV_8U);
                cv::absdiff(blurred, backgroundU8, diff);
                cv::threshold(diff, mask, kDiffThreshold, 255, cv::THRESH_BINARY);
                cv::dilate(mask, mask, cv::Mat(), cv::Point(-1, -1), 2);
                cv::accumulateWeighted(blurred, background, 0.02);

                cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
                double bestArea = 0.0;
                for (const auto& c : contours) {
                    const double area = cv::contourArea(c);
                    if (area > bestArea) {
                        bestArea = area;
                        trackBox = cv::boundingRect(c);
                    }
                }
                tracked = bestArea >= kMinMotionArea;
            }
        }

        if (tracked) {
            const double halfW = gray.cols * 0.5;
            const double cx = trackBox.x + trackBox.width * 0.5;
            const double cy = trackBox.y + trackBox.height * 0.5;

            // A long dropout usually means the player moved; re-seed rather
            // than snapping back to a neutral that is no longer where they are.
            if (now - lastTracked > kReacquireReset) {
                positionFilter.Reset();
                centerYHistory.clear();
                neutralSeeded = false;
            }
            lastTracked = now;

            const double raw = std::clamp((cx - halfW) / halfW, -1.0, 1.0);
            const double smoothed = positionFilter.Filter(raw, dt);

            if (!neutralSeeded) {
                neutralX = smoothed;
                neutralSeeded = true;
            }
            double offset = smoothed - neutralX;
            // Trim out a camera or seating bias, but only while the player is
            // near their neutral, so holding a real lean can't drag it along.
            if (usingFace && std::fabs(offset) < kNeutralTrimBand) {
                neutralX += (smoothed - neutralX) * (1.0 - std::exp(-dt / kNeutralTau));
                neutralX = std::clamp(neutralX, -kNeutralLimit, kNeutralLimit);
                offset = smoothed - neutralX;
            }

            // Blended dead zone: the response leaves zero continuously instead
            // of stepping, so small corrections near centre stay controllable.
            double magnitude = std::fabs(offset);
            magnitude = magnitude <= kSteerDeadZone
                            ? 0.0
                            : (magnitude - kSteerDeadZone) / (1.0 - kSteerDeadZone);
            const double gain = sensitivity_.load(std::memory_order_relaxed);
            steer = std::clamp(std::copysign(magnitude, offset) * gain, -1.0, 1.0);

            // Jump: a quick rise of the tracked centre. Measuring it in face
            // heights keeps the gesture the same whether the player is sitting
            // close to the camera or standing back from it.
            const double jumpRise = usingFace ? kJumpRiseFaces * trackBox.height
                                              : kJumpRiseFraction * gray.rows;
            centerYHistory.emplace_back(now, cy);
            while (!centerYHistory.empty() && now - centerYHistory.front().first > kJumpWindow) {
                centerYHistory.pop_front();
            }
            if (!centerYHistory.empty() && now - lastJump > kJumpCooldown) {
                if (centerYHistory.front().second - cy > jumpRise) {
                    std::lock_guard<std::mutex> lock(mutex_);
                    jumpPending_ = true;
                    lastJump = now;
                    centerYHistory.clear();
                }
            }
        } else {
            // Ease to centre rather than cutting, so losing the player for a
            // frame doesn't yank the board straight.
            steer += (0.0 - steer) * (1.0 - std::exp(-dt / 0.12));
            centerYHistory.clear();
        }

        // Annotate the preview in place - procColor is not needed afterwards.
        const int midX = procColor.cols / 2;
        const int neutralPx = static_cast<int>(midX + neutralX * midX);
        cv::line(procColor, { neutralPx, 0 }, { neutralPx, procColor.rows },
                 cv::Scalar(180, 180, 180), 1);
        if (tracked) {
            cv::rectangle(procColor, trackBox,
                          usingFace ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 200, 255), 2);
        }
        const int barY = procColor.rows - 10;
        cv::line(procColor, { midX, barY - 5 }, { midX, barY + 5 }, cv::Scalar(180, 180, 180), 1);
        cv::line(procColor, { midX, barY }, { midX + static_cast<int>(steer * midX), barY },
                 cv::Scalar(0, 255, 0), 4);
        cv::cvtColor(procColor, procColor, cv::COLOR_BGR2RGB);

        // Fill the back buffer outside the lock, then swap it in: the game
        // thread never waits on a copy.
        previewBack_.assign(procColor.datastart, procColor.dataend);
        {
            std::lock_guard<std::mutex> lock(mutex_);
            steer_ = static_cast<float>(steer);
            previewFront_.swap(previewBack_);
            previewWidth_ = procColor.cols;
            previewHeight_ = procColor.rows;
            hasNewPreview_ = true;
        }
        available_.store(true, std::memory_order_relaxed);
    }
}

} // namespace sb
