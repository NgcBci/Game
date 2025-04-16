#pragma once
#include <SDL_mixer.h>

class Music {
private:
    Mix_Music* gMusic;
    Mix_Chunk* grabSound;
    Mix_Chunk* fallSound;
    Mix_Chunk* applauseSound;
    Mix_Chunk* congratsSound;
    bool isGrabSoundPlaying;
    bool isFalling;
    bool hasPlayedApplause;
    Uint32 respawnTime;
    static const Uint32 RESPAWN_COOLDOWN = 1000;  // 1 second cooldown after respawn

public:
    Music() : gMusic(nullptr), grabSound(nullptr), fallSound(nullptr), applauseSound(nullptr),
              congratsSound(nullptr), isGrabSoundPlaying(false), isFalling(false), 
              hasPlayedApplause(false), respawnTime(0) {}

    Mix_Music* loadMusic(const char* path) {
        gMusic = Mix_LoadMUS(path);
        if (gMusic == nullptr) {
            SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION,
                          SDL_LOG_PRIORITY_ERROR,
                          "Could not load music! SDL_mixer Error: %s", Mix_GetError());
        }
        return gMusic;
    }

    void loadSound(const char* path) {
        grabSound = Mix_LoadWAV(path);
        if (grabSound != nullptr) {
            // Set the length of the sound to 1 second (assuming 44.1kHz sample rate)
            grabSound->alen = 44100 * 2 * 2;  // 44100 samples/sec * 2 channels * 2 bytes/sample
        } else {
            SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION,
                          SDL_LOG_PRIORITY_ERROR,
                          "Could not load sound effect! SDL_mixer Error: %s", Mix_GetError());
        }
    }

    void loadFallSound(const char* path) {
        fallSound = Mix_LoadWAV(path);
        if (fallSound == nullptr) {
            SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION,
                          SDL_LOG_PRIORITY_ERROR,
                          "Could not load fall sound effect! SDL_mixer Error: %s", Mix_GetError());
        }
    }

    void loadApplauseSound(const char* path) {
        applauseSound = Mix_LoadWAV(path);
        if (applauseSound == nullptr) {
            SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION,
                          SDL_LOG_PRIORITY_ERROR,
                          "Could not load applause sound effect! SDL_mixer Error: %s", Mix_GetError());
        }
    }

    void loadCongratsSound(const char* path) {
        congratsSound = Mix_LoadWAV(path);
        if (congratsSound == nullptr) {
            SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION,
                          SDL_LOG_PRIORITY_ERROR,
                          "Could not load congrats sound effect! SDL_mixer Error: %s", Mix_GetError());
        }
    }

    void play() {
        if (gMusic == nullptr) return;

        if (Mix_PlayingMusic() == 0) {
            Mix_PlayMusic(gMusic, -1);
        }
        else if (Mix_PausedMusic() == 1) {
            Mix_ResumeMusic();
        }
    }

    void playGrabSound() {
        if (grabSound != nullptr && !isGrabSoundPlaying) {
            Mix_PlayChannel(-1, grabSound, 0);
            isGrabSoundPlaying = true;
            // Reset the flag after the sound duration (1 second)
            SDL_AddTimer(1000, [](Uint32 interval, void* param) -> Uint32 {
                Music* music = static_cast<Music*>(param);
                music->isGrabSoundPlaying = false;
                return 0;  // Don't repeat the timer
            }, this);
        }
    }

    void playFallSound(bool isCurrentlyFalling) {
        if (fallSound != nullptr) {
            Uint32 currentTime = SDL_GetTicks();
            bool isInRespawnCooldown = (currentTime - respawnTime) < RESPAWN_COOLDOWN;

            if (isCurrentlyFalling && !isFalling && !isInRespawnCooldown) {
                // Start falling
                Mix_PlayChannel(-1, fallSound, 0);
                isFalling = true;
            }
            else if (!isCurrentlyFalling) {
                // Stop falling
                isFalling = false;
            }
        }
    }

    void setRespawnTime() {
        respawnTime = SDL_GetTicks();
    }

    void playApplauseSound() {
        if (applauseSound != nullptr && !hasPlayedApplause) {
            Mix_PlayChannel(-1, applauseSound, 0);
            hasPlayedApplause = true;
        }
    }

    void resetApplause() {
        hasPlayedApplause = false;
    }

    void playCongratsSound() {
        if (congratsSound != nullptr) {
            Mix_PlayChannel(-1, congratsSound, 0);
        }
    }

    void pause() {
        if (Mix_PlayingMusic() == 1) {
            Mix_PauseMusic();
        }
    }

    void stop() {
        Mix_HaltMusic();
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
        if (congratsSound != nullptr) {
            Mix_FreeChunk(congratsSound);
            congratsSound = nullptr;
        }
    }
};
