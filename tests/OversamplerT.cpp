// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "sfizz/OversamplerHelpers.h"
#include "catch2/catch.hpp"
#include <array>
#include <cmath>

TEST_CASE("[Oversampler] Conversion factor")
{
    REQUIRE(sfz::Upsampler::conversionFactor(44100.0, 44100.0) == 1);
    REQUIRE(sfz::Upsampler::conversionFactor(44100.0, 44101.0) == 2);
    REQUIRE(sfz::Upsampler::conversionFactor(44100.0, 88200.0) == 2);
    REQUIRE(sfz::Upsampler::conversionFactor(44100.0, 88201.0) == 4);
    REQUIRE(sfz::Upsampler::conversionFactor(44100.0, 176400.0) == 4);
    REQUIRE(sfz::Upsampler::conversionFactor(44100.0, 176401.0) == 8);
    // low and high limits
    REQUIRE(sfz::Upsampler::conversionFactor(44100.0, 1.0) == 1);
    REQUIRE(sfz::Upsampler::conversionFactor(44100.0, 1e10) == 128);
}

#if SFIZZ_HAVE_SSE
TEST_CASE("[Oversampler] SSE downsampler matches FPU")
{
    hiir::Downsampler2xSse<12> sse;
    hiir::Downsampler2xFpu<12> fpu;
    sse.set_coefs(sfz::OSCoeffs2x);
    fpu.set_coefs(sfz::OSCoeffs2x);

    std::array<float, 128> sseOutput {};
    std::array<float, 128> fpuOutput {};
    for (unsigned i = 0; i < 128; ++i) {
        const float phase = static_cast<float>(i) * 0.125f;
        const float input[2] { std::sin(phase), std::cos(phase) };
        const float paddedInput[4] { input[0], input[1], 12345.0f, -12345.0f };

        sseOutput[i] = sse.process_sample(paddedInput);
        fpuOutput[i] = fpu.process_sample(input);
    }

    // The SSE implementation has one output-sample of latency relative to FPU.
    constexpr unsigned sseLatency = 1;
    for (unsigned i = sseLatency; i < sseOutput.size(); ++i) {
        REQUIRE(sseOutput[i] == Approx(fpuOutput[i - sseLatency]).margin(1e-6f));
    }
}
#endif
