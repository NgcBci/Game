#pragma once
#include <SDL_mixer.h>

class Music {
private:
    Mix_Music* gMusic;
    Mix_Chunk* grabSound;
    Mix_Chunk* fallSound;
    Mix_Chunk* applauseSound;
    bool isGrabSoundPlaying;
    bool isFalling;
    bool hasPlayedApplause;
    Uint32 respawnTime;
    static const Uint32 RESPAWN_COOLDOWN = 1000;  // 1 second cooldown after respawn
    int musicVolume;  // Store current music volume (0-128)
    int sfxVolume;    // Store current SFX volume (0-128)

public:
    Music() : gMusic(nullptr), grabSound(nullptr), fallSound(nullptr), applauseSound(nullptr),
              isGrabSoundPlaying(false), isFalling(false), hasPlayedApplause(false), respawnTime(0),
              musicVolume(MIX_MAX_VOLUME), sfxVolume(MIX_MAX_VOLUME) {}

    Mix_Music* loadMusic(const char* path) {
        gMusic = Mix_LoadMUS(path);
        return gMusic;
    }

    void loadSound(const char* path) {
        grabSound = Mix_LoadWAV(path);
        if (grabSound != nullptr) {
            grabSound->alen = 44100 * 2 * 2;
        }
    }

    void loadFallSound(const char* path) {
        fallSound = Mix_LoadWAV(path);
    }

    void loadApplauseSound(const char* path) {
        applauseSound = Mix_LoadWAV(path);
    }

    void play() {
        if (gMusic == nullptr) return;

        if (Mix_PlayingMusic() == 0) {
            Mix_PlayMusic(gMusic, -1);
            setMusicVolume(musicVolume);
        }
        else if (Mix_PausedMusic() == 1) {
            Mix_ResumeMusic();
        }
    }

    void playGrabSound() {
        if (grabSound != nullptr && !isGrabSoundPlaying) {
            int channel = Mix_PlayChannel(-1, grabSound, 0);
            Mix_Volume(channel, sfxVolume);
            isGrabSoundPlaying = true;
            SDL_AddTimer(1000, [](Uint32 interval, void* param) -> Uint32 {
                Music* music = static_cast<Music*>(param);
                music->isGrabSoundPlaying = false;
                return 0;
            }, this);
        }
    }

    void playFallSound(bool isCurrentlyFalling) {
        if (fallSound != nullptr) {
            Uint32 currentTime = SDL_GetTicks();
            bool isInRespawnCooldown = (currentTime - respawnTime) < RESPAWN_COOLDOWN;

            if (isCurrentlyFalling && !isFalling && !isInRespawnCooldown) {
                int channel = Mix_PlayChannel(-1, fallSound, 0);
                Mix_Volume(channel, sfxVolume);
                isFalling = true;
            }
            else if (!isCurrentlyFalling) {
                isFalling = false;
            }
        }
    }

    void setRespawnTime() {
        respawnTime = SDL_GetTicks();
    }

    void playApplauseSound() {
        if (applauseSound != nullptr && !hasPlayedApplause) {
            int channel = Mix_PlayChannel(-1, applauseSound, 0);
            Mix_Volume(channel, sfxVolume);
            hasPlayedApplause = true;
        }
    }

    void resetApplause() {
        hasPlayedApplause = false;
    }

    void pause() {
        if (Mix_PlayingMusic() == 1) {
            Mix_PauseMusic();
        }
    }

    void stop() {
        Mix_HaltMusic();
    }

    void setMusicVolume(int volume) {
        musicVolume = volume;
        Mix_VolumeMusic(musicVolume);
    }

    void setSfxVolume(int volume) {
        sfxVolume = volume;
        for (int i = 0; i < 8; i++) {
            if (Mix_Playing(i)) {
                Mix_Volume(i, sfxVolume);
            }
        }
    }

    int getMusicVolume() const {
        return musicVolume;
    }

    int getSfxVolume() const {
        return sfxVolume;
    }

    ~Music() {
        if (gMusic != nullptr) {
            Mix_FreeMusic(gMusic);
            gMusic = nullptr;
        }
        if (grabSound != nullptr) {
            Mix_FreeChunk(grabSound);
            grabSound = nullptr;
        }
        if (fallSound != nullptr) {
            Mix_FreeChunk(fallSound);
            fallSound = nullptr;
        }
        if (applauseSound != nullptr) {
            Mix_FreeChunk(applauseSound);
            applauseSound = nullptr;
        }
    }
};
