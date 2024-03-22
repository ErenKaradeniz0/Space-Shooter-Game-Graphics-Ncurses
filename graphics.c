#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

int main() {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    TTF_Font* font = NULL;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create window
    window = SDL_CreateWindow("Print Character", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Initialize SDL_ttf
    if (TTF_Init() < 0) {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        return 1;
    }

    // Load font
    font = TTF_OpenFont("arial.ttf", 18);
    if (font == NULL) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        return 1;
    }

    // Set background color
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Color textColor = {255, 255, 255}; // Black color
    char character = '#';
    SDL_Surface* surface = TTF_RenderText_Solid(font, &character, textColor);
    if (surface == NULL) {
        printf("Failed to render text! SDL_ttf Error: %s\n", TTF_GetError());
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        printf("Failed to create texture! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    for (int i = 0; i < 640; i+=10) {
        SDL_Rect dstRect = {i, 0, surface->w, surface->h}; // Position and size
        SDL_RenderCopy(renderer, texture, NULL, &dstRect);
        SDL_Rect dstRect1 = {i, 460, surface->w, surface->h}; // Position and size
        SDL_RenderCopy(renderer, texture, NULL, &dstRect1);
    }
    for (int i = 0; i < 460; i+=14) {
        SDL_Rect dstRect = {0, i, surface->w, surface->h}; // Position and size
        SDL_RenderCopy(renderer, texture, NULL, &dstRect);
        SDL_Rect dstRect1 = {630, i, surface->w, surface->h}; // Position and size
        SDL_RenderCopy(renderer, texture, NULL, &dstRect1);
    }

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);

    // Update screen
    SDL_RenderPresent(renderer);

    // Wait for 5 seconds
    SDL_Delay(5000);

    // Free resources
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
