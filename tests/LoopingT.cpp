// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "TestHelpers.h"
#include "sfizz/AudioBuffer.h"
#include "sfizz/Synth.h"
#include "sfizz/Voice.h"
#include "catch2/catch.hpp"
#include "ghc/fs_std.hpp"
#include <algorithm>
#include <limits>
#include <string>
using namespace sfz;

// loop_wrap.wav carries 0.25 on frames 0 to 4 and -1 on every frame after
// them, so a source index leaving the loop shows up as a negative sample.
// Frame 4 is there because the nearest interpolator reads one frame past the
// index it is given. transpose=38 advances the source by about 9.5 frames per
// output sample, so a single step spans the 4-frame loop more than twice.
static constexpr int kLoopSize = 4;
static constexpr float kFramesPerSample = 9.5137f;
static constexpr int kLoopCount = 64;

static std::string loopingSfz(const char* extraOpcodes)
{
    return std::string(
        "<region> sample=loop_wrap.wav sample_quality=0"
        " loop_mode=loop_continuous loop_start=0 loop_end=3"
        " pitch_keycenter=60 transpose=38 ") + extraOpcodes;
}

static float lowestSample(const AudioBuffer<float>& buffer)
{
    float lowest = std::numeric_limits<float>::max();
    for (size_t channel = 0; channel < buffer.getNumChannels(); ++channel) {
        const auto span = buffer.getConstSpan(channel);
        lowest = std::min(lowest, *std::min_element(span.begin(), span.end()));
    }
    return lowest;
}

TEST_CASE("[Looping] A step spanning several loop lengths stays in the loop")
{
    Synth synth;
    synth.enableFreeWheeling();
    synth.setSampleRate(44100);
    synth.setSamplesPerBlock(128);
    AudioBuffer<float> buffer { 2, 128 };
    synth.loadSfzString(fs::current_path() / "tests/TestFiles/loop_wrap.sfz", loopingSfz(""));

    synth.noteOn(0, 60, 100);
    for (int block = 0; block < 2; ++block) {
        synth.renderBlock(buffer);
        REQUIRE( lowestSample(buffer) >= 0.0f );
        const Voice* voice = synth.getVoiceView(0);
        REQUIRE( voice->getSourcePosition() >= 0 );
        REQUIRE( voice->getSourcePosition() < kLoopSize );
    }
}

TEST_CASE("[Looping] loop_count counts every loop a step spans")
{
    Synth synth;
    synth.enableFreeWheeling();
    synth.setSampleRate(44100);
    synth.setSamplesPerBlock(8);
    AudioBuffer<float> buffer { 2, 8 };
    synth.loadSfzString(fs::current_path() / "tests/TestFiles/loop_wrap.sfz", loopingSfz("loop_count=64"));

    // The 64 loops are spent at about 2.4 loops per output sample, so the voice
    // leaves the loop after roughly 27 samples; counting a single loop per
    // sample instead would take 64 of them.
    const int expected = static_cast<int>(kLoopCount * kLoopSize / kFramesPerSample);
    int firstNegative = -1;

    synth.noteOn(0, 60, 100);
    for (int block = 0; block < 12 && firstNegative < 0; ++block) {
        synth.renderBlock(buffer);
        const auto span = buffer.getConstSpan(0);
        for (size_t i = 0; i < span.size(); ++i) {
            if (span[i] < 0.0f) {
                firstNegative = block * static_cast<int>(span.size()) + static_cast<int>(i);
                break;
            }
        }
    }
    REQUIRE( firstNegative >= expected );
    REQUIRE( firstNegative <= expected + 2 * static_cast<int>(buffer.getNumFrames()) );
}
