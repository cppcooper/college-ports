#include <space_invaders/audio.h>

#include <cheryl/modules/miniaudio.h>

#include <algorithm>
#include <cmath>
#include <exception>
#include <iostream>
#include <optional>
#include <utility>
#include <vector>

#include <space_invaders/events.h>

namespace {
    // These files are restored by assets/init.sh.
    const std::filesystem::path music_file =
        "audio/music/Minifantasy_ForgottenPlains_Music/Music/Fair_Fight_(Battle).wav";
    const std::filesystem::path effects_directory =
        "audio/sfx/Helton Yan's Pixel Combat - Single Files";
    constexpr auto player_shot_file = "MAGSpel_CAST-Pew Laser_HY_PC-001.wav";
    constexpr auto enemy_shot_file = "DSGNMisc_PROJECTILE-Laser Shot_HY_PC-001.wav";
    constexpr double enemy_shot_spacing = 0.022;
    constexpr std::size_t maximum_effect_voices = 24;

    CE::Audio::Clip trim_effect(const CE::Audio::Clip& source) {
        const auto format = source.format();
        const auto samples = source.samples();
        const auto channels = format.channels;
        float peak = 0.0f;
        for (const float sample : samples) peak = std::max(peak, std::abs(sample));
        if (peak == 0.0f) return source;
        const float threshold = peak * 0.005f;
        const auto audible = [&](std::size_t frame) {
            for (std::size_t channel = 0; channel < channels; ++channel)
                if (std::abs(samples[frame * channels + channel]) > threshold) return true;
            return false;
        };
        std::size_t first = 0;
        std::size_t last = source.frame_count();
        while (first < last && !audible(first)) ++first;
        while (last > first && !audible(last - 1)) --last;
        // Remove the long silent tails, retaining a short fade to avoid clicks.
        const std::size_t fade = std::max<std::size_t>(1, format.sample_rate / 333);
        first = first > fade ? first - fade : 0;
        last = std::min<std::size_t>(source.frame_count(), last + fade);
        std::vector<float> trimmed(samples.begin() + first * channels, samples.begin() + last * channels);
        const auto frames = last - first;
        for (std::size_t frame = 0; frame < frames; ++frame) {
            const float gain = std::min(1.0f, static_cast<float>(std::min(frame, frames - frame - 1)) / fade);
            for (std::size_t channel = 0; channel < channels; ++channel)
                trimmed[frame * channels + channel] *= gain;
        }
        return CE::Audio::Clip::from_samples(format, std::move(trimmed));
    }

    CE::Audio::Clip make_volley(const CE::Audio::Clip& shot, std::size_t count) {
        const auto format = shot.format();
        const auto spacing_frames = static_cast<std::size_t>(std::llround(format.sample_rate * enemy_shot_spacing));
        const auto spacing_samples = spacing_frames * format.channels;
        std::vector<float> mixed(shot.samples().size() + (count - 1) * spacing_samples, 0.0f);
        const float gain = 1.0f / std::sqrt(static_cast<float>(count));
        for (std::size_t index = 0; index < count; ++index)
            for (std::size_t sample = 0; sample < shot.samples().size(); ++sample)
                mixed[index * spacing_samples + sample] += shot.samples()[sample] * gain;
        float peak = 1.0f;
        for (const float sample : mixed) peak = std::max(peak, std::abs(sample));
        if (peak > 1.0f)
            for (float& sample : mixed) sample /= peak;
        return CE::Audio::Clip::from_samples(format, std::move(mixed));
    }
}

struct GameAudio::State {
    std::unique_ptr<CE::Audio::Miniaudio::System> system;
    std::shared_ptr<CE::Audio::Voice> music;
    std::optional<CE::Audio::Clip> player_shot;
    std::vector<CE::Audio::Clip> enemy_volleys;
    std::vector<std::shared_ptr<CE::Audio::Voice>> effects;
    CE::Events::EventBus::Registration shot_listener;
    std::size_t player_shots = 0;
    std::size_t enemy_shots = 0;
    float music_rate = 1.0f;
    bool suspended = true;
    bool started = false;

    void stop_effects() {
        for (const auto& voice : effects) voice->stop();
        effects.clear();
        player_shots = enemy_shots = 0;
    }

    void play(const CE::Audio::Clip& clip, float volume) {
        if (effects.size() == maximum_effect_voices) {
            effects.front()->stop();
            effects.erase(effects.begin());
        }
        effects.push_back(system->play(clip, {.volume = volume}));
    }
};

GameAudio::GameAudio() = default;
GameAudio::~GameAudio() { close(); }

void GameAudio::load(const std::filesystem::path& asset_root, const std::size_t maximum_enemy_volley) {
    close();
    try {
        auto prepared = std::make_unique<State>();
        prepared->player_shot = trim_effect(CE::Audio::Miniaudio::decode_file(asset_root / effects_directory / player_shot_file));
        const auto enemy_shot = trim_effect(CE::Audio::Miniaudio::decode_file(asset_root / effects_directory / enemy_shot_file));
        // One PCM voice per volley keeps the 22 ms spacing independent of frame rate.
        prepared->enemy_volleys.reserve(std::max<std::size_t>(1, maximum_enemy_volley));
        for (std::size_t count = 1; count <= std::max<std::size_t>(1, maximum_enemy_volley); ++count)
            prepared->enemy_volleys.push_back(make_volley(enemy_shot, count));
        prepared->effects.reserve(maximum_effect_voices);
        prepared->system = CE::Audio::Miniaudio::System::open_device();
        prepared->system->set_volume(0.6f);
        prepared->music = prepared->system->stream(
            asset_root / music_file, {.volume = 0.35f, .looping = true, .paused = true, .variable_rate = true}
        );
        state = std::move(prepared);
    } catch (const std::exception& error) {
        std::cerr << "[space-invaders] Audio unavailable: " << error.what() << '\n';
    }
}

void GameAudio::start(CE::Events::EventBus& events) {
    if (!state || state->started) return;
    try {
        state->shot_listener = events.register_listener(
            SpaceInvadersEvents::shot_requested_event,
            [this](const SpaceInvadersEvents::ShotRequested& shot) {
                if (state->suspended) return;
                if (shot.speed > 0.0)
                    state->player_shots = std::min(state->player_shots + 1, maximum_effect_voices);
                else
                    state->enemy_shots = std::min(state->enemy_shots + 1, state->enemy_volleys.size());
            }
        );
        if (state->music->snapshot().state == CE::Audio::PlaybackState::Paused) state->music->resume();
        else state->music->restart();
        state->started = true;
        state->suspended = false;
    } catch (const std::exception& error) {
        std::cerr << "[space-invaders] Audio stopped: " << error.what() << '\n';
        close();
    }
}

void GameAudio::update(const bool paused, const double game_speed) {
    if (!state || !state->started) return;
    try {
        if (paused != state->suspended) {
            if (paused) {
                state->music->pause();
                state->stop_effects();
            } else state->music->resume();
            state->suspended = paused;
        }
        // Increase from normal playback to double speed over the same 1..10
        // range used for the game's visual intensity and bullet scaling.
        const float rate = 1.0f + static_cast<float>(std::clamp((game_speed - 1.0) / 9.0, 0.0, 1.0));
        if (rate != state->music_rate) {
            state->music->set_playback_rate(rate);
            state->music_rate = rate;
        }
        std::erase_if(state->effects, [](const auto& voice) {
            return voice->snapshot().state != CE::Audio::PlaybackState::Playing;
        });
        if (!paused) {
            for (std::size_t count = 0; count < state->player_shots; ++count)
                state->play(*state->player_shot, 0.7f);
            if (state->enemy_shots > 0)
                state->play(state->enemy_volleys[state->enemy_shots - 1], 0.45f);
        }
        state->player_shots = state->enemy_shots = 0;
        state->system->maintain();
        state->system->publish_notifications();
    } catch (const std::exception& error) {
        std::cerr << "[space-invaders] Audio stopped: " << error.what() << '\n';
        close();
    }
}

void GameAudio::restart() {
    if (!state || !state->started) return;
    try {
        state->stop_effects();
        state->music->set_playback_rate(1.0f);
        state->music_rate = 1.0f;
        state->music->restart();
        state->suspended = false;
    } catch (const std::exception& error) {
        std::cerr << "[space-invaders] Audio stopped: " << error.what() << '\n';
        close();
    }
}

void GameAudio::stop() {
    if (!state) return;
    state->shot_listener.disconnect();
    state->shot_listener = {};
    try {
        state->music->stop();
        state->stop_effects();
    } catch (const std::exception&) {
        // Closing the output below also stops every voice after a device failure.
        state->system->close();
    }
    state->started = false;
    state->suspended = true;
}

void GameAudio::close() {
    stop();
    state.reset();
}
