// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "sfizz/effects/Rectify.h"
#include "sfizz/Opcode.h"
#include "catch2/catch.hpp"
#include <array>
#include <cmath>

TEST_CASE("[Effects] Rectify renders a block")
{
    constexpr unsigned numFrames = 64;
    const std::array<sfz::Opcode, 2> opcodes { {
        { "rectify_mode", "full" },
        { "rectify", "100" },
    } };
    std::unique_ptr<sfz::Effect> effect = sfz::fx::Rectify::makeInstance(opcodes);
    effect->setSampleRate(48000.0);
    effect->setSamplesPerBlock(numFrames);
    effect->clear();

    std::array<float, numFrames> leftInput {};
    std::array<float, numFrames> rightInput {};
    std::array<float, numFrames> leftOutput {};
    std::array<float, numFrames> rightOutput {};
    for (unsigned i = 0; i < numFrames; ++i) {
        leftInput[i] = std::sin(static_cast<float>(i) * 0.25f);
        rightInput[i] = std::cos(static_cast<float>(i) * 0.25f);
    }

    const float* inputs[] { leftInput.data(), rightInput.data() };
    float* outputs[] { leftOutput.data(), rightOutput.data() };
    effect->process(inputs, outputs, numFrames);

    for (unsigned i = 0; i < numFrames; ++i) {
        REQUIRE(std::isfinite(leftOutput[i]));
        REQUIRE(std::isfinite(rightOutput[i]));
    }
}
