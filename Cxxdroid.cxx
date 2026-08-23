#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm> // Required for std::min

using namespace std;

// ============================================================================
// ✏️ CUSTOMIZE YOUR TEXT & SETTINGS HERE
// ============================================================================
const string HER_NAME         = "ZAIBBY"; 
const string PHOTO_FILE_PATH  = "Test.jpg"; // Must be in the same folder as the executable
const string MAIN_TITLE       = "HAPPY BIRTHDAY " + HER_NAME + "! 🎂";
const string SUBTITLE_TEXT    = "You make every day brighter... ✨";
const string PROMPT_TEXT      = "[ TAP SCREEN TO UNLOCK SURPRISE ]";
// ============================================================================

// Helper to render text into a clean SDL Texture
SDL_Texture* createTextTexture(SDL_Renderer* renderer, TTF_Font* font, const string& text, SDL_Color color, int& outW, int& outH) {
    if (!font) return NULL;
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surface) return NULL;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    outW = surface->w;
    outH = surface->h;
    SDL_FreeSurface(surface);
    return texture;
}

// Safe Image Loader
static SDL_Texture *loadImage(SDL_Renderer *renderer, const char *path) {
    SDL_Surface *img = IMG_Load(path);
    if (img == NULL) return NULL;
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, img);
    SDL_FreeSurface(img);
    return texture;
}

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) return 1;
    if (TTF_Init() != 0) return 1;

    SDL_Window *window = SDL_CreateWindow("Birthday Surprise", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    // Enable high-quality bilinear texture filtering to keep image crisp
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    // Smart Font Loader: Checks Windows Fonts first, then Android system fonts
    TTF_Font *fontBig = TTF_OpenFont("C:\\Windows\\Fonts\\segoeui.ttf", 36);
    TTF_Font *fontSmall = TTF_OpenFont("C:\\Windows\\Fonts\\segoeui.ttf", 22);

    if (!fontBig)   fontBig   = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 36);
    if (!fontSmall) fontSmall = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 22);
    if (!fontBig)   fontBig   = TTF_OpenFont("/system/fonts/Roboto-Regular.ttf", 36);
    if (!fontSmall) fontSmall = TTF_OpenFont("/system/fonts/Roboto-Regular.ttf", 22);

    // Palette for alternating neon text colors
    vector<SDL_Color> neonColors = {
        {255, 20, 147, 255},  // Deep Pink
        {255, 215, 0, 255},   // Gold / Yellow
        {0, 255, 255, 255},   // Bright Cyan
        {255, 105, 180, 255}, // Hot Pink
        {153, 51, 255, 255},  // Neon Purple
        {255, 255, 255, 255}  // Pure White
    };

    SDL_Texture *photoTexture = loadImage(renderer, PHOTO_FILE_PATH.c_str());

    int screenW, screenH;
    SDL_GetRendererOutputSize(renderer, &screenW, &screenH);

    // --- PROPER ASPECT RATIO CALCULATION (Fixes Distorted Image) ---
    SDL_Rect imgDest = {0, 0, screenW, screenH};
    if (photoTexture != NULL) {
        int imgW = 0, imgH = 0;
        SDL_QueryTexture(photoTexture, NULL, NULL, &imgW, &imgH);

        // Calculate scaling factor to preserve true aspect ratio
        float scale = min((float)screenW / imgW, (float)screenH / imgH);
        int finalW = (int)(imgW * scale);
        int finalH = (int)(imgH * scale);

        // Center the high-resolution image on the screen
        imgDest.w = finalW;
        imgDest.h = finalH;
        imgDest.x = (screenW - finalW) / 2;
        imgDest.y = (screenH - finalH) / 2;
    }

    bool running = true;
    bool showPhoto = false;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false; // Only close when user clicks 'X' or exits app
            } 
            else if (event.type == SDL_FINGERDOWN || event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONDOWN) {
                if (!showPhoto) {
                    showPhoto = true; // First tap unlocks photo and keeps it displayed permanently
                }
            }
        }

        // Dark background
        SDL_SetRenderDrawColor(renderer, 15, 15, 25, 255);
        SDL_RenderClear(renderer);

        if (showPhoto && photoTexture != NULL) {
            // Renders image preserving aspect ratio & maximum resolution
            SDL_RenderCopy(renderer, photoTexture, NULL, &imgDest);
        } 
        else {
            int colorIndex = (SDL_GetTicks() / 250) % neonColors.size();
            SDL_Color currentColor = neonColors[colorIndex];
            bool isBlinkingVisible = (SDL_GetTicks() / 400) % 2 == 0;
            int textW, textH;

            // 1. Main Title
            SDL_Texture* titleTex = createTextTexture(renderer, fontBig, MAIN_TITLE, currentColor, textW, textH);
            if (titleTex) {
                SDL_Rect titleRect = { (screenW - textW) / 2, (screenH / 3), textW, textH };
                SDL_RenderCopy(renderer, titleTex, NULL, &titleRect);
                SDL_DestroyTexture(titleTex);
            }

            // 2. Subtitle
            SDL_Color subColor = { 200, 200, 255, 255 };
            SDL_Texture* subTex = createTextTexture(renderer, fontSmall, SUBTITLE_TEXT, subColor, textW, textH);
            if (subTex) {
                SDL_Rect subRect = { (screenW - textW) / 2, (screenH / 3) + 70, textW, textH };
                SDL_RenderCopy(renderer, subTex, NULL, &subRect);
                SDL_DestroyTexture(subTex);
            }

            // 3. Action Prompt
            if (isBlinkingVisible) {
                SDL_Color blinkColor = { 255, 215, 0, 255 };
                SDL_Texture* blinkTex = createTextTexture(renderer, fontSmall, PROMPT_TEXT, blinkColor, textW, textH);
                if (blinkTex) {
                    SDL_Rect blinkRect = { (screenW - textW) / 2, (screenH / 3) + 160, textW, textH };
                    SDL_RenderCopy(renderer, blinkTex, NULL, &blinkRect);
                    SDL_DestroyTexture(blinkTex);
                }
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // 60 FPS cap
    }

    // Clean up resources on exit
    if (photoTexture) SDL_DestroyTexture(photoTexture);
    if (fontBig) TTF_CloseFont(fontBig);
    if (fontSmall) TTF_CloseFont(fontSmall);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
