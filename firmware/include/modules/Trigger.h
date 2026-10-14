#pragma once

/**
 * Platform input trigger: whether the environment is asking modes for activity right now.
 *
 * The lamp answers from the microphone (sound level above threshold) and always answers true when no microphone is
 * built in. Other platforms may script it.
 */
namespace Trigger
{
[[nodiscard]] bool active();
}
